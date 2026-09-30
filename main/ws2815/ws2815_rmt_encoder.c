#include <stdlib.h>

#include "esp_check.h"
#include "ws2815_rmt_encoder.h"

static const char *TAG = "ws2815_encoder";

typedef struct {
    rmt_encoder_t base;
    rmt_encoder_handle_t bytes_encoder;
    rmt_encoder_handle_t copy_encoder;
    int state;
    rmt_symbol_word_t reset_code;
} ws2815_rmt_encoder_t;

RMT_ENCODER_FUNC_ATTR
static size_t ws2815_encode(rmt_encoder_t *encoder, rmt_channel_handle_t channel,
                            const void *data, size_t data_size,
                            rmt_encode_state_t *ret_state)
{
    ws2815_rmt_encoder_t *ws2815 =
        __containerof(encoder, ws2815_rmt_encoder_t, base);
    rmt_encode_state_t session_state = RMT_ENCODING_RESET;
    rmt_encode_state_t state = RMT_ENCODING_RESET;
    size_t encoded_symbols = 0;

    switch (ws2815->state) {
    case 0:
        encoded_symbols += ws2815->bytes_encoder->encode(ws2815->bytes_encoder, channel,
                                                          data, data_size, &session_state);
        if (session_state & RMT_ENCODING_COMPLETE) {
            ws2815->state = 1;
        }
        if (session_state & RMT_ENCODING_MEM_FULL) {
            state |= RMT_ENCODING_MEM_FULL;
            goto out;
        }
        /* Pixel data fit in this block; append the latch-low symbol as well. */
        /* fall through */
    case 1:
        encoded_symbols += ws2815->copy_encoder->encode(ws2815->copy_encoder, channel,
                                                         &ws2815->reset_code,
                                                         sizeof(ws2815->reset_code),
                                                         &session_state);
        if (session_state & RMT_ENCODING_COMPLETE) {
            ws2815->state = RMT_ENCODING_RESET;
            state |= RMT_ENCODING_COMPLETE;
        }
        if (session_state & RMT_ENCODING_MEM_FULL) {
            state |= RMT_ENCODING_MEM_FULL;
        }
        break;
    }

out:
    *ret_state = state;
    return encoded_symbols;
}

static esp_err_t ws2815_encoder_del(rmt_encoder_t *encoder)
{
    ws2815_rmt_encoder_t *ws2815 =
        __containerof(encoder, ws2815_rmt_encoder_t, base);
    rmt_del_encoder(ws2815->bytes_encoder);
    rmt_del_encoder(ws2815->copy_encoder);
    free(ws2815);
    return ESP_OK;
}

RMT_ENCODER_FUNC_ATTR
static esp_err_t ws2815_encoder_reset(rmt_encoder_t *encoder)
{
    ws2815_rmt_encoder_t *ws2815 =
        __containerof(encoder, ws2815_rmt_encoder_t, base);
    rmt_encoder_reset(ws2815->bytes_encoder);
    rmt_encoder_reset(ws2815->copy_encoder);
    ws2815->state = RMT_ENCODING_RESET;
    return ESP_OK;
}

esp_err_t ws2815_new_rmt_encoder(const ws2815_rmt_encoder_config_t *config,
                                  rmt_encoder_handle_t *ret_encoder)
{
    ESP_RETURN_ON_FALSE(config && ret_encoder, ESP_ERR_INVALID_ARG, TAG,
                        "invalid encoder configuration");

    ws2815_rmt_encoder_t *ws2815 = rmt_alloc_encoder_mem(sizeof(*ws2815));
    ESP_RETURN_ON_FALSE(ws2815, ESP_ERR_NO_MEM, TAG, "no memory for encoder");

    ws2815->base.encode = ws2815_encode;
    ws2815->base.del = ws2815_encoder_del;
    ws2815->base.reset = ws2815_encoder_reset;

    /* WS2815B uses the WS281x 800 kHz, MSB-first waveform. */
    rmt_bytes_encoder_config_t bytes_config = {
        .bit0 = {
            .level0 = 1,
            .duration0 = config->resolution_hz * 3 / 10000000, /* 0.3 us */
            .level1 = 0,
            .duration1 = config->resolution_hz * 9 / 10000000, /* 0.9 us */
        },
        .bit1 = {
            .level0 = 1,
            .duration0 = config->resolution_hz * 9 / 10000000, /* 0.9 us */
            .level1 = 0,
            .duration1 = config->resolution_hz * 3 / 10000000, /* 0.3 us */
        },
        .flags.msb_first = 1,
    };

    esp_err_t ret = rmt_new_bytes_encoder(&bytes_config, &ws2815->bytes_encoder);
    if (ret != ESP_OK) {
        free(ws2815);
        return ret;
    }

    rmt_copy_encoder_config_t copy_config = {};
    ret = rmt_new_copy_encoder(&copy_config, &ws2815->copy_encoder);
    if (ret != ESP_OK) {
        rmt_del_encoder(ws2815->bytes_encoder);
        free(ws2815);
        return ret;
    }

    uint32_t half_reset_ticks = config->resolution_hz / 1000000 * config->reset_us / 2;
    ws2815->reset_code = (rmt_symbol_word_t) {
        .level0 = 0,
        .duration0 = half_reset_ticks,
        .level1 = 0,
        .duration1 = half_reset_ticks,
    };
    *ret_encoder = &ws2815->base;
    return ESP_OK;
}
