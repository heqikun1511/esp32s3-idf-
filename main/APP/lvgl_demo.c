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
#include "lvgl.h"
#include "ui/ui.h"

/* MIPI竖屏旋转标志和PSRAM旋转缓冲区 */
static bool g_need_rotate = false;
static lv_color_t *g_rot_buf = NULL;

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

    while (1)
    {
        lv_timer_handler();             /* LVGL计时器 */
        vTaskDelay(pdMS_TO_TICKS(10));  /* 延时10毫秒 */
    }
}

/**
 * @brief       初始化并注册显示设备
 * @param       无
 * @retval      lvgl显示设备指针
 */
lv_disp_t *lv_port_disp_init(void)
{
    void *lcd_buffer[2];        /* 指向屏幕双缓存 */

    /* 初始化显示设备LCD */
    lcd_init();                /* LCD初始化 */

    /*-----------------------------
     * 创建一个绘图缓冲区
     *----------------------------*/
    /**
     * LVGL 需要一个缓冲区用来绘制小部件
     * 随后，这个缓冲区的内容会通过显示设备的 'flush_cb'(显示设备刷新函数) 复制到显示设备上
     * 这个缓冲区的大小需要大于显示设备一行的大小
     *
     * 这里有3种缓冲配置:
     * 1. 单缓冲区:
     *      LVGL 会将显示设备的内容绘制到这里，并将他写入显示设备。
     *
     * 2. 双缓冲区:
     *      LVGL 会将显示设备的内容绘制到其中一个缓冲区，并将他写入显示设备。
     *      需要使用 DMA 将要显示在显示设备的内容写入缓冲区。
     *      当数据从第一个缓冲区发送时，它将使 LVGL 能够将屏幕的下一部分绘制到另一个缓冲区。
     *      这样使得渲染和刷新可以并行执行。
     *
     * 3. 全尺寸双缓冲区
     *      设置两个屏幕大小的全尺寸缓冲区，并且设置 disp_drv.full_refresh = 1。
     *      这样，LVGL将始终以 'flush_cb' 的形式提供整个渲染屏幕，您只需更改帧缓冲区的地址。
     */
    /* 使用双缓冲 */
    if (lcddev.id <= 0x7084)    /* RGB屏触摸屏 */
    {
        ESP_ERROR_CHECK(esp_lcd_rgb_panel_get_frame_buffer(lcddev.lcd_panel_handle, 2, &lcd_buffer[0], &lcd_buffer[1]));
    }
    else                        /* MIPI屏触摸屏 */
    {
        ESP_ERROR_CHECK(esp_lcd_dpi_panel_get_frame_buffer(lcddev.lcd_panel_handle, 2, &lcd_buffer[0], &lcd_buffer[1])); 
    }
    size_t draw_buffer_sz = lcddev.width * lcddev.height;                          /* 计算绘画缓冲区大小(像素数) */

    /* 初始化显示缓冲区 */
    static lv_disp_draw_buf_t disp_buf;                                                 /* 保存显示缓冲区信息的结构体 */
    lv_disp_draw_buf_init(&disp_buf, lcd_buffer[0], lcd_buffer[1], draw_buffer_sz);     /* 初始化显示缓冲区 */
    
    /* 在LVGL中注册显示设备 */
    static lv_disp_drv_t disp_drv;      /* 显示设备的描述符(HAL要注册的显示驱动程序、与显示交互并处理与图形相关的结构体、回调函数) */
    lv_disp_drv_init(&disp_drv);        /* 初始化显示设备 */
    
    /* 设置显示设备的分辨率 
     * MIPI屏为竖屏(1080x1920)，通过在flush回调中手动旋转实现横屏(1920x1080)
     */
    g_need_rotate = (lcddev.height > lcddev.width);
    if (g_need_rotate) {
        disp_drv.hor_res = lcddev.height;               /* 逻辑横屏: 1920 */
        disp_drv.ver_res = lcddev.width;                /* 逻辑横屏: 1080 */
        ESP_LOGI("lvgl_demo", "MIPI portrait, logical landscape: %dx%d", disp_drv.hor_res, disp_drv.ver_res);
        /* 分配PSRAM旋转缓冲区 */
        if (!g_rot_buf) {
            g_rot_buf = (lv_color_t *)heap_caps_malloc(lcddev.width * lcddev.height * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
            ESP_LOGI("lvgl_demo", "Rotate buffer: %p size=%d", g_rot_buf, lcddev.width * lcddev.height * (int)sizeof(lv_color_t));
        }
    } else {
        disp_drv.hor_res = lcddev.width;                /* 水平分辨率 */
        disp_drv.ver_res = lcddev.height;               /* 垂直分辨率 */
        ESP_LOGI("lvgl_demo", "RGB landscape: %dx%d", lcddev.width, lcddev.height);
    }

    /* 用来将缓冲区的内容复制到显示设备 */
    disp_drv.flush_cb = lvgl_disp_flush_cb;             /* 设置刷新回调函数 */

    /* 设置显示缓冲区 */
    disp_drv.draw_buf = &disp_buf;                      /* 设置绘画缓冲区 */

    disp_drv.user_data = lcddev.lcd_panel_handle;       /* 传递屏幕控制句柄 */
    disp_drv.full_refresh = 1;                          /* 全刷新 */
    /* 注册显示设备 */
    return lv_disp_drv_register(&disp_drv);                    
}

/**
 * @brief       初始化并注册输入设备
 * @param       无
 * @retval      lvgl输入设备指针
 */
lv_indev_t *lv_port_indev_init(void)
{
    /* 初始化触摸屏 */
    tp_dev.init();

    /* 初始化输入设备 */
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);

    /* 配置输入设备类型 */
    indev_drv.type = LV_INDEV_TYPE_POINTER;

    /* 设置输入设备读取回调函数 */
    indev_drv.read_cb = touchpad_read;

    /* 在LVGL中注册驱动程序，并保存创建的输入设备对象 */
    return lv_indev_drv_register(&indev_drv);
}

/**
* @brief        将内部缓冲区的内容刷新到显示屏上的特定区域
* @note         可以使用 DMA 或者任何硬件在后台加速执行这个操作
*               但是，需要在刷新完成后调用函数 'lv_disp_flush_ready()'
* @param        disp_drv : 显示设备
* @param        area : 要刷新的区域，包含了填充矩形的对角坐标
* @param        color_map : 颜色数组
* @retval       无
*/
static void lvgl_disp_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)drv->user_data;

    if (g_need_rotate) {
        /* 手动旋转: 将逻辑横屏(1920x1080)旋转为物理竖屏(1080x1920) */
        int32_t logical_w = lv_area_get_width(area);
        int32_t logical_h = lv_area_get_height(area);
        lv_coord_t phys_w = lcddev.width;   /* 1080 */
        lv_coord_t phys_h = lcddev.height;  /* 1920 */

        for (int32_t ly = area->y1; ly <= area->y2; ly++) {
            for (int32_t lx = area->x1; lx <= area->x2; lx++) {
                /* 逻辑坐标(lx, ly) → 物理坐标(px, py), 90度旋转 */
                /* 注意: 用 logical_w (1920) 计算 py, 不是 phys_w (1080) */
                lv_coord_t px = ly;
                lv_coord_t py = logical_w - 1 - lx;
                if (px >= 0 && px < phys_w && py >= 0 && py < phys_h) {
                    g_rot_buf[py * phys_w + px] = color_map[(ly - area->y1) * logical_w + (lx - area->x1)];
                }
            }
        }
        esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, phys_w, phys_h, (uint8_t *)g_rot_buf);
    } else {
        /* 不旋转，直接刷新 */
        esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, (uint8_t *)color_map);
    }

    /* 重要!!! 通知图形库，已经刷新完毕了 */
    lv_disp_flush_ready(drv);
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
void touchpad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    static lv_coord_t last_x = 0;
    static lv_coord_t last_y = 0;

    /* 保存按下的坐标和状态 */
    if(touchpad_is_pressed())
    {
        touchpad_get_xy(&last_x, &last_y);  /* 在触摸屏被按下的时候读取 x、y 坐标 */
        /* 横屏旋转: 触摸物理坐标→逻辑坐标 */
        if (g_need_rotate) {
            lv_coord_t tmp = last_x;
            last_x = last_y;
            last_y = lcddev.height - 1 - tmp;  /* 用height(1920)不是width(1080) */
        }
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
