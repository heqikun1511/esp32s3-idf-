/**
 ******************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       LVGL V8移植 实验
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ******************************************************************************
 * @attention
 * 
 * 实验平台:正点原子 ESP32-P4 开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 ******************************************************************************
 */

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
#include "ui.h"

static const char *TAG = "MAIN";

/* 当前转速值(由CAN回调更新, 由LVGL任务读取) */
static volatile int g_current_rpm = 0;
/* 互斥信号量, 保护转速数据 */
static SemaphoreHandle_t g_rpm_mutex = NULL;

/* KEY1引脚定义 (BOOT按键, 低电平有效) */
#define KEY1_GPIO_PIN   GPIO_NUM_35

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
    uint8_t key1_last = 1;          /* KEY1上次状态 (1=松开) */
    uint8_t current_screen_idx = 0; /* 当前屏幕索引 (0~1) */
    lv_obj_t * screen_list[] = {
        objects.driver_view,
        objects.autonomous
    };
    const char * screen_names[] = {
        "DriverView",
        "Autonomous"
    };
    const uint8_t screen_count = sizeof(screen_list) / sizeof(screen_list[0]);
    uint32_t debug_tick = 0;        /* 调试日志计时 */

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

            /* 更新车速显示 (将转速按比例映射到0-120范围, 可根据实际调整) */
            int speed_val = (abs(current_rpm) * 120) / 10000;
            if (speed_val > 120) speed_val = 120;

            if (speed_val != last_speed)
            {
                last_speed = speed_val;

                /* 更新EEZ车速标签 */
                if (objects.speed_label)
                {
                    snprintf(buf, sizeof(buf), "%03d", speed_val);
                    lv_label_set_text_static(objects.speed_label, buf);
                }
            }
        }

        /* === KEY1 按键扫描 (低电平有效, 简单消抖) === */
        uint8_t key1_val = gpio_get_level(KEY1_GPIO_PIN);
        static uint8_t key1_debounce_cnt = 0;

        /* 连续采样: 5次中至少4次一致才判定为有效 */
        if (key1_val == 0) {
            if (key1_debounce_cnt < 5) key1_debounce_cnt++;
        } else {
            if (key1_debounce_cnt > 0) key1_debounce_cnt--;
        }

        uint8_t key1_curr = (key1_debounce_cnt >= 4) ? 0 : 1;

        /* 下降沿检测 (按下) */
        if (key1_last == 1 && key1_curr == 0)
        {
            ESP_LOGI(TAG, "KEY1 pressed! (raw=%d, debounce=%d)", key1_val, key1_debounce_cnt);

            /* 切换到下一个屏幕 (循环) */
            current_screen_idx = (current_screen_idx + 1) % screen_count;
            ESP_LOGI(TAG, "Switching to %s [%d/%d]",
                     screen_names[current_screen_idx],
                     current_screen_idx + 1, screen_count);
            lv_disp_load_scr(screen_list[current_screen_idx]);
        }
        key1_last = key1_curr;

        /* 每5秒打印一次按键状态用于调试 */
        debug_tick++;
        if (debug_tick >= 250)  /* 5秒 = 250 * 20ms */
        {
            debug_tick = 0;
            ESP_LOGI(TAG, "KEY1 state: raw=%d, debounce=%d, stable=%s",
                     gpio_get_level(KEY1_GPIO_PIN), key1_debounce_cnt,
                     key1_curr == 0 ? "PRESSED" : "RELEASED");
        }

        /* 每20ms检查一次 (兼顾按键响应和CPU占用) */
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

    /* 初始化LVGL显示 */
    lvgl_demo();                /* 运行LVGL例程 */

    /* 初始化KEY1 (BOOT按键, GPIO0, 上拉输入) */
    gpio_config_t key1_cfg = {
        .pin_bit_mask = 1ULL << KEY1_GPIO_PIN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&key1_cfg);
    ESP_LOGI(TAG, "KEY1 initialized on GPIO%d", KEY1_GPIO_PIN);

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
