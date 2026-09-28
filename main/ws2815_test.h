#pragma once

/**
 * @brief 在 GPIO3(接 WS2815B 的 DI 数据脚)上启动临时的 3.3V 直驱测试。
 *        内部会创建一个后台任务, 循环显示 红 / 绿 / 蓝 / 灭, 每 500ms 切换一次。
 * @param 无
 * @retval 无
 */
void ws2815_test_start(void);
