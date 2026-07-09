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
    esptim_int_init(1000000);
    while (1)
    {

        // uint8_t key = key_scan(0); /* 获取键值 */
        // switch (key)
        // {
        // case BOOT_PRES:
        // {
        //     LED_TOGGLE();
        //     break;
        // }
        // default:
        // {
        //     break;
        // }
        // }
        // /* BOOT 被按下 */
        // /* LED 状态翻转 */
        // vTaskDelay(10);
        // /* 获取环形缓冲区数据长度 */
        // uart_get_buffered_data_len(USART_UX, (size_t *)&len);
        // if (len > 0)
        // {
        //     memset(data, 0, RX_BUF_SIZE);
        //     printf("\n 您发送的消息为:\n");
        //     uart_read_bytes(USART_UX, data, len, 100);
        //     uart_write_bytes(USART_UX,
        //                      (const char *)data,
        //                      strlen((const char *)data));
        //     vTaskDelay(10);
        // }
        // else
        // {
        //     /* 判断数据长度 */
        //     /* 对缓冲区清零 */
        //     /* 读数据 */
        //     /* 写数据 */
        //     vTaskDelay(10);
        // }
        // times++;
        // if (times % 5000 == 0)
        // {
        //     printf("\n 串口实验\n");
        
        // }
        // if (times % 200 == 0)
        // {
        //     printf("请输入数据，以回车键结束\n");
        // }
        // if (times % 30 == 0)
        // {
        //     LED_TOGGLE();
        // }
    }
}
