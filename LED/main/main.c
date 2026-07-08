#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"


#define LED_GPIO_PORT       GPIO_NUM_1      /* LED连接到GPIO0 */
#define LED_GPIO_PIN        LED_GPIO_PORT

void app_main(void)
{

while(1)
{/* 输入输出模式 */
/* 使能上拉 */
/* 失能下拉 */
/* 设置的引脚的位掩码*/
/* 配置 GPIO */
gpio_config_t gpio_init_struct = {0};
gpio_init_struct.intr_type = GPIO_INTR_DISABLE;
gpio_init_struct.mode = GPIO_MODE_INPUT_OUTPUT;
gpio_init_struct.pull_up_en = GPIO_PULLUP_ENABLE;
gpio_init_struct.pull_down_en = GPIO_PULLDOWN_DISABLE;
gpio_init_struct.pin_bit_mask = 1ull << LED_GPIO_PIN;
gpio_config(&gpio_init_struct);/* 失能引脚中断 */

gpio_set_level(LED_GPIO_PORT, !gpio_get_level(LED_GPIO_PORT));
vTaskDelay(1000 / portTICK_PERIOD_MS);/* 延时1秒 */

}
}