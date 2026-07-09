#ifndef __ESPTIM_H__
#define __ESPTIM_H__

void esptim_int_init(uint64_t tps);
void esptim_callback(void *arg);
/* 初始化初始化高分辨率定时器 */
/* 定时器回调函数 */

#endif /* __ESPTIM_H__ */   