

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "nvs_flash.h"
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "hal/gpio_types.h"
#include "driver/gpio.h"
#include "lvgl.h"
#include "lvgl_demo.h"
#include "bsp_can.h"
#include "myiic.h"
#include "xl9555.h"
#include "ui.h"
#include "ui/vars.h"

static const char *TAG = "MAIN";

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
#define CAN_TX_PIN  GPIO_NUM_27
/* CAN RX引脚 (根据实际硬件修改) */
#define CAN_RX_PIN  GPIO_NUM_26

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
    if (data && len > 0) {
        g_last_can_rx_tick = xTaskGetTickCount();
        g_can_rx_seen = true;
    }

    /* 检查数据长度 */
    if (!data || len < 2) return;

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
#define INSPECT_STAGE_COUNT  5

#define ASSI_OFF_COLOR       0x68717d
#define ASSI_YELLOW_COLOR    0xffd400
#define ASSI_BLUE_COLOR      0x168bff

/* 主页面六个任务按屏幕上的顺时针顺序排列。 */
static lv_obj_t *ami_selection_led(uint8_t index)
{
    lv_obj_t *leds[AMI_SELECTION_COUNT] = {
        objects.driving, objects.line_acc, objects.eight,
        objects.high_foll, objects.ebs_1, objects.check
    };
    return index < AMI_SELECTION_COUNT ? leds[index] : NULL;
}

static void ami_show_selection(uint8_t selected)
{
    for (uint8_t i = 0; i < AMI_SELECTION_COUNT; ++i) {
        lv_obj_t *led = ami_selection_led(i);
        if (led) {
            lv_led_set_color(led, lv_color_hex(i == selected ? 0xff0000 : 0x00ff73));
            lv_led_set_brightness(led, 255);
        }
    }
}

static void inspect_show_stage(uint8_t stage)
{
    lv_obj_t *circles[INSPECT_STAGE_COUNT] = {
        objects.obj40, objects.obj44, objects.obj48, objects.obj52, objects.obj56
    };
    lv_obj_t *numbers[INSPECT_STAGE_COUNT] = {
        objects.obj41, objects.obj45, objects.obj49, objects.obj53, objects.obj57
    };
    lv_obj_t *names[INSPECT_STAGE_COUNT] = {
        objects.obj42, objects.obj46, objects.obj50, objects.obj54, objects.obj58
    };
    const char *actions[INSPECT_STAGE_COUNT] = {
        "LV ON", "SELF TEST", "READY", "DRIVE", "FINISH"
    };
    const char *details[INSPECT_STAGE_COUNT] = {
        "SWITCH LV ON", "RUN SYSTEM SELF TEST", "CHECK READY CONDITIONS",
        "BEGIN INSPECTION DRIVE", "INSPECTION COMPLETE"
    };
    const char *as_states[INSPECT_STAGE_COUNT] = {
        "AS OFF", "SELF TEST", "AS READY", "AS DRIVING", "AS FINISHED"
    };
    const uint32_t assi_colors[INSPECT_STAGE_COUNT] = {
        ASSI_OFF_COLOR, ASSI_YELLOW_COLOR, ASSI_YELLOW_COLOR,
        ASSI_YELLOW_COLOR, ASSI_BLUE_COLOR
    };

    uint32_t current_assi_color = assi_colors[stage];

    for (uint8_t i = 0; i < INSPECT_STAGE_COUNT; ++i) {
        uint32_t color = i < stage ? 0x39ff65 :
                         (i == stage ? current_assi_color : 0x68717d);
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
    if (stage == 0) {
        lv_obj_add_flag(objects.inspect_state_led, LV_OBJ_FLAG_HIDDEN);
    } else {
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
    int last_speed = -1;    /* 用于车速显示(0-120) */
    char buf[16];
    uint8_t boot_last = 1;
    uint8_t extio9_last = 1;
    uint8_t extio10_last = 1;
    uint8_t ami_selection = UINT8_MAX; /* 尚未选择；首次BOOT选择第一个 */
    uint8_t inspect_stage = 0;
    TickType_t inspect_blink_tick = xTaskGetTickCount();
    bool inspect_led_on = true;
    bool last_can_online = true; /* 强制首次循环将界面更新为CAN OFF */

    ESP_LOGI(TAG, "UI update task started");

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
            if (speed_val > 120) speed_val = 120;

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
        if (can_online != last_can_online && objects.inspect_can_status) {
            if (lvgl_port_lock(portMAX_DELAY)) {
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

        /* === 独立BOOT / GPIO35：主页面顺时针轮选 === */
        {
            uint8_t val = gpio_get_level(BOOT_GPIO_PIN);
            static uint8_t debounce_cnt = 0;
            if (val == 0) { if (debounce_cnt < 5) debounce_cnt++; }
            else          { if (debounce_cnt > 0) debounce_cnt--; }
            uint8_t curr = (debounce_cnt >= 4) ? 0 : 1;

            if (boot_last == 1 && curr == 0)
            {
                ESP_LOGI(TAG, "BOOT pressed (GPIO35)");

                if (active_scr == objects.ami) {
                    ami_selection = (ami_selection == UINT8_MAX)
                        ? 0 : (uint8_t)((ami_selection + 1) % AMI_SELECTION_COUNT);
                    ami_show_selection(ami_selection);
                    ESP_LOGI(TAG, "AMI selection=%u", ami_selection);
                }
            }
            boot_last = curr;
        }

        /* === EXIO9：确认主页面任务 === */
        {
            uint8_t val = xl9555_extio9_read();
            static uint8_t debounce_cnt = 0;
            if (val == 0) { if (debounce_cnt < 5) debounce_cnt++; }
            else          { if (debounce_cnt > 0) debounce_cnt--; }
            uint8_t curr = (debounce_cnt >= 4) ? 0 : 1;

            if (extio9_last == 1 && curr == 0)
            {
                if (active_scr == objects.ami && ami_selection != UINT8_MAX) {
                    if (ami_selection == AMI_SELECTION_COUNT - 1) {
                        inspect_stage = 0;
                        inspect_led_on = false;
                        inspect_blink_tick = xTaskGetTickCount();
                        inspect_show_stage(inspect_stage);
                        loadScreen(SCREEN_ID_INSPECT);
                    } else {
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
            if (val == 0) { if (debounce_cnt < 5) debounce_cnt++; }
            else          { if (debounce_cnt > 0) debounce_cnt--; }
            uint8_t curr = (debounce_cnt >= 4) ? 0 : 1;

            if (extio10_last == 1 && curr == 0 && active_scr == objects.inspect) {
                if (inspect_stage + 1 < INSPECT_STAGE_COUNT) {
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
        if (active_scr == objects.inspect && inspect_stage > 0) {
            TickType_t now = xTaskGetTickCount();
            if ((now - inspect_blink_tick) >= pdMS_TO_TICKS(1000)) {
                /* 全屏刷新可能持锁较久，必须成功取得锁后才更新节拍。 */
                if (lvgl_port_lock(portMAX_DELAY)) {
                    inspect_blink_tick = xTaskGetTickCount();
                    inspect_led_on = !inspect_led_on;
                    if (inspect_led_on) {
                        lv_obj_clear_flag(objects.inspect_state_led, LV_OBJ_FLAG_HIDDEN);
                    } else {
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
    esp_err_t ret;
    ESP_LOGI(TAG, "System starting...");

    ret = nvs_flash_init();     /* 初始化NVS */
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    /* 创建互斥信号量 */
    g_rpm_mutex = xSemaphoreCreateMutex();
    assert(g_rpm_mutex);

    /* 初始化I2C总线 (XL9555 IO扩展器使用) */
    ret = myiic_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C init failed!");
    } else {
        ESP_LOGI(TAG, "I2C initialized (SCL=32, SDA=33)");
    }

    /* 初始化XL9555 IO扩展器 (EXIO8/9/10为输入) */
    ret = xl9555_init(bus_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "XL9555 init failed! EXIO buttons will not work!");
    } else {
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
    lvgl_demo();                /* 运行LVGL例程 */

    /* 初始化CAN (TWAI) */
    ESP_LOGI(TAG, "Initializing CAN (TX:%d, RX:%d)...", CAN_TX_PIN, CAN_RX_PIN);
    if (bsp_can_init(CAN_TX_PIN, CAN_RX_PIN, can_data_callback) == 0)
    {
        ESP_LOGI(TAG, "CAN initialized successfully");
    }
    else
    {
        ESP_LOGW(TAG, "CAN initialization failed, UI will still work");
    }

    /* 创建UI更新任务 */
    xTaskCreatePinnedToCore(
        ui_update_task,
        "ui_update",
        4096,
        NULL,
        3,
        NULL,
        tskNO_AFFINITY
    );
    
    ESP_LOGI(TAG, "System started");
    
}
