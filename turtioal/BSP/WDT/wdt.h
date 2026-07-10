#ifndef __WDT_H__
#define __WDT_H__

#include <stdint.h>

/* 函数声明 */
void wdt_init(uint64_t timeout_us);  /* 初始化看门狗，设置超时时间(微秒) */
void wdt_feed(uint64_t timeout_us);  /* 喂狗，重新开始计时 */
void wdt_isr_handler(void *arg);     /* 看门狗回调函数 */

#endif /* __WDT_H__ */