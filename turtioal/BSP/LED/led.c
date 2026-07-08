/**
 ******************************************************************************
 * @file     led.c
 * @author   正点原子团队(ALIENTEK)
 * @version  V1.0
 * @date     2023-08-26
 * @brief    LED驱动代码
 * @license  Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ******************************************************************************
 * @attention
 *
 * 实验平台:正点原子 ESP32-S3 开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 ******************************************************************************
 */

#include "led.h"

/**
 * @brief       初始化LED
 * @param       无
 * @retval      无
 */
void led_init(void)
{
    gpio_config_t gpio_init_struct = {0};

    gpio_init_struct.intr_type = GPIO_INTR_DISABLE;     /* 禁止中断 */
    gpio_init_struct.mode = GPIO_MODE_OUTPUT;           /* 输出模式 */
    gpio_init_struct.pin_bit_mask = 1ULL << LED_GPIO_PORT; /* 要配置的引脚 */
    gpio_init_struct.pull_down_en = GPIO_PULLDOWN_DISABLE; /* 禁止下拉 */
    gpio_init_struct.pull_up_en = GPIO_PULLUP_DISABLE;     /* 禁止上拉 */

    gpio_config(&gpio_init_struct);                     /* 配置GPIO */

    gpio_set_level(LED_GPIO_PORT, 1);                   /* 默认关闭LED（高电平） */
}
