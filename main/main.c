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
#include "lvgl.h"
#include "lvgl_demo.h"
#include "bsp_can.h"
#include "ui/ui.h"
#include "ui/screens.h"
#include "ui/images.h"

static const char *TAG = "MAIN";

/* 当前转速值(由CAN回调更新, 由LVGL任务读取) */
static volatile int g_current_rpm = 0;
/* 互斥信号量, 保护转速数据 */
static SemaphoreHandle_t g_rpm_mutex = NULL;

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

            /* 更新 RPM 标签 */
            if (objects.rpm)
            {
                snprintf(buf, sizeof(buf), "%04d", abs(current_rpm));
                lv_label_set_text_static(objects.rpm, buf);
            }

            /* 更新车速显示 (将转速按比例映射到0-120范围, 可根据实际调整) */
            int speed_val = (abs(current_rpm) * 120) / 10000;
            if (speed_val > 120) speed_val = 120;

            if (speed_val != last_speed)
            {
                last_speed = speed_val;

                /* 更新车速标签 */
                if (objects.km_h_value)
                {
                    snprintf(buf, sizeof(buf), "%03d", speed_val);
                    lv_label_set_text_static(objects.km_h_value, buf);
                }
            }
        }

        /* 每100ms检查一次 */
        vTaskDelay(pdMS_TO_TICKS(100));
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
