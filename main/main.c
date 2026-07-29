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

/* KEY0引脚定义 (EXIO8 - 外部IO扩展器引脚8, 用户根据硬件调整) */
#define KEY0_GPIO_PIN   GPIO_NUM_0

/*
 * 任务选择 (Mission Selection)
 * 对应 AMI 屏幕上的六边形布局
 */
#define MISSION_COUNT 6

static const char *mission_names[MISSION_COUNT] = {
    "车辆任务",     /* cherck - 0 */
    "有人驾驶",     /* youren - 1 (初始选中) */
    "直线加速",     /* line_acc - 2 */
    "八字绕杆",     /* eight - 3 */
    "高速循迹",     /* high_foll - 4 */
    "EBS测试",      /* ebs - 5 */
};

/* 当前选中的任务索引 (初始为1=有人驾驶) */
static uint8_t current_mission = 1;

/*
 * 将LED数组索引映射到screens.h中的objects_t成员
 * 顺序必须与mission_names一致
 */
static lv_obj_t **mission_leds[MISSION_COUNT] = {
    &objects.cherck,     /* 0: 车辆任务 */
    &objects.youren,     /* 1: 有人驾驶 */
    &objects.line_acc,   /* 2: 直线加速 */
    &objects.eight,      /* 3: 八字绕杆 */
    &objects.high_foll,  /* 4: 高速循迹 */
    &objects.ebs,        /* 5: EBS测试 */
};

/** 更新所有任务LED：选中=红色，未选中=绿色 */
static void update_mission_leds(void)
{
    for (int i = 0; i < MISSION_COUNT; i++) {
        lv_obj_t *led = *mission_leds[i];
        if (!led) continue;
        if (i == current_mission) {
            lv_led_set_color(led, lv_color_hex(0xff0000));  /* 红色: 选中 */
        } else {
            lv_led_set_color(led, lv_color_hex(0x09cd4c));  /* 绿色: 未选中 */
        }
    }
}

/** 切换到下一个任务 (循环) */
static void next_mission(void)
{
    current_mission = (current_mission + 1) % MISSION_COUNT;
    ESP_LOGI(TAG, "Mission selected: %s [%d/%d]",
             mission_names[current_mission], current_mission + 1, MISSION_COUNT);
    update_mission_leds();
}

/** 确认当前任务, 跳转到DriverView并设置MISSION标签 */
static void confirm_mission(void)
{
    ESP_LOGI(TAG, "Mission confirmed: %s", mission_names[current_mission]);

    /* 更新DriverView上的MISSION标签 (obj15) */
    if (objects.obj15) {
        char mission_buf[32];
        snprintf(mission_buf, sizeof(mission_buf), "\"MISSION: %s\"", mission_names[current_mission]);
        lv_label_set_text(objects.obj15, mission_buf);
    }

    /* 跳转到DriverView */
    lv_disp_load_scr(objects.driver_view);
}

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
    uint8_t boot_last = 1;          /* BOOT按键上次状态 (1=松开) */
    uint8_t key0_last = 1;          /* KEY0上次状态 (1=松开) */

    ESP_LOGI(TAG, "UI update task started");

    /* 初始选中 "有人驾驶" */
    update_mission_leds();

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

        /* === BOOT按键 (GPIO35) 扫描 === */
        {
            uint8_t val = gpio_get_level(KEY1_GPIO_PIN);
            static uint8_t debounce_cnt = 0;
            if (val == 0) { if (debounce_cnt < 5) debounce_cnt++; }
            else          { if (debounce_cnt > 0) debounce_cnt--; }
            uint8_t curr = (debounce_cnt >= 4) ? 0 : 1;

            if (boot_last == 1 && curr == 0)
            {
                ESP_LOGI(TAG, "BOOT pressed");

                if (active_scr == objects.ami) {
                    /* AMI屏幕: 切换任务 */
                    next_mission();
                } else if (active_scr == objects.driver_view) {
                    /* DriverView: 返回任务选择 */
                    lv_disp_load_scr(objects.ami);
                }
            }
            boot_last = curr;
        }

        /* === KEY0 (EXIO8) 扫描 === */
        {
            uint8_t val = gpio_get_level(KEY0_GPIO_PIN);
            static uint8_t debounce_cnt = 0;
            if (val == 0) { if (debounce_cnt < 5) debounce_cnt++; }
            else          { if (debounce_cnt > 0) debounce_cnt--; }
            uint8_t curr = (debounce_cnt >= 4) ? 0 : 1;

            if (key0_last == 1 && curr == 0)
            {
                ESP_LOGI(TAG, "KEY0 pressed");

                if (active_scr == objects.ami) {
                    /* AMI屏幕: 确认当前任务, 跳转到DriverView */
                    confirm_mission();
                }
            }
            key0_last = curr;
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

    /* 初始化LVGL显示 */
    lvgl_demo();                /* 运行LVGL例程 */

    /* 初始化KEY1 (BOOT按键, GPIO35, 上拉输入) */
    gpio_config_t key1_cfg = {
        .pin_bit_mask = 1ULL << KEY1_GPIO_PIN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&key1_cfg);
    ESP_LOGI(TAG, "KEY1 initialized on GPIO%d", KEY1_GPIO_PIN);

    /* 初始化KEY0 (EXIO8, 上拉输入) */
    gpio_config_t key0_cfg = {
        .pin_bit_mask = 1ULL << KEY0_GPIO_PIN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&key0_cfg);
    ESP_LOGI(TAG, "KEY0 initialized on GPIO%d (EXIO8)", KEY0_GPIO_PIN);

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
