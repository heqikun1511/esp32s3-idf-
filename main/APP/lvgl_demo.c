/**
 ****************************************************************************************************
 * @file        lvgl_demo.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       LVGL V8移植 实验
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 ESP32-P4 开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 ****************************************************************************************************
 */

#include "lvgl_demo.h"
#include "lcd.h"
#include "touch.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "ui/ui.h"

/* MIPI竖屏旋转和PSRAM旋转缓冲区 */
static bool g_need_rotate = false;
static uint16_t *g_rot_buf = NULL;

/* PARTIAL渲染模式的缓冲区高度(行数) */
#define PARTIAL_BUF_HEIGHT  50

/**
 * @brief       LVGL定时器处理任务(独立任务,避免阻塞app_main)
 * @param       arg : 未使用
 * @retval      无
 */
static void lvgl_timer_task(void *arg)
{
    ESP_LOGI("lvgl_demo", "LVGL timer task started");

    while (1)
    {
        lv_timer_handler();             /* LVGL计时器处理 */
        vTaskDelay(pdMS_TO_TICKS(10));  /* 延时10毫秒 */
    }
}

/**
 * @brief       lvgl_demo入口函数
 * @param       无
 * @retval      无
 */
void lvgl_demo(void)
{
    lv_init();              /* 初始化LVGL图形库 */
    lv_port_disp_init();    /* lvgl显示接口初始化,放在lv_init()的后面 */
    lv_port_indev_init();   /* lvgl输入接口初始化,放在lv_init()的后面 */

    /* 为LVGL提供时基单元 */
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &increase_lvgl_tick,    /* 设置定时器回调 */
        .name = "lvgl_tick"                 /* 定时器名称 */
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));     /* 创建定时器 */
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, 1 * 1000));           /* 启动定时器 */

    /* EEZ Studio UI */
    ui_init();

    /* 创建独立任务运行 lv_timer_handler 循环, 避免阻塞 app_main
     * 注意: LVGL软件渲染需要较大栈空间, 8192为安全值
     */
    xTaskCreatePinnedToCore(
        lvgl_timer_task,
        "lvgl_timer",
        8192,
        NULL,
        2,              /* 较低优先级, 让CAN和UI更新任务优先 */
        NULL,
        tskNO_AFFINITY
    );
}

/**
 * @brief       初始化并注册显示设备 (LVGL v9)
 * @param       无
 * @retval      lvgl显示设备指针
 */
lv_display_t *lv_port_disp_init(void)
{
    void *lcd_buffer[2];        /* 指向屏幕双缓存 */

    /* 初始化显示设备LCD */
    lcd_init();                /* LCD初始化 */

    /* 获取硬件帧缓冲 */
    if (lcddev.id <= 0x7084)    /* RGB屏触摸屏 */
    {
        ESP_ERROR_CHECK(esp_lcd_rgb_panel_get_frame_buffer(lcddev.lcd_panel_handle, 2, &lcd_buffer[0], &lcd_buffer[1]));
    }
    else                        /* MIPI屏触摸屏 */
    {
        ESP_ERROR_CHECK(esp_lcd_dpi_panel_get_frame_buffer(lcddev.lcd_panel_handle, 2, &lcd_buffer[0], &lcd_buffer[1])); 
    }
    
    /* 创建显示设备: MIPI竖屏用逻辑横屏 + 手动旋转 + PSRAM缓冲 */
    g_need_rotate = (lcddev.height > lcddev.width);
    lv_display_t *disp;
    size_t buf_size;
    
    if (g_need_rotate) {
        /* 逻辑分辨率 = 横屏 1920x1080 */
        disp = lv_display_create(lcddev.height, lcddev.width);
        /* PSRAM旋转缓冲区 (全屏大小, 用于旋转) */
        if (!g_rot_buf) {
            g_rot_buf = (uint16_t *)heap_caps_malloc(lcddev.width * lcddev.height * sizeof(uint16_t), MALLOC_CAP_SPIRAM);
            ESP_LOGI("lvgl_demo", "Rotate buffer: %p", g_rot_buf);
        }
        buf_size = lcddev.height * lcddev.width * sizeof(uint16_t);
        ESP_LOGI("lvgl_demo", "MIPI portrait, logical landscape: %dx%d, FULL buf=%dKB",
                 lcddev.height, lcddev.width, buf_size / 1024);
    } else {
        disp = lv_display_create(lcddev.width, lcddev.height);
        buf_size = lcddev.width * lcddev.height * sizeof(uint16_t);
        ESP_LOGI("lvgl_demo", "RGB landscape: %dx%d, FULL buf=%dKB",
                 lcddev.width, lcddev.height, buf_size / 1024);
    }
    
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(disp, lvgl_disp_flush_cb);
    lv_display_set_user_data(disp, lcddev.lcd_panel_handle);
    /* FULL模式: 全屏渲染, 配合硬件双缓冲, 无变化时不刷新 */
    lv_display_set_buffers(disp, lcd_buffer[0], lcd_buffer[1], buf_size, LV_DISPLAY_RENDER_MODE_FULL);
    
    return disp;                    
}

/**
 * @brief       初始化并注册输入设备 (LVGL v9)
 * @param       无
 * @retval      lvgl输入设备指针
 */
lv_indev_t *lv_port_indev_init(void)
{
    /* 初始化触摸屏 */
    tp_dev.init();

    /* 创建输入设备 */
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touchpad_read);

    return indev;
}

/**
* @brief        将内部缓冲区的内容刷新到显示屏上的特定区域 (LVGL v9)
* @note         LVGL v9使用lv_display_set_rotation自动旋转，flush_cb收到的已经是正确方向的像素
* @param        disp : 显示设备
* @param        area : 要刷新的区域
* @param        px_map : 像素数据 (uint8_t*)
* @retval       无
*/
static void lvgl_disp_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)lv_display_get_user_data(disp);

    if (g_need_rotate) {
        /* 手动旋转: 逻辑横屏(1920x1080) → 物理竖屏(1080x1920)
         * 逻辑坐标(lx, ly) → 物理坐标(px=ly, py=logical_total_w-1-lx)
         * 仅旋转area指定的脏区域, 然后只刷新旋转后的物理区域
         */
        uint16_t *src = (uint16_t *)px_map;
        lv_coord_t logical_total_w = lv_display_get_horizontal_resolution(disp);
        lv_coord_t area_w = lv_area_get_width(area);
        lv_coord_t phys_w = lcddev.width;
        lv_coord_t phys_h = lcddev.height;

        for (lv_coord_t ly = area->y1; ly <= area->y2; ly++) {
            for (lv_coord_t lx = area->x1; lx <= area->x2; lx++) {
                lv_coord_t px = ly;
                lv_coord_t py = logical_total_w - 1 - lx;
                if (px >= 0 && px < phys_w && py >= 0 && py < phys_h) {
                    g_rot_buf[py * phys_w + px] = src[(ly - area->y1) * area_w + (lx - area->x1)];
                }
            }
        }

        /* 计算旋转后脏区域在物理屏幕上的包围盒
         * 逻辑rect (x1,y1)-(x2,y2) → 物理rect:
         *   phys_x1 = y1, phys_x2 = y2+1
         *   phys_y1 = logical_total_w-1-x2, phys_y2 = logical_total_w-1-x1+1
         */
        lv_coord_t flush_x1 = area->y1;
        lv_coord_t flush_x2 = area->y2 + 1;
        lv_coord_t flush_y1 = logical_total_w - 1 - area->x2;
        lv_coord_t flush_y2 = logical_total_w - 1 - area->x1 + 1;

        /* 限制在物理屏幕范围内 */
        if (flush_x1 < 0) flush_x1 = 0;
        if (flush_x2 > phys_w) flush_x2 = phys_w;
        if (flush_y1 < 0) flush_y1 = 0;
        if (flush_y2 > phys_h) flush_y2 = phys_h;

        esp_lcd_panel_draw_bitmap(panel_handle, flush_x1, flush_y1, flush_x2, flush_y2,
                                  (uint8_t *)g_rot_buf);
    } else {
        /* 非旋转: 只刷新变化的区域 */
        esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1,
                                  area->x2 + 1, area->y2 + 1, px_map);
    }
    lv_display_flush_ready(disp);
}

/**
 * @brief       告诉LVGL运行时间
 * @param       arg : 传入参数(未用到)
 * @retval      无
 */
static void increase_lvgl_tick(void *arg)
{
    /* 告诉LVGL已经过了多少毫秒 */
    lv_tick_inc(1);
}

/**
 * @brief       获取触摸屏设备的状态
 * @param       无
 * @retval      返回触摸屏设备是否被按下
 */
static bool touchpad_is_pressed(void)
{
    tp_dev.scan(0);     /* 触摸按键扫描 */

    if (tp_dev.sta & TP_PRES_DOWN)
    {
        return true;
    }

    return false;
}


/**
 * @brief       在触摸屏被按下的时候读取 x、y 坐标
 * @param       x   : x坐标的指针
 * @param       y   : y坐标的指针
 * @retval      无
 */
static void touchpad_get_xy(lv_coord_t *x, lv_coord_t *y)
{
    (*x) = tp_dev.x[0];
    (*y) = tp_dev.y[0];
}

/**
 * @brief       图形库的触摸屏读取回调函数
 * @param       indev_drv   : 触摸屏设备
 * @param       data        : 输入设备数据结构体
 * @retval      无
 */
void touchpad_read(lv_indev_t *indev, lv_indev_data_t *data)
{
    static lv_coord_t last_x = 0;
    static lv_coord_t last_y = 0;

    /* 保存按下的坐标和状态 */
    if(touchpad_is_pressed())
    {
        touchpad_get_xy(&last_x, &last_y);  /* 在触摸屏被按下的时候读取 x、y 坐标 */
        data->state = LV_INDEV_STATE_PR;
    } 
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }

    /* 设置最后按下的坐标 */
    data->point.x = last_x;
    data->point.y = last_y;
}
