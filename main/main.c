

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "nvs_flash.h"
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_system.h"
#include "esp_chip_info.h"
#include "hal/gpio_types.h"
#include "driver/gpio.h"
#include "lvgl.h"
#include "lvgl_demo.h"
#include "bsp_can.h"
#include "myiic.h"
#include "xl9555.h"
#include "ui.h"
#include "ui/vars.h"
#include "ui/fonts.h"
#include "lcd.h"
#include "tinyusb.h"
#include "tinyusb_cdc_acm.h"
#include "tinyusb_console.h"
/* TINYUSB_DEFAULT_CONFIG() 宏定义在该头文件中, tinyusb.h 并不包含它 */
#include "tinyusb_default_config.h"
#include "ws2815_test.h"
#include "rotary_input.h"

static const char *TAG = "MAIN";

/*
 * 仅用于硬件定位：设为 1 时，app_main() 在 USB CDC 就绪后直接初始化 MIPI
 * 屏，并循环显示纯色；LVGL、触摸、I2C 扩展器、CAN 和 UI 任务均不会启动。
 * 确认屏幕正常后改回 0，即可恢复原应用。
 */
#define LCD_MINIMAL_TEST 0
/*
 * 正点原子开发板对照测试的第一阶段：只验证 GPIO53 背光。
 * 保持为 0 时绝不初始化 DSI PHY、绝不读屏 ID、绝不发送任何 MIPI 命令。
 * 确认背光后才改为 1 进入下一阶段。
 */
#define LCD_MINIMAL_TEST_ENABLE_MIPI 0

/* GPIO13 WS2812 board wiring test: hold the data pin at a steady 3.3 V. */
#define GPIO13_HIGH_LEVEL_TEST 0

/* GPIO13 WS2812 test mode: all 16 LEDs remain at full-bright red. */
#define WS2815_FULL_RED_TEST 1

/* LCD初始化完成后启用KEY2/KEY3/KEY4旋钮(GPIO3/4/5)。 */
#define ROTARY_INPUT_ENABLE 0

/* 当前转速值(由CAN回调更新, 由LVGL任务读取) */
static volatile int g_current_rpm = 0;
static volatile bool g_can_rx_seen = false;
static volatile TickType_t g_last_can_rx_tick = 0;
/* 互斥信号量, 保护转速数据 */
static SemaphoreHandle_t g_rpm_mutex = NULL;

/* 独立BOOT按键，直接连接ESP32-P4 GPIO35，低电平有效。 */
#define BOOT_GPIO_PIN GPIO_NUM_35

/* EXIO8/9/10通过XL9555 IO扩展器连接，均按低电平有效处理。 */

/* CAN TX引脚 (根据实际硬件修改) */
#define CAN_TX_PIN GPIO_NUM_27
/* CAN RX引脚 (根据实际硬件修改) */
#define CAN_RX_PIN GPIO_NUM_26

/*tinyusb初始化函数*/
/**
 * @brief       USB CDC 虚拟串口控制台初始化
 * @note        整个程序只能调用一次; 调用成功后 ESP_LOGI/printf 的输出会同时
 *              从 USB 虚拟串口输出, 形成“USB 调试串口”。
 *              该函数内部会把日志任务绑到默认的 TinyUSB 任务上, 因此不要在
 *              高优先级任务中反复调用。
 * @param       无
 * @retval      无
 */
static void usb_cdc_console_init(void)
{
    /*
     * 第1步: 安装 TinyUSB 设备驱动, 并启动 TinyUSB 后台任务。
     * TINYUSB_DEFAULT_CONFIG() 会按目标芯片自动填入默认参数:
     *   - ESP32-P4 使用高速端口 TINYUSB_PORT_HIGH_SPEED_0
     *   - PHY 使用默认值, 且不监控 VBUS(vbus_monitor_io = -1)
     *   - 设备/配置描述符指针为空, 由协议栈回退到内置的默认描述符
     * 若后续要自定义 VID/PID 或字符串描述符, 需要在此结构体的
     * descriptor 成员中填入自己的描述符表。
     */
    const tinyusb_config_t tusb_cfg = TINYUSB_DEFAULT_CONFIG();
    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

    /*
     * 第2步: 初始化一路 CDC ACM 接口(即虚拟串口)。
     * cdc_port 指定 CDC 接口号, TINYUSB_CDC_ACM_0 表示第 0 路;
     * 这里不注册任何回调, 收发数据使用 tinyusb_cdcacm_read/write,
     * 若只当作控制台使用则无需手动读写。
     * 其余回调成员未指定, 由 C 语言自动初始化为 NULL。
     */
    tinyusb_config_cdcacm_t cdc_cfg = {
        .cdc_port = TINYUSB_CDC_ACM_0,
    };
    ESP_ERROR_CHECK(tinyusb_cdcacm_init(&cdc_cfg));

    /*
     * 第3步: 把标准输入输出(stdio)重定向到该 CDC 接口, 使其成为控制台。
     * 注意 esp_tinyusb v2.x 的形参是 int 类型的“接口号”,
     * 而不是 tinyusb_config_cdcacm_t 结构体指针;
     * 接口未初始化时返回 ESP_ERR_INVALID_STATE(已被 ESP_ERROR_CHECK 拦截)。
     */
    ESP_ERROR_CHECK(tinyusb_console_init(cdc_cfg.cdc_port));
}

#if LCD_MINIMAL_TEST
static void lcd_minimal_test(void)
{
#if LCD_MINIMAL_TEST_ENABLE_MIPI
    static const uint16_t colors[] = { RED, GREEN, BLUE, WHITE, BLACK };
    static const char *const names[] = { "RED", "GREEN", "BLUE", "WHITE", "BLACK" };
#endif
    const gpio_config_t lcd_ctrl_cfg = {
#if LCD_MINIMAL_TEST_ENABLE_MIPI
        .pin_bit_mask = (1ULL << LCD_BL_PIN) | (1ULL << LCD_RST_PIN),
#else
        /* 背光单项测试：GPIO52/MIPI_RST 与全部 DSI 专用引脚保持完全未配置。 */
        .pin_bit_mask = (1ULL << LCD_BL_PIN),
#endif
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(gpio_config(&lcd_ctrl_cfg));
    LCD_BACKLIGHT_OFF();
#if LCD_MINIMAL_TEST_ENABLE_MIPI
    LCD_RESET_ASSERT();
    vTaskDelay(pdMS_TO_TICKS(20));
    LCD_RESET_RELEASE();
#else
    vTaskDelay(pdMS_TO_TICKS(20));
#endif
    LCD_BACKLIGHT_ON();
#if LCD_MINIMAL_TEST_ENABLE_MIPI
    ESP_LOGI(TAG, "LCD signals: GPIO53(BL)=1, GPIO52(RST)=1; hold for 2 seconds");
#else
    ESP_LOGI(TAG, "Backlight-only test: GPIO53(LCD_BL)=1; GPIO52 and MIPI DSI are untouched");
#endif
    vTaskDelay(pdMS_TO_TICKS(2000));

#if LCD_MINIMAL_TEST_ENABLE_MIPI
    /* 放在 DSI 前，避免 USB 枚举初期的日志被 monitor 漏掉。 */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    ESP_LOGW(TAG, "ESP32-P4 silicon revision: %u (sdkconfig minimum: 3.0)", chip_info.revision);
    ESP_LOGW(TAG, "Reset reason immediately before DSI: %d", esp_reset_reason());
    ESP_LOGI(TAG, "LCD minimal test: initialising MIPI LCD only");
    lcd_init();
    if (lcddev.lcd_panel_handle == NULL) {
        ESP_LOGE(TAG, "MIPI initialisation failed; GPIO53 remains HIGH for hardware checks");
        for (;;) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
    ESP_LOGI(TAG, "LCD ready: %lux%lu; backlight is ON", lcddev.width, lcddev.height);

    /* Generate bars inside the P4 DSI host itself. This bypasses PSRAM frame
     * buffers, DMA and lcd_clear(), leaving only the DSI video transmitter and
     * the panel as the variables under test. */
    ESP_ERROR_CHECK(esp_lcd_dpi_panel_set_pattern(lcddev.lcd_panel_handle,
                                                  MIPI_DSI_PATTERN_BAR_VERTICAL));
    ESP_LOGI(TAG, "DSI hardware vertical colour-bar pattern is active");

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    for (size_t i = 0;; i = (i + 1) % (sizeof(colors) / sizeof(colors[0]))) {
        ESP_LOGI(TAG, "LCD test colour: %s", names[i]);
        lcd_clear(colors[i]);
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
#else
    ESP_LOGI(TAG, "GPIO-only test active: MIPI DSI is deliberately disabled");
    for (;;) {
        /* Keep LCD_BL asserted continuously; GPIO52 and all DSI pins stay untouched. */
        LCD_BACKLIGHT_ON();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
#endif
}
#endif

/**
 * @brief       CAN数据接收回调(在CAN任务上下文中调用)
 * @param       can_id : CAN扩展帧ID
 * @param       data   : 数据缓冲区
 * @param       len    : 数据长度
 * @retval      无
 */
static void can_data_callback(uint32_t can_id, uint8_t *data, uint8_t len)
{
    /* 任意有效CAN帧都用于刷新通信在线时间。 */
    if (data && len > 0)
    {
        g_last_can_rx_tick = xTaskGetTickCount();

        g_can_rx_seen = true;
    }

    /* 检查数据长度 */
    if (!data || len < 2)
        return;

    int rpm = 0;

    switch (can_id)
    {
    case VCU_TO_MCU_CMD_ID:
        /* VCU→MCU 电机控制命令: 字节1-2为转速值 */
        rpm = bsp_can_parse_speed(data[0], data[1]);
        ESP_LOGD(TAG, "VCU→MCU speed: %d rpm", rpm);
        break;

    case MCU_TO_VCU_STATUS1_ID:
        /* MCU→VCU 电机状态信息1: 字节1-2为转速值 */
        rpm = bsp_can_parse_speed(data[0], data[1]);
        ESP_LOGD(TAG, "MCU→VCU speed: %d rpm", rpm);
        break;

    default:
        /* 其他报文忽略 */
        return;
    }

    /* 更新转速值(带保护) */
    if (g_rpm_mutex)
    {
        xSemaphoreTake(g_rpm_mutex, portMAX_DELAY);
        g_current_rpm = rpm;
        xSemaphoreGive(g_rpm_mutex);
    }
}

#define AMI_SELECTION_COUNT 6
#define INSPECT_STAGE_COUNT 5

#define ASSI_OFF_COLOR 0x68717d
#define ASSI_YELLOW_COLOR 0xffd400
#define ASSI_BLUE_COLOR 0x168bff

/* 主页面六个任务按屏幕上的顺时针顺序排列。 */
static lv_obj_t *ami_selection_led(uint8_t index)
{
    lv_obj_t *leds[AMI_SELECTION_COUNT] = {
        objects.driving, objects.line_acc, objects.eight,
        objects.high_foll, objects.ebs_1, objects.check};
    return index < AMI_SELECTION_COUNT ? leds[index] : NULL;
}

static lv_obj_t *s_ami_selection_label;

static void ami_show_selection(uint8_t selected)
{
    static const char *const names[AMI_SELECTION_COUNT] = {
        "DRIVING", "LINE ACC", "EIGHT", "HIGH FOLLOW", "EBS", "INSPECT"};

    if (!objects.ami)
        return;

    if (!s_ami_selection_label)
    {
        s_ami_selection_label = lv_label_create(objects.ami);
        lv_obj_set_pos(s_ami_selection_label, 780, 500);
        lv_obj_set_size(s_ami_selection_label, 360, 70);
        lv_obj_set_style_text_align(s_ami_selection_label, LV_TEXT_ALIGN_CENTER,
                                    LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(s_ami_selection_label, lv_color_hex(0xffffff),
                                    LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(s_ami_selection_label, &ui_font_orbitron_bold_30,
                                   LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    for (uint8_t i = 0; i < AMI_SELECTION_COUNT; ++i)
    {
        lv_obj_t *led = ami_selection_led(i);
        if (led)
        {
            bool is_selected = (i == selected);
            /* 未选择为红色，当前选择为绿色。 */
            lv_led_set_color(led, lv_color_hex(is_selected ? 0x39ff65 : 0xff2020));
            lv_led_set_brightness(led, 255);
        }
    }

    if (selected < AMI_SELECTION_COUNT)
    {
        lv_label_set_text(s_ami_selection_label, names[selected]);
        lv_obj_clear_flag(s_ami_selection_label, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        /* 未选择时不显示任何提示文字，避免遮挡主界面。 */
        lv_label_set_text(s_ami_selection_label, "");
        lv_obj_add_flag(s_ami_selection_label, LV_OBJ_FLAG_HIDDEN);
    }

    lv_obj_invalidate(objects.ami);
}

static void inspect_show_stage(uint8_t stage)
{
    lv_obj_t *circles[INSPECT_STAGE_COUNT] = {
        objects.obj40, objects.obj44, objects.obj48, objects.obj52, objects.obj56};
    lv_obj_t *numbers[INSPECT_STAGE_COUNT] = {
        objects.obj41, objects.obj45, objects.obj49, objects.obj53, objects.obj57};
    lv_obj_t *names[INSPECT_STAGE_COUNT] = {
        objects.obj42, objects.obj46, objects.obj50, objects.obj54, objects.obj58};
    const char *actions[INSPECT_STAGE_COUNT] = {
        "LV ON", "SELF TEST", "READY", "DRIVE", "FINISH"};
    const char *details[INSPECT_STAGE_COUNT] = {
        "SWITCH LV ON", "RUN SYSTEM SELF TEST", "CHECK READY CONDITIONS",
        "BEGIN INSPECTION DRIVE", "INSPECTION COMPLETE"};
    const char *as_states[INSPECT_STAGE_COUNT] = {
        "AS OFF", "SELF TEST", "AS READY", "AS DRIVING", "AS FINISHED"};
    const uint32_t assi_colors[INSPECT_STAGE_COUNT] = {
        ASSI_OFF_COLOR, ASSI_YELLOW_COLOR, ASSI_YELLOW_COLOR,
        ASSI_YELLOW_COLOR, ASSI_BLUE_COLOR};

    uint32_t current_assi_color = assi_colors[stage];

    for (uint8_t i = 0; i < INSPECT_STAGE_COUNT; ++i)
    {
        uint32_t color = i < stage ? 0x39ff65 : (i == stage ? current_assi_color : 0x68717d);
        uint32_t bg = i <= stage ? color : 0x11151a;
        lv_obj_set_style_bg_color(circles[i], lv_color_hex(bg), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(circles[i], lv_color_hex(color), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(numbers[i], lv_color_hex(i <= stage ? 0x050607 : color), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(names[i], lv_color_hex(color), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    set_var_ami_main_action(actions[stage]);
    set_var_ami_main_detail(details[stage]);
    set_var_ami_as_state_text(as_states[stage]);
    set_var_ami_as_state_code(stage);
    tick_screen_inspect();

    /* AUTONOMOUS SYSTEM大字和AMOY左侧ASSI指示灯使用同一状态颜色。 */
    lv_color_t assi_color = lv_color_hex(current_assi_color);
    lv_obj_set_style_text_color(objects.inspect_as_state, assi_color,
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(objects.inspect_main_action, assi_color,
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(objects.inspect_footer_status, assi_color,
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(objects.inspect_state_led, assi_color,
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(objects.inspect_state_led, assi_color,
                                  LV_PART_MAIN | LV_STATE_DEFAULT);
    /* AS OFF灭灯，其余动作阶段由定时扫描控制1秒闪烁。 */
    lv_obj_set_style_bg_opa(objects.inspect_state_led,
                            stage == 0 ? LV_OPA_TRANSP : LV_OPA_COVER,
                            LV_PART_MAIN | LV_STATE_DEFAULT);
    if (stage == 0)
    {
        lv_obj_add_flag(objects.inspect_state_led, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_clear_flag(objects.inspect_state_led, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief       LVGL UI更新任务
 * @param       arg : 未使用
 * @retval      无
 */
static void ui_update_task(void *arg)
{
    int last_rpm = -1;
    int last_speed = -1; /* 用于车速显示(0-120) */
    char buf[16];
    uint8_t boot_last = 1;
    uint8_t extio9_last = 1;
    uint8_t extio10_last = 1;
    uint8_t ami_selection = 0; /* 默认选择有人驾驶/DRIVING */
    uint8_t inspect_stage = 0;
    TickType_t inspect_blink_tick = xTaskGetTickCount();
    bool inspect_led_on = true;
    bool last_can_online = true; /* 强制首次循环将界面更新为CAN OFF */

    ESP_LOGI(TAG, "UI update task started");
    ami_show_selection(ami_selection);

    while (1)
    {
        int current_rpm = 0;

        /* 读取当前转速值 */
        if (g_rpm_mutex)
        {
            xSemaphoreTake(g_rpm_mutex, portMAX_DELAY);
            current_rpm = g_current_rpm;
            xSemaphoreGive(g_rpm_mutex);
        }

        /* 仅在数值变化时更新UI, 减少LVGL刷新 */
        if (current_rpm != last_rpm)
        {
            last_rpm = current_rpm;

            /* 更新车速显示 (将转速按比例映射到0-120范围) */
            int speed_val = (abs(current_rpm) * 120) / 10000;
            if (speed_val > 120)
                speed_val = 120;

            if (speed_val != last_speed)
            {
                last_speed = speed_val;
                if (objects.speed_label)
                {
                    snprintf(buf, sizeof(buf), "%03d", speed_val);
                    lv_label_set_text_static(objects.speed_label, buf);
                }
            }
        }

        /* === 检测当前显示的屏幕 === */
        lv_obj_t *active_scr = lv_scr_act();

        /* 启动时默认CAN OFF；收到帧后亮绿，超过1秒无新帧则离线。 */
        TickType_t can_now = xTaskGetTickCount();
        bool can_online = g_can_rx_seen &&
                          (can_now - g_last_can_rx_tick <= pdMS_TO_TICKS(1000));
        if (can_online != last_can_online && objects.inspect_can_status)
        {
            if (lvgl_port_lock(portMAX_DELAY))
            {
                lv_label_set_text(objects.inspect_can_status,
                                  can_online ? "CAN OK" : "CAN OFF");
                lv_obj_set_style_text_color(objects.inspect_can_status,
                                            lv_color_hex(can_online ? 0x39ff65 : 0xff2c2c),
                                            LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_invalidate(objects.inspect_can_status);
                lvgl_port_unlock();
            }
            last_can_online = can_online;
        }

        /* === 独立BOOT / GPIO35：任何页面返回主界面 === */
        {
            uint8_t val = gpio_get_level(BOOT_GPIO_PIN);
            static uint8_t debounce_cnt = 0;
            if (val == 0)
            {
                if (debounce_cnt < 5)
                    debounce_cnt++;
            }
            else
            {
                if (debounce_cnt > 0)
                    debounce_cnt--;
            }
            uint8_t curr = (debounce_cnt >= 4) ? 0 : 1;

            if (boot_last == 1 && curr == 0)
            {
                ESP_LOGI(TAG, "BOOT pressed (GPIO35)");
                ami_selection = 0;
                ami_show_selection(ami_selection);
                loadScreen(SCREEN_ID_AMI);
            }
            boot_last = curr;
        }

        /* === 旋钮：主界面左/右选择，按下确认 === */
#if ROTARY_INPUT_ENABLE
        {
            rotary_event_t event;
            while (rotary_input_get_event(&event, 0))
            {
                if (active_scr != objects.ami)
                    continue;

                if (event == ROTARY_EVENT_LEFT)
                {
                    ami_selection = (ami_selection == UINT8_MAX || ami_selection == 0)
                                        ? (AMI_SELECTION_COUNT - 1)
                                        : (uint8_t)(ami_selection - 1);
                    ami_show_selection(ami_selection);
                    ESP_LOGI(TAG, "Rotary left, selection=%u", ami_selection);
                }
                else if (event == ROTARY_EVENT_RIGHT)
                {
                    ami_selection = (ami_selection == UINT8_MAX)
                                        ? 0
                                        : (uint8_t)((ami_selection + 1) % AMI_SELECTION_COUNT);
                    ami_show_selection(ami_selection);
                    ESP_LOGI(TAG, "Rotary right, selection=%u", ami_selection);
                }
                else if (event == ROTARY_EVENT_PRESS && ami_selection != UINT8_MAX)
                {
                    if (ami_selection == AMI_SELECTION_COUNT - 1)
                    {
                        inspect_stage = 0;
                        inspect_led_on = false;
                        inspect_blink_tick = xTaskGetTickCount();
                        inspect_show_stage(inspect_stage);
                        loadScreen(SCREEN_ID_INSPECT);
                    }
                    else
                    {
                        loadScreen(SCREEN_ID_AUTONOMOUS);
                    }
                    ESP_LOGI(TAG, "Rotary press, selection=%u", ami_selection);
                }
            }
        }
#endif

        /* === EXIO9：确认主页面任务 === */
        {
            uint8_t val = xl9555_extio9_read();
            static uint8_t debounce_cnt = 0;
            if (val == 0)
            {
                if (debounce_cnt < 5)
                    debounce_cnt++;
            }
            else
            {
                if (debounce_cnt > 0)
                    debounce_cnt--;
            }
            uint8_t curr = (debounce_cnt >= 4) ? 0 : 1;

            if (extio9_last == 1 && curr == 0)
            {
                if (active_scr == objects.ami && ami_selection != UINT8_MAX)
                {
                    if (ami_selection == AMI_SELECTION_COUNT - 1)
                    {
                        inspect_stage = 0;
                        inspect_led_on = false;
                        inspect_blink_tick = xTaskGetTickCount();
                        inspect_show_stage(inspect_stage);
                        loadScreen(SCREEN_ID_INSPECT);
                    }
                    else
                    {
                        loadScreen(SCREEN_ID_AUTONOMOUS);
                    }
                }
            }
            extio9_last = curr;
        }

        /* === EXIO10：车检页面进入下一阶段，到FINISH后保持 === */
        {
            uint8_t val = xl9555_extio10_read();
            static uint8_t debounce_cnt = 0;
            if (val == 0)
            {
                if (debounce_cnt < 5)
                    debounce_cnt++;
            }
            else
            {
                if (debounce_cnt > 0)
                    debounce_cnt--;
            }
            uint8_t curr = (debounce_cnt >= 4) ? 0 : 1;

            if (extio10_last == 1 && curr == 0 && active_scr == objects.inspect)
            {
                if (inspect_stage + 1 < INSPECT_STAGE_COUNT)
                {
                    ++inspect_stage;
                    inspect_led_on = true;
                    inspect_blink_tick = xTaskGetTickCount();
                    inspect_show_stage(inspect_stage);
                    ESP_LOGI(TAG, "Inspection stage=%u", inspect_stage);
                }
            }
            extio10_last = curr;
        }

        /* 车检页除AS OFF外：ASSI灯按当前颜色每1秒切换一次亮/灭。 */
        if (active_scr == objects.inspect && inspect_stage > 0)
        {
            TickType_t now = xTaskGetTickCount();
            if ((now - inspect_blink_tick) >= pdMS_TO_TICKS(1000))
            {
                /* 全屏刷新可能持锁较久，必须成功取得锁后才更新节拍。 */
                if (lvgl_port_lock(portMAX_DELAY))
                {
                    inspect_blink_tick = xTaskGetTickCount();
                    inspect_led_on = !inspect_led_on;
                    if (inspect_led_on)
                    {
                        lv_obj_clear_flag(objects.inspect_state_led, LV_OBJ_FLAG_HIDDEN);
                    }
                    else
                    {
                        lv_obj_add_flag(objects.inspect_state_led, LV_OBJ_FLAG_HIDDEN);
                    }
                    lv_obj_invalidate(objects.inspect_state_led);
                    lvgl_port_unlock();
                    ESP_LOGI(TAG, "ASSI LED %s", inspect_led_on ? "ON" : "OFF");
                }
            }
        }
        /* 每20ms检查一次 */
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

/**
 * @brief       程序入口
 * @param       无
 * @retval      无
 */
void app_main(void)
{
#if GPIO13_HIGH_LEVEL_TEST
    gpio13_high_level_test();
    return;
#endif

#if WS2815_FULL_RED_TEST
    /* Keep this before every other peripheral initialisation for a focused RMT test. */
    ws2815_full_red_test();
    return;
#endif

#if LCD_MINIMAL_TEST
    /*
     * 硬件第一阶段必须先于 USB：若 USB CDC 配置或枚举失败，仍可判断芯片
     * 是否已经运行到应用程序。此时 X2-7 应立即为 3.3 V，背光应点亮。
     */
    const gpio_config_t lcd_probe_cfg = {
        .pin_bit_mask = (1ULL << LCD_BL_PIN) | (1ULL << LCD_RST_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&lcd_probe_cfg));
    LCD_RESET_ASSERT();
    LCD_BACKLIGHT_OFF();
    vTaskDelay(pdMS_TO_TICKS(20));
    LCD_RESET_RELEASE();
    LCD_BACKLIGHT_ON();
    vTaskDelay(pdMS_TO_TICKS(2000));
#endif

    usb_cdc_console_init();
    ESP_LOGI(TAG, "USB CDC ready");
    ESP_LOGW(TAG, "Reset reason: %d", esp_reset_reason());

#if LCD_MINIMAL_TEST
    /* 留出 CDC 枚举时间，之后只执行 LCD 驱动和纯色刷新。 */
    vTaskDelay(pdMS_TO_TICKS(1000));
    lcd_minimal_test();
    return;
#endif

    esp_err_t ret;
    ESP_LOGI(TAG, "System starting...");

    ret = nvs_flash_init(); /* 初始化NVS */
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    /* 创建互斥信号量 */
    g_rpm_mutex = xSemaphoreCreateMutex();
    assert(g_rpm_mutex);

    /* 16-LED tachometer uses the VCU/MCU speed field updated in the CAN callback. */
    ws2815_rpm_bar_start(&g_current_rpm);

    /* Keep the tachometer independent of LCD/LVGL initialisation. */
    ESP_LOGI(TAG, "Initializing CAN (TX:%d, RX:%d)...", CAN_TX_PIN, CAN_RX_PIN);
    if (bsp_can_init(CAN_TX_PIN, CAN_RX_PIN, can_data_callback) == 0)
    {
        ESP_LOGI(TAG, "CAN initialized successfully");
    }
    else
    {
        ESP_LOGW(TAG, "CAN initialization failed; RPM bar remains in standby");
    }

    /* 初始化I2C总线 (XL9555 IO扩展器使用) */
    ret = myiic_init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C init failed!");
    }
    else
    {
        ESP_LOGI(TAG, "I2C initialized (SCL=32, SDA=33)");
    }

    /* 初始化XL9555 IO扩展器 (EXIO8/9/10为输入) */
    ret = xl9555_init(bus_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "XL9555 init failed! EXIO buttons will not work!");
    }
    else
    {
        ESP_LOGI(TAG, "XL9555 initialized (EXIO8/9/10 inputs)");
    }

    gpio_config_t boot_cfg = {
        .pin_bit_mask = 1ULL << BOOT_GPIO_PIN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&boot_cfg));
    ESP_LOGI(TAG, "BOOT initialized (GPIO%d)", BOOT_GPIO_PIN);

    /* 初始化LVGL显示 */
    lvgl_demo(); /* 运行LVGL例程 */

    /*
     * GPIO3还用于LCD RGB识别(M2)，因此旋钮必须放在lvgl_demo()之后初始化；
     * 否则旋钮模块的VCC/上拉会改变LCD识别电平，导致MIPI屏被误判为RGB屏。
     */
#if ROTARY_INPUT_ENABLE
    ESP_ERROR_CHECK(rotary_input_init());
#endif

    /* 创建UI更新任务 */
    xTaskCreatePinnedToCore(
        ui_update_task,
        "ui_update",
        4096,
        NULL,
        3,
        NULL,
        tskNO_AFFINITY);

    ESP_LOGI(TAG, "System started");
}
