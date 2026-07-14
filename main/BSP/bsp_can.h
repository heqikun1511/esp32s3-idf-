#ifndef __BSP_CAN_H
#define __BSP_CAN_H

#include <stdint.h>

/* CAN扩展帧ID定义 */
#define VCU_TO_MCU_CMD_ID     0x08C1EF21  /* VCU→MCU 电机控制命令 */
#define MCU_TO_VCU_STATUS1_ID 0x0CFFC6EF  /* MCU→VCU 电机状态信息1 */

/* CAN接收回调函数类型 */
typedef void (*can_data_callback_t)(uint32_t can_id, uint8_t *data, uint8_t len);

/**
 * @brief       初始化CAN(TWAI)接口
 * @param       tx_pin : CAN发送引脚(GPIO号)
 * @param       rx_pin : CAN接收引脚(GPIO号)
 * @param       callback : 接收到报文时的回调函数(可为NULL)
 * @retval      0:成功 -1:失败
 */
int bsp_can_init(int tx_pin, int rx_pin, can_data_callback_t callback);

/**
 * @brief       发送CAN扩展帧
 * @param       can_id : 29位扩展帧ID
 * @param       data   : 数据缓冲区
 * @param       len    : 数据长度(≤8)
 * @retval      0:成功 -1:失败
 */
int bsp_can_send(uint32_t can_id, uint8_t *data, uint8_t len);

/**
 * @brief       解析转速值(遵循协议: 0.5rpm/bit, offset -10000rpm)
 * @param       raw_low  : 低字节
 * @param       raw_high : 高字节
 * @retval      实际转速值(rpm)
 */
static inline int bsp_can_parse_speed(uint8_t raw_low, uint8_t raw_high)
{
    int16_t raw = (int16_t)(raw_low | ((uint16_t)raw_high << 8));
    /* actual_rpm = raw_value * 0.5 */
    return (int)(raw * 0.5f);
}

#endif