
#ifndef __LED_H
#define __LED_H

#include "driver/gpio.h"

/* LED引脚定义 */
#define LED_GPIO_PORT       GPIO_NUM_0      /* LED连接到GPIO0 */

/* LED端口操作封装 */
#define LED_TOGGLE()        gpio_set_level(LED_GPIO_PORT, !gpio_get_level(LED_GPIO_PORT))

/* 函数声明 */
void led_init(void);                        /* 初始化LED */

#endif
