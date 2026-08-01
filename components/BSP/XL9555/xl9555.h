/**
 * @file        xl9555.h
 * @brief       XL9555 I2C IO扩展器驱动 (PCA9555兼容)
 * @author      正点原子团队(ALIENTEK)
 */

#ifndef __XL9555_H
#define __XL9555_H

#include <stdint.h>
#include "esp_err.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef __cplusplus
extern "C" {
#endif

/* XL9555 设备地址 (A2=A1=A0=GND) */
#define XL9555_ADDR         0x20

/* XL9555 寄存器地址 */
#define XL9555_INPUT_PORT0  0x00    /* 输入端口0 (只读) */
#define XL9555_INPUT_PORT1  0x01    /* 输入端口1 (只读) */
#define XL9555_OUTPUT_PORT0 0x02    /* 输出端口0 */
#define XL9555_OUTPUT_PORT1 0x03    /* 输出端口1 */
#define XL9555_CFG_PORT0    0x06    /* 配置端口0 (0=输出, 1=输入) */
#define XL9555_CFG_PORT1    0x07    /* 配置端口1 (0=输出, 1=输入) */

/*
 * EXIO引脚功能定义 (对照原理图XL9555)
 *
 * EXIO0 (IO0_0) - BEEP
 * EXIO1 (IO0_1) - SPK_EN
 * EXIO2 (IO0_2) - GEC_KEY
 * EXIO3 (IO0_3) - RS485_RE
 * EXIO4 (IO0_4) - SLCD_PWR
 * EXIO5 (IO0_5) - SLCD_RST
 * EXIO6 (IO0_6) - KEY2
 * EXIO7 (IO0_7) - KEY1   ←
 * EXIO8 (IO1_0) - KEY0   ←
 * EXIO9 (IO1_1) - AP_INT
 * EXIO10(IO1_2) - QMI_INT
 * EXIO11(IO1_3) - LED1
 * EXIO12-15     - 其他
 */

/* KEY连接在EXIO上的位定义 */
#define XL9555_KEY0_BIT     (1 << 0)    /* EXIO8 = IO1_0 */
#define XL9555_KEY1_BIT     (1 << 7)    /* EXIO7 = IO0_7 */

/**
 * @brief       初始化XL9555
 * @param       bus_handle : I2C总线句柄
 * @retval      ESP_OK:成功
 */
esp_err_t xl9555_init(i2c_master_bus_handle_t bus_handle);

/**
 * @brief       读取KEY0状态 (EXIO8)
 * @retval      0=按下, 1=松开 (低电平有效)
 */
uint8_t xl9555_key0_read(void);

/**
 * @brief       读取KEY1状态 (EXIO7)
 * @retval      0=按下, 1=松开 (低电平有效)
 */
uint8_t xl9555_key1_read(void);

#ifdef __cplusplus
}
#endif

#endif /* __XL9555_H */
