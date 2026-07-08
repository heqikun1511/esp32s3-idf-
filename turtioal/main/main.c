#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "esp_err.h"
#include "BSP/LED/led.h"
#include "BSP/KEY/key.h"
#include "BSP/EXIT/exti.h"

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    /* 初始化 NVS */
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    led_init();
    key_init(); /* 初始化 LED */
    exit_init(); /* 初始化外部中断 */

    while (1)
    {
        uint8_t key = key_scan(0); /* 获取键值 */
        switch (key)
        {
        case BOOT_PRES:
        {
            LED_TOGGLE();
            break;
        }
        default:
        {
            break;
        }
        }
        /* BOOT 被按下 */
        /* LED 状态翻转 */
        vTaskDelay(10);
    }
}