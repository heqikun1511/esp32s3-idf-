/* 引脚定义 */
#define BOOT_INT_GPIO_PIN   GPIO_NUM_0
/* IO 操作 */
#define BOOT_INT            gpio_get_level(BOOT_INT_GPIO_PIN)
/* 函数声明 */
void exit_init(void);
/* 外部中断初始化程序 */