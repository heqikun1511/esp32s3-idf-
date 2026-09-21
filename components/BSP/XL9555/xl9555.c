/**
 * @file        xl9555.c
 * @brief       XL9555 I2C IO扩展器驱动实现
 */

#include "xl9555.h"
#include "driver/i2c_master.h"
#include "esp_log.h"

static const char *TAG = "XL9555";

static i2c_master_dev_handle_t xl9555_dev = NULL;

static uint8_t xl9555_port1_key_read(uint8_t mask);

/**
 * @brief       写XL9555寄存器
 */
static esp_err_t xl9555_write_reg(uint8_t reg, uint8_t data)
{
    uint8_t write_buf[2] = { reg, data };
    return i2c_master_transmit(xl9555_dev, write_buf, sizeof(write_buf), pdMS_TO_TICKS(100));
}

/**
 * @brief       读XL9555寄存器
 */
static esp_err_t xl9555_read_reg(uint8_t reg, uint8_t *data)
{
    return i2c_master_transmit_receive(xl9555_dev, &reg, 1, data, 1, pdMS_TO_TICKS(100));
}

/**
 * @brief       初始化XL9555
 */
esp_err_t xl9555_init(i2c_master_bus_handle_t bus_handle)
{
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = XL9555_ADDR,
        .scl_speed_hz = 400000,
    };

    esp_err_t ret = i2c_master_bus_add_device(bus_handle, &dev_cfg, &xl9555_dev);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add XL9555 device");
        return ret;
    }

    /* 官方手册配置字为0x1F00：EXIO8~12均为输入。 */
    xl9555_write_reg(XL9555_CFG_PORT0, 0x00);
    xl9555_write_reg(XL9555_CFG_PORT1, 0x1F);

    ESP_LOGI(TAG, "XL9555 initialized (KEY0=EXIO8, KEY1=EXIO9, KEY2=EXIO10)");
    return ESP_OK;
}

/**
 * @brief       读取KEY0状态 (EXIO8 = IO1_0)
 */
uint8_t xl9555_key0_read(void)
{
    uint8_t val = 0;
    if (xl9555_read_reg(XL9555_INPUT_PORT1, &val) == ESP_OK) {
        return (val & XL9555_KEY0_BIT) ? 1 : 0;
    }
    return 1; /* 读取失败返回松开 */
}

/**
 * @brief       读取KEY1状态 (EXIO9 = IO1_1)
 */
uint8_t xl9555_key1_read(void)
{
    return xl9555_port1_key_read(XL9555_KEY1_BIT);
}

static uint8_t xl9555_port1_key_read(uint8_t mask)
{
    uint8_t val = 0;
    if (xl9555_read_reg(XL9555_INPUT_PORT1, &val) == ESP_OK) {
        return (val & mask) ? 1 : 0;
    }
    return 1;
}

uint8_t xl9555_extio9_read(void)
{
    return xl9555_port1_key_read(XL9555_EXTIO9_BIT);
}

uint8_t xl9555_extio10_read(void)
{
    return xl9555_port1_key_read(XL9555_EXTIO10_BIT);
}

uint8_t xl9555_key2_read(void)
{
    return xl9555_port1_key_read(XL9555_KEY2_BIT);
}
