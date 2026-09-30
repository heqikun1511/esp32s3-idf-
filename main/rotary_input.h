#pragma once

#include "esp_err.h"
#include "freertos/FreeRTOS.h"

typedef enum {
    ROTARY_EVENT_NONE = 0,
    ROTARY_EVENT_LEFT,
    ROTARY_EVENT_RIGHT,
    ROTARY_EVENT_PRESS,
} rotary_event_t;

esp_err_t rotary_input_init(void);
bool rotary_input_get_event(rotary_event_t *event, TickType_t wait_ticks);
