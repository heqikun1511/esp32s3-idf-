#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_check.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/rmt_tx.h"
#include <string.h>
#include "ws2815_rmt_encoder.h"
#include "ws2815_test.h"

/*
 * 临时硬件测试配置。
 * 用户的 WS2815B 数据输入脚 DI 接在扩展排针标注为 GPIO3 的引脚上。
 * 备份输入脚 BI 接 GND。这是 3.3V 直驱的信号电平测试。
 */
#define WS2815_TEST_GPIO          GPIO_NUM_3              /* 数据输出脚 -> WS2815B 的 DI */
#define WS2815_TEST_LED_COUNT     16                       /* 本次只点一颗灯珠 */
#define WS2815_RMT_RESOLUTION_HZ  (10 * 1000 * 1000)      /* RMT 时钟 10MHz, 即 1 tick = 0.1us */

static const char *TAG = "WS2815_TEST";

/**
 * @brief  向 WS2815B 发送一帧数据(点亮一颗灯珠)
 * @param  channel RMT 发送通道(已使能)
 * @param  encoder WS2815B 编码器
 * @param  red     红分量 0-255
 * @param  green   绿分量 0-255
 * @param  blue    蓝分量 0-255
 *
 * 说明: WS2815B 的字节顺序是 G、R、B(不是常见的 R、G、B), 每颗灯珠共 24 bit。
 */
static void ws2815_send_frame(rmt_channel_handle_t channel,
                               rmt_encoder_handle_t encoder,
                               const uint8_t *pixels)
{
    rmt_transmit_config_t tx_config = { .loop_count = 0 };

    ESP_ERROR_CHECK(rmt_transmit(
        channel,
        encoder,
        pixels,
        WS2815_TEST_LED_COUNT * 3,
        &tx_config
    ));

    ESP_ERROR_CHECK(rmt_tx_wait_all_done(channel, portMAX_DELAY));
}

/**
 * @brief  WS2815B 测试任务: 仅第 1 颗灯持续显示暗红色
 * @param  arg 未使用
 */
static void ws2815_test_task(void *arg)
{
    rmt_channel_handle_t channel = NULL;
    rmt_encoder_handle_t encoder = NULL;

    /*
     * RMT 发送通道配置:
     *   clk_src           : 时钟源, 用默认值即可
     *   gpio_num          : 信号输出引脚(GPIO3 -> WS2815B 的 DI)
     *   mem_block_symbols : 通道硬件内存大小, 单位是 symbol(一个 symbol = 一段电平)。
     *                       1 颗灯珠 24bit = 24 个 symbol, 加复位码 2 个, 64 足够;
     *                       灯珠数量多时要相应加大, 否则需要多次分批发送。
     *   resolution_hz     : 时钟分辨率 10MHz, 精度 0.1us, 足够表达 0.3/0.9us 的码型
     *   trans_queue_depth : 发送队列深度, 测试场景用 1 即可
     */
    rmt_tx_channel_config_t channel_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .gpio_num = WS2815_TEST_GPIO,
        .mem_block_symbols = 64,
        .resolution_hz = WS2815_RMT_RESOLUTION_HZ,
        .trans_queue_depth = 1,
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&channel_config, &channel));

    /* 编码器配置: 时钟分辨率要和通道一致; reset_us = 300us 满足 WS2815B 的 >280us 要求 */
    ws2815_rmt_encoder_config_t encoder_config = {
        .resolution_hz = WS2815_RMT_RESOLUTION_HZ,
        .reset_us = 300,
    };
    ESP_ERROR_CHECK(ws2815_new_rmt_encoder(&encoder_config, &encoder));
    /* 创建完通道和编码器后必须使能通道, 硬件才会真正开始工作 */
    ESP_ERROR_CHECK(rmt_enable(channel));

    ESP_LOGW(TAG, "3.3V direct test on GPIO%d: only LED0 red", WS2815_TEST_GPIO);

    uint8_t pixels[WS2815_TEST_LED_COUNT * 3];
    memset(pixels, 0, sizeof(pixels));
    /* WS2815B byte order is G, R, B. LEDs 1..15 remain explicitly off. */
    pixels[0] = 0;
    pixels[1] = 32;
    pixels[2] = 0;

    while (true) {
        ws2815_send_frame(channel, encoder, pixels);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void ws2815_test_start(void)
{
    /* 创建测试任务: 栈 4096 字节, 优先级 4。xTaskCreate 失败(内存不足)时用 ESP_ERROR_CHECK 断言 */
    BaseType_t created = xTaskCreate(ws2815_test_task, "ws2815_test", 4096,
                                     NULL, 4, NULL);
    ESP_ERROR_CHECK(created == pdPASS ? ESP_OK : ESP_ERR_NO_MEM);
}
