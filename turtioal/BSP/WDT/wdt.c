#include "BSP/WDT/wdt.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_system.h"

static const char *TAG = "WDT";
static esp_timer_handle_t wdt_timer_handle = NULL;  /* 看门狗定时器句柄 */

/**
 * @brief       初始化看门狗定时器
 * @param       timeout_us  超时时间（微秒）
 *              例如: 1秒 = 1000000μs
 * @note        若在 timeout_us 时间内未调用 wdt_feed() 喂狗，系统将自动复位
 */
void wdt_init(uint64_t timeout_us)
{
    esp_timer_create_args_t wdt_args = {
        .callback = &wdt_isr_handler,
        .arg = NULL,
        .name = "wdt_timer"
    };

    ESP_ERROR_CHECK(esp_timer_create(&wdt_args, &wdt_timer_handle));
    ESP_ERROR_CHECK(esp_timer_start_periodic(wdt_timer_handle, timeout_us));

    ESP_LOGI(TAG, "看门狗初始化完成，超时时间: %llu μs", timeout_us);
}

/**
 * @brief       喂狗 — 重新启动看门狗定时器
 * @param       timeout_us  新的超时时间（微秒）
 * @note        每次喂狗会重置计时器，防止系统复位
 */
void wdt_feed(uint64_t timeout_us)
{
    if (wdt_timer_handle != NULL) {
        ESP_ERROR_CHECK(esp_timer_restart(wdt_timer_handle, timeout_us));
    }
}

/**
 * @brief       看门狗中断回调函数（IRAM）
 * @param       arg     无参数传入
 * @note        若未及时喂狗，系统将执行 esp_restart() 进行复位
 */
void IRAM_ATTR wdt_isr_handler(void *arg)
{
    ESP_EARLY_LOGE(TAG, "看门狗超时! 系统即将复位...");
    esp_restart(); /* 若没有及时进行喂狗，那么芯片将一直进行复位 */
}