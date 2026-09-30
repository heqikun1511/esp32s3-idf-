/* Minimal RMT encoder for WS281x-compatible LEDs, including WS2815B. */
#pragma once

#include <stdint.h>
#include "driver/rmt_encoder.h"

typedef struct {
    uint32_t resolution_hz;
    uint32_t reset_us;
} ws2815_rmt_encoder_config_t;

esp_err_t ws2815_new_rmt_encoder(const ws2815_rmt_encoder_config_t *config,
                                  rmt_encoder_handle_t *ret_encoder);
