#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "esp_err.h"
#include "BSP/LED/led.h"
#include "BSP/KEY/key.h"
#include "BSP/EXIT/exti.h"
#include "BSP/UART/uart.h"
#include "driver/uart.h"
#include "BSP/esptim/esptim.h"
#include "BSP/WDT/wdt.h"

void app_main(void)
{
    // uint8_t len = 0;
    // uint16_t times=0;
    //  unsigned char data[RX_BUF_SIZE];
    uart_init(115200);
    esp_err_t ret = nvs_flash_init();
    /* 初始化 NVS */
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    led_init();
    key_init();  /* 初始化 LED */
    exit_init(); /* 初始化外部中断 */
    led_init();
    // esptim_int_init(1000000);
    wdt_init(1000000); /* 初始化看门狗定时器，设置超时时间为 1 秒（1000000 μs） */

    while (1)
    {

        if (key_scan(0) == BOOT_PRES)
        {
            wdt_feed(1000000); /* 按下 BOOT 键喂狗，重新计时 1 秒 */
        }
        /* 如果 BOOT 按下则喂狗 */
        /* 喂狗 */
        /* LED 闪烁 */
        vTaskDelay(10);
    }
}
