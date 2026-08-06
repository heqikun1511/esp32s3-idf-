

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

static const char *TAG = "MAIN";

/* 当前转速值(由CAN回调更新, 由LVGL任务读取) */
static volatile int g_current_rpm = 0;
/* 互斥信号量, 保护转速数据 */
static SemaphoreHandle_t g_rpm_mutex = NULL;

/* BOOT按键 (物理BOOT键, 直接GPIO, 低电平有效) */
#define BOOT_GPIO_PIN   GPIO_NUM_35

/* KEY0和KEY1通过XL9555 IO扩展器连接 (见原理图) */
/* KEY0 = EXIO8 (IO1_0), KEY1 = EXIO7 (IO0_7) */

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
    uint8_t boot_last = 1;          /* BOOT按键 (GPIO35) 上次状态 */
    uint8_t key1_last = 1;          /* KEY1 (EXIO7) 上次状态 */

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

        /* === BOOT按键 (GPIO35, 物理BOOT键) 扫描 === */
        {
            uint8_t val = gpio_get_level(BOOT_GPIO_PIN);
            static uint8_t debounce_cnt = 0;
            if (val == 0) { if (debounce_cnt < 5) debounce_cnt++; }
            else          { if (debounce_cnt > 0) debounce_cnt--; }
            uint8_t curr = (debounce_cnt >= 4) ? 0 : 1;

            if (boot_last == 1 && curr == 0)
            {
                ESP_LOGI(TAG, "BOOT pressed");

                if (active_scr == objects.ami) {
                    /* AMI → AUTONOMOUS */
                    loadScreen(SCREEN_ID_AUTONOMOUS);
                } else if (active_scr == objects.autonomous) {
                    /* AUTONOMOUS → DRIVER_VIEW */
                    loadScreen(SCREEN_ID_DRIVER_VIEW);
                } else {
                    /* DRIVER_VIEW (或其它) → AMI */
                    loadScreen(SCREEN_ID_AMI);
                }
            }
            boot_last = curr;
        }

        /* === KEY1 (EXIO7, XL9555) 扫描 — 切换 DriverView/Autonomous === */
        {
            uint8_t val = xl9555_key1_read();
            static uint8_t debounce_cnt = 0;
            if (val == 0) { if (debounce_cnt < 5) debounce_cnt++; }
            else          { if (debounce_cnt > 0) debounce_cnt--; }
            uint8_t curr = (debounce_cnt >= 4) ? 0 : 1;

            if (key1_last == 1 && curr == 0)
            {
                ESP_LOGI(TAG, "KEY1 pressed (EXIO7)");

                if (active_scr == objects.autonomous) {
                    lv_disp_load_scr(objects.driver_view);
                } else {
                    lv_disp_load_scr(objects.autonomous);
                }
            }
            key1_last = curr;
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

    /* 初始化XL9555 IO扩展器 (KEY0=EXIO8, KEY1=EXIO7) */
    ret = xl9555_init(bus_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "XL9555 init failed! KEY1 will not work!");
    } else {
        ESP_LOGI(TAG, "XL9555 initialized (KEY1=EXIO7, KEY0=EXIO8)");
    }

    /* 初始化BOOT按键 (GPIO35, 物理BOOT键) */
    gpio_config_t boot_cfg = {
        .pin_bit_mask = 1ULL << BOOT_GPIO_PIN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&boot_cfg); 
    ESP_LOGI(TAG, "BOOT button initialized on GPIO%d", BOOT_GPIO_PIN);

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
