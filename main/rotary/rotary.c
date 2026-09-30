#include "rotary_input.h"

#include "driver/gpio.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

#define TAG "ROTARY"

#define ROTARY_A_GPIO  GPIO_NUM_3
#define ROTARY_B_GPIO  GPIO_NUM_5
#define ROTARY_SW_GPIO GPIO_NUM_4

#define ROTARY_QUEUE_LENGTH       16
#define ROTARY_TASK_PERIOD_MS     2
#define ROTARY_PRESS_FILTER_COUNT 10

static QueueHandle_t s_event_queue;
static TaskHandle_t s_task_handle;

static void rotary_task(void *arg)
{
    uint8_t previous_state =
        ((gpio_get_level(ROTARY_A_GPIO) ? 1 : 0) << 1) |
        (gpio_get_level(ROTARY_B_GPIO) ? 1 : 0);

    int8_t step_accumulator = 0;

    uint8_t last_switch = 1;
    uint8_t stable_switch = 1;
    uint8_t switch_count = 0;

    /*
     * 四相编码状态表。
     * 如果左右方向反了，把 LEFT 和 RIGHT 对调即可。
     */
    static const int8_t transition_table[16] = {
         0, -1,  1,  0,
         1,  0,  0, -1,
        -1,  0,  0,  1,
         0,  1, -1,  0
    };

    while (1)
    {
        uint8_t a = gpio_get_level(ROTARY_A_GPIO) ? 1 : 0;
        uint8_t b = gpio_get_level(ROTARY_B_GPIO) ? 1 : 0;

        uint8_t current_state = (a << 1) | b;
        uint8_t table_index = (previous_state << 2) | current_state;

        step_accumulator += transition_table[table_index];
        previous_state = current_state;

        /*
         * 一个完整定位档通常会产生4次状态变化。
         */
        if (step_accumulator >= 4)
        {
            rotary_event_t event = ROTARY_EVENT_RIGHT;
            xQueueSend(s_event_queue, &event, 0);
            step_accumulator = 0;
        }
        else if (step_accumulator <= -4)
        {
            rotary_event_t event = ROTARY_EVENT_LEFT;
            xQueueSend(s_event_queue, &event, 0);
            step_accumulator = 0;
        }

        /*
         * C脚低电平表示按下。
         */
        uint8_t raw_switch =
            gpio_get_level(ROTARY_SW_GPIO) ? 1 : 0;

        if (raw_switch == last_switch)
        {
            if (switch_count < ROTARY_PRESS_FILTER_COUNT)
            {
                switch_count++;
            }
        }
        else
        {
            switch_count = 0;
            last_switch = raw_switch;
        }

        if (switch_count >= ROTARY_PRESS_FILTER_COUNT &&
            stable_switch != raw_switch)
        {
            stable_switch = raw_switch;

            if (stable_switch == 0)
            {
                rotary_event_t event = ROTARY_EVENT_PRESS;
                xQueueSend(s_event_queue, &event, 0);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(ROTARY_TASK_PERIOD_MS));
    }
}

esp_err_t rotary_input_init(void)
{
    const gpio_config_t config = {
        .pin_bit_mask =
            (1ULL << ROTARY_A_GPIO) |
            (1ULL << ROTARY_B_GPIO) |
            (1ULL << ROTARY_SW_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    esp_err_t err = gpio_config(&config);
    if (err != ESP_OK)
    {
        return err;
    }

    s_event_queue = xQueueCreate(
        ROTARY_QUEUE_LENGTH,
        sizeof(rotary_event_t));

    if (s_event_queue == NULL)
    {
        return ESP_ERR_NO_MEM;
    }

    BaseType_t result = xTaskCreate(
        rotary_task,
        "rotary_input",
        3072,
        NULL,
        10,
        &s_task_handle);

    if (result != pdPASS)
    {
        vQueueDelete(s_event_queue);
        s_event_queue = NULL;
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG,
             "Rotary initialized: A=GPIO%d B=GPIO%d SW=GPIO%d",
             ROTARY_A_GPIO,
             ROTARY_B_GPIO,
             ROTARY_SW_GPIO);

    return ESP_OK;
}

bool rotary_input_get_event(rotary_event_t *event,
                            TickType_t wait_ticks)
{
    if (event == NULL || s_event_queue == NULL)
    {
        return false;
    }

    return xQueueReceive(
               s_event_queue,
               event,
               wait_ticks) == pdTRUE;
}