#include "exti.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "BSP/LED/led.h"
static void IRAM_ATTR exit_gpio_isr_handler(void *arg)
{
uint32_t gpio_num = (uint32_t) arg;
if (gpio_num == BOOT_INT_GPIO_PIN)
{
LED_TOGGLE();
}
}


void exit_init(void)
{
gpio_config_t gpio_init_struct;
/* 配置 BOOT 引脚和外部中断 */
gpio_init_struct.mode = GPIO_MODE_INPUT;
/* 选择为输入模式 */
gpio_init_struct.pull_up_en = GPIO_PULLUP_ENABLE;
/* 上拉使能 */
gpio_init_struct.pull_down_en = GPIO_PULLDOWN_DISABLE;
/* 下拉失能 */
gpio_init_struct.intr_type = GPIO_INTR_NEGEDGE;
/* 下降沿触发 */
gpio_init_struct.pin_bit_mask = 1ull << BOOT_INT_GPIO_PIN;//告诉要配置的引脚
gpio_config(&gpio_init_struct);
/* 配置使能 */
/* 注册中断服务 */
gpio_install_isr_service(ESP_INTR_FLAG_EDGE);
/* 设置 GPIO 的中断回调函数 */
gpio_isr_handler_add(BOOT_INT_GPIO_PIN, exit_gpio_isr_handler,
(void*) BOOT_INT_GPIO_PIN);
/* 使能 GPIO 模块中断信号 */
gpio_intr_enable(BOOT_INT_GPIO_PIN);
}