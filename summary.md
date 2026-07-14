# 编译错误修复总结

## 修复前错误列表

构建时共发现 **6 个编译错误**，涉及 4 个源文件 + 1 个配置文件：

| # | 文件 | 错误信息 | 原因 |
|---|------|---------|------|
| 1 | `main/ui/styles.c:71` | `expected ';' before 'lv_font_montserrat_48'` | 函数参数重复书写错误 |
| 2 | `main/ui/styles.c:161` | `expected ';' before 'lv_font_montserrat_48'` | 同上，重复出现 |
| 3 | `main/main.c:23` | `nvs_flash.h: No such file or directory` | `nvs_flash` 组件未加入 REQUIRES |
| 4 | `main/main.c:41` | `'GPIO_NUM_27' undeclared` | 缺少 `gpio_types.h` 头文件 |
| 5 | `main/APP/lvgl_demo.c:22` | `lcd.h: No such file or directory` | `BSP` 组件未加入 REQUIRES |
| 6 | `main/APP/lvgl_demo.c:24` | `esp_timer.h: No such file or directory` | `esp_timer` 组件未加入 REQUIRES |
| 7 | `main/BSP/bsp_can.c` | `#warning "The legacy TWAI driver is deprecated..."` | 旧版 TWAI driver 的 `#warning` 被 `-Werror` 转为错误 |

---

## 修复详情

### 1. `main/ui/styles.c` — 参数重复

**行 71 和 161** 原代码：
```c
lv_style_set_text_font(style, &lv_font_montserrat_48)lv_font_montserrat_48);
```
参数 `&lv_font_montserrat_48)` 被重复写了两次，并且缺少分号。

**修复后**：
```c
lv_style_set_text_font(style, &lv_font_montserrat_48);
```

---

### 2. `main/main.c` — 缺少头文件和组件依赖

**问题 a：`nvs_flash.h` 找不到**

`nvs_flash` 是 ESP-IDF 的一个独立组件，需要在 `main/CMakeLists.txt` 的 `REQUIRES` 中声明。

**问题 b：`GPIO_NUM_27` / `GPIO_NUM_26` 未声明**

需要包含 `hal/gpio_types.h` 来获取 GPIO 引脚号定义。

**修复 — `main/main.c`**：
```c
// 新增头文件
#include "hal/gpio_types.h"
```

---

### 3. `main/CMakeLists.txt` — 组件依赖缺失

**修复前**：
```cmake
REQUIRES
    driver
```

**修复后**：
```cmake
REQUIRES
    driver
    nvs_flash
    esp_timer
    BSP
```

新增了三个 REQUIRES：
- `nvs_flash` — 使能 `nvs_flash.h` 的查找
- `esp_timer` — 使能 `esp_timer.h` 的查找
- `BSP` — 使能 `lcd.h`、`touch.h` 等 BSP 组件头文件的查找

---

### 4. `sdkconfig` — TWAI 弃用警告

**问题**：`bsp_can.c` 使用了旧版 `driver/twai.h`，该头文件包含：
```c
#if !CONFIG_TWAI_SUPPRESS_DEPRECATE_WARN
#warning "The legacy TWAI driver is deprecated, please use esp_twai.h"
#endif
```
由于编译选项启用了 `-Werror`，`#warning` 被当作错误处理。

**修复 — `sdkconfig`**：
```
# 修改前
# CONFIG_TWAI_SUPPRESS_DEPRECATE_WARN is not set

# 修改后
CONFIG_TWAI_SUPPRESS_DEPRECATE_WARN=y
```

---

## 构建结果

修复后构建成功通过，生成了固件镜像：
- 输出：`build/lvgl_transplant.bin`
- 大小：`0xc69b0` bytes（分区剩余 22% 空间）

### 剩余警告（不影响编译）
- `lvgl_demo.h:32` — `increase_lvgl_tick` 声明为 `static` 但未定义（函数定义在 `.c` 文件中，非静态）
- `lvgl_demo.h:33` — `lvgl_disp_flush_cb` 同上

这些警告可后续通过调整头文件中的 `static` 关键字来解决。

---

# 运行时问题修复总结

## 问题：LVGL 屏幕启动后 CAN 通信失败

### 根因

`main/APP/lvgl_demo.c` 中 `lvgl_demo()` 函数末尾有一个 **`while(1)` 死循环**：

```c
void lvgl_demo(void)
{
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();
    /* ... */
    ui_init();

    while (1)          // ← 死循环！永不返回
    {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
```

在 `main.c` 的 `app_main()` 中调用顺序为：

```
app_main()
  ├── lvgl_demo()    ← 进入 while(1)，永不返回
  ├── bsp_can_init() ← ❌ 从未执行
  ├── ui_update_task ← ❌ 从未创建
  └── ...
```

**导致 CAN 不是"启动后失败"，而是根本从未初始化过。**

### 修复方案

#### 1. `main/APP/lvgl_demo.c`

将 `lv_timer_handler()` 循环从内联死循环改为独立的 FreeRTOS 任务，使 `lvgl_demo()` 正常返回：

| 修改前 | 修改后 |
|--------|--------|
| `while(1) { lv_timer_handler(); delay(10); }` 阻塞在 `lvgl_demo()` 内部 | 创建 `lvgl_timer` 任务（优先级 2）运行循环，`lvgl_demo()` 返回 |

新增头文件：
```c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
```

#### 2. `main/main.c`

移除了 `ui_update_task` 中每 100ms 的 `ESP_LOGI` 冗余日志，减少日志输出对 CAN 时序的干扰。

### 任务优先级（修复后）

| 任务 | 优先级 | 说明 |
|------|--------|------|
| `can_rx` | **5** | CAN 接收，最高优先级，保证不丢帧 |
| `ui_update` | **3** | UI 数据更新（转速、车速等） |
| `lvgl_timer` | **2** | LVGL 内部定时器处理（`lv_timer_handler`） |

### 构建结果

修复后编译通过，`app_main()` 执行流程恢复正常：
```
app_main()
  ├── lvgl_demo()    ← 初始化 LVGL，创建 lvgl_timer 任务后返回 ✅
  ├── bsp_can_init() ← 初始化 TWAI 驱动，创建 can_rx 任务 ✅
  ├── ui_update_task ← 创建 UI 更新任务 ✅
  └── System started ✅
```

---

# 运行时问题修复总结（二）

## 问题 1：屏幕显示后 `lvgl_timer` 栈溢出崩溃

### 现象

```
Guru Meditation Error: Core 0 panic'ed (Stack protection fault)
Detected in task "lvgl_timer" at lv_draw_sw_blend_color_to_rgb565.c:256
Stack pointer: 0x4ff3bb70
Stack bounds: 0x4ff3bb78 - 0x4ff3cb70
```

**SP (0x4ff3bb70) < Stack bounds (0x4ff3bb78)** → 栈指针溢出。

### 根因

`lvgl_timer` 任务栈仅 **4096 字节**，而 LVGL 的软件渲染（字体位图解压、颜色混合 `lv_draw_sw_blend_to_rgb565`）需要更深的调用栈。在 PARTIAL / FULL 模式下执行首次全屏渲染时，函数调用链路过深导致栈溢出。

### 修复 — `main/APP/lvgl_demo.c`

```
xTaskCreatePinnedToCore(
    lvgl_timer_task,
    "lvgl_timer",
    8192,       // ← 4096 → 8192 字节
    ...
);
```

---

## 问题 2：PARTIAL 渲染模式 + 旋转导致花屏

### 现象

屏幕显示花屏/乱码。

### 根因

尝试改为 `LV_DISPLAY_RENDER_MODE_PARTIAL` 以降低刷新率，但 PARTIAL 模式与**手动旋转缓冲区**不兼容：

| 问题 | 说明 |
|------|------|
| PARTIAL 分块渲染 | LVGL 将屏幕分成 50 行一块，逐块调用 flush_cb |
| 旋转后数据散布 | 每块旋转后的像素分散在 `g_rot_buf` 的物理坐标位置 |
| MIPI DSI 要求连续 | `esp_lcd_panel_draw_bitmap` 期望缓冲区数据是连续的矩形区域 |
| **结果** | 旋转后的散落数据不符合 DSI 要求 → **花屏** |

### 修复方案

回退到 `LV_DISPLAY_RENDER_MODE_FULL` + 硬件双帧缓冲：

```c
// 恢复 FULL 模式 + 硬件帧缓冲
lv_display_set_buffers(disp, lcd_buffer[0], lcd_buffer[1], buf_size,
                       LV_DISPLAY_RENDER_MODE_FULL);
```

| 模式 | 效果 |
|------|------|
| `FULL` + 双缓冲 | LVGL 渲染到离屏缓冲 → 整帧旋转 → 整帧 MIPI DSI 刷新 → **正确显示** |
| `ui_update_task` 优化 | 仅在数值变化时更新 LVGL 对象，不触发无效刷新 |
| 无变化时 | `lv_timer_handler()` 什么都不做，屏幕 **完全静止常亮** |

### 最终任务状态

| 任务 | 栈大小 | 优先级 | 说明 |
|------|--------|--------|------|
| `lvgl_timer` | **8192** ✅ | 2 | LVGL 定时器处理，栈空间充足 |
| `can_rx` | 4096 | 5 | CAN 接收，最高优先级 |
| `ui_update` | 4096 | 3 | UI 数据更新 |

### 经验教训

1. **PARTIAL 模式 + 手动旋转不兼容** — 旋转后像素在缓冲区的物理坐标是散列的，而 MIPI DSI 的 `draw_bitmap` 期望连续的行主序数据。如需局部刷新，需额外分配一块连续的目标区域缓冲区。
2. **LVGL 任务栈至少 8192** — 软件渲染的调用链（draw → blend → font bitmap → malloc）深度较大，4096 不够。
3. **FULL 模式在无变化时不会刷新** — LVGL 不会无缘无故重绘，屏幕常亮没有问题。
