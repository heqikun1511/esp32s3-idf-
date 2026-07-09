#include "BSP/esptim/esptim.h"
#include "driver/timer.h"
#include "BSP/LED/led.h"


void esptim_int_init(uint64_t tps)
{
esp_timer_handle_t esp_tim_handle;
/* 定时器回调函数句柄 */
/* 定义一个定时器结构体 */
esp_timer_create_args_t tim_periodic_arg = {
.callback = &esptim_callback,
.arg = NULL,
};
/* 设置回调函数 */
/* 不携带参数 */
esp_timer_create(&tim_periodic_arg, &esp_tim_handle); /* 创建一个事件 */
esp_timer_start_periodic(esp_tim_handle, tps);
/* 每周期内触发一次 */
}

void esptim_callback(void *arg)
{
LED_TOGGLE();
}