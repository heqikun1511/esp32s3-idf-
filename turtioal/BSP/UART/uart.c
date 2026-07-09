#include "BSP/UART/uart.h"
#include "driver/uart.h"
#include "driver/gpio.h"

void uart_init(uint32_t baudrate)
{
    uart_config_t uart_config;
    /* 串口配置句柄 */
    /* 波特率 */
    uart_config.baud_rate = baudrate;
    uart_config.data_bits = UART_DATA_8_BITS;
    /* 字长为 8 位数据格式 */
    uart_config.parity = UART_PARITY_DISABLE;
    /* 无奇偶校验位 */
    uart_config.stop_bits = UART_STOP_BITS_1;
    /* 一个停止位 */
    uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    /* 无硬件控制流 */
                               uart_config.source_clk = UART_SCLK_DEFAULT;
    /* 配置时钟源，UART_SCLK_DEFAULT 让驱动自动选择合适时钟 */
    uart_config.rx_flow_ctrl_thresh = 122;
    /* 硬件控制流阈值 */
    uart_param_config(USART_UX, &uart_config);
    /* 配置 uart 端口 */
    /* 配置 uart 引脚 */
    uart_set_pin(USART_UX,
                 USART_TX_GPIO_PIN,
                 USART_RX_GPIO_PIN,
                 UART_PIN_NO_CHANGE,
                 UART_PIN_NO_CHANGE);
    /* 安装串口驱动 */
    uart_driver_install(USART_UX,
                        RX_BUF_SIZE * 2,
                        RX_BUF_SIZE * 2,
                        20,
                        NULL,
                        0);
}