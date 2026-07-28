# 工程迁移与适配总结

## 本次会话完成的工作（2026-07-27）

### 1. LVGL 8.4 → 9.5 升级（后降级回 8.4）

| 步骤 | 说明 |
|------|------|
| 升级到 9.5 | `main/idf_component.yml`: `lvgl/lvgl: "^9.5"` |
| 降级回 8.4 | 重新改为 `"^8.3.11"`，重新下载 managed component |
| 显示驱动适配 | `lv_port_disp_init` 在 v8/v9 间来回切换 |

### 2. SquareLine Studio → EEZ Studio 迁移

| 文件 | 修改 |
|------|------|
| `main/CMakeLists.txt` | 移除 `ui/SquareLine_Project/*` 路径，添加 `ui`、`ui/fonts` |
| `main/main.c` | 屏幕列表从 `ui_Screen1/ui_Screen2` 改为 `objects.driver_view/autonomous` |
| `main/main.c` | 移除 `ui_Label1/ui_Label3` 引用，改用 `objects.speed_label` |
| `main/ui/` | 完整替换为 EEZ Studio 导出的 UI 文件 |
| `main/ui/screens.h` | 补全 EEZ Studio 导出遗漏的 `objects_t` 成员（`hv_bar_1`、`brake_presure_bar_1` 等 30+ 个） |

### 3. 依赖清理

| 依赖 | 处理 |
|------|------|
| ROS2 (`ros2subscriber.h`) | 删除，`screens.c`/`ui.c` 中移除 include |
| DBC (`dbc_api.h`) | 删除，CAN 数据结构定义移到 `screens.c` 本地 |
| EEZ Flow (`getFlowState`/`evalTextProperty`) | 用桩函数替代，后随新导出完全移除 |
| EEZ Flow `get_var_*()` | 改为全局变量 + 字符串格式化函数 |

### 4. 分辨率修复

| 问题 | 修复 |
|------|------|
| EEZ 导出所有屏幕为 800×480 | `sed -i 's/800, 480/1920, 1080/g'` 批量替换 |
| EEZ Studio bug：改 Display 设置后导出模板不更新 | 已在代码层修复 |

### 5. 看门狗与崩溃修复

| 问题 | 原因 | 修复 |
|------|------|------|
| 白屏 | `lv_display_create()` 不设默认显示器（LVGL 9.5） | 加 `lv_display_set_default()` |
| 白屏 | `lv_scr_load_anim` 需 `lv_timer_handler` 循环 | 改为 `lv_screen_load()` / `lv_scr_load()` |
| 崩溃 `lv_draw_add_task` | LVGL 8.x 字体格式在 9.5 不兼容 | 降级回 LVGL 8.4 |
| 看门狗复位 | `create_screens()` 耗时过长 | 加 `esp_task_wdt_add/reset/delete` |
| 控件 NULL 崩溃 | 字体格式不兼容导致渲染崩溃 | 降级 + 字体兼容 |

### 6. 关键文件改动清单

| 文件 | 改动 |
|------|------|
| `main/idf_component.yml` | LVGL 版本 8.3.11 ↔ 9.5 ↔ 8.3.11 |
| `main/CMakeLists.txt` | SquareLine → EEZ 路径切换 |
| `main/main.c` | 屏幕列表、标签引用、屏幕数量更新 |
| `main/APP/lvgl_demo.c` | 显示驱动 v8→v9→v8，喂狗 |
| `main/APP/lvgl_demo.h` | 函数签名同步更新 |
| `main/ui/ui.c` | EEZ init/loadScreen，ROS2 移除 |
| `main/ui/ui.h` | 保持 EEZ 导出 |
| `main/ui/screens.c` | ROS2/DBC 移除，CAN 数据结构，get_var 桩函数 |
| `main/ui/screens.h` | 补全缺失对象成员 |
| `main/ui/eez_flow_stubs.h` | 创建后随新导出删除 |

### 7. 当前状态

| 项目 | 状态 |
|------|------|
| LVGL 版本 | **8.4.0** |
| 屏幕分辨率 | **1920×1080** |
| ROS2 依赖 | **已移除** |
| EEZ Flow 依赖 | **已移除** |
| CAN 数据桥 | **就绪**（全局变量 `g_var_*` + `ui_set_*()` API） |
| 编译 | **通过** |
| KEY1 切换 | Driver View ↔ Autonomous |
| 显示 | 测试屏幕正常，EEZ 屏幕有字体兼容问题 |

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

## Git 操作总结

### 1. 忽略 `managed_components/` 目录

`managed_components/` 是 ESP-IDF 组件管理器自动下载的第三方组件目录（如 `lvgl__lvgl`），不应纳入版本控制。

**修改 `.gitignore`**：
```
# 修改前
managed_components/lvgl__lvgl/*

# 修改后
managed_components/
```

**取消已跟踪文件的缓存**：
```bash
git rm -r --cached managed_components/
```

**提交**：
```bash
git add .gitignore
git commit -m "chore: ignore managed_components/ directory"
```

**验证**：
```
.gitignore:12:managed_components/       managed_components/lvgl__lvgl/README.md
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
|------|--summary.md----|
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

---

# 编译错误修复总结（三）

## 修复前summary.md----|---------|------|
| 1 | `main/main.c:117` | `'bb' undeclared` / `expected ';'` | 编辑器残留文本 `bb,0` |
| 2 | `main/main.c:159` | `expected '(' before 'vTaskDelay'` | 孤立无条件的 `if` 语句 |
| 3 | `main/ui/screens.c:252` | `'lv_font_montserrat_40' undeclared` | `CONFIG_LV_FONT_MONTSERRAT_40` 未启用 |

## 修复详情

### 1. `main/main.c` — 编辑器残留 `bb,0`

第 117 行有意外写入的文本 `bb,0`，导致后续所有变量声明（如 `speed_val`）被认为是非声明语句而报错。

**修复[text](summary.md)**：删除 `bb,0`。

### 2. `main/main.c` — 孤立 `if`

第 159 行有一个无条件的 `if`（缺少条件和花括号），导致 `vTaskDelay` 的语法错误。

**修复**：删除孤立的 `if`。 

### 3. `sdkconfig` — 字体未启用

`ui/screens.c` 使用了 `lv_font_montserrat_40`，但 `sdkconfig` 中 `CONFIG_LV_FONT_MONTSERRAT_40` 未设置。

**修复**：
```
# sdkconfig — 启用字体
CONFIG_LV_FONT_MONTSERRAT_40=y
```

## 分区问题修复

固件 `lvgl_transplant.bin` 大小约 **9.1MB**，但：
- 默认分区表 `partitions_singleapp.csv` 的 `factory` 分区仅 **1MB**
- 闪存大小配置为 **2MB**

### 修复

| 修改 | 前值 | 后值 |
|------|------|------|
| 闪存大小 | `2MB` | `16MB` |
| 分区表类型 | `SINGLE_APP` | `CUSTOM` |
| 分区表文件 | `partitions_singleapp.csv` | `partitions-16MiB.csv` |
| `facsummary.mdtory` 分区大小 | `0x100000` (1MB) | `0xE00000` (14MB) |
| `partitions.csv` | 不存在 | 从 `partitions-16MiB.csv` 复制 |

**构建结果**：
```
lvgl_transplant.bin binary size 0x8f1600 bytes.
Smallest app partition is 0xe00000 bytes.
0x50ea00 bytes (36%) free.
```

---

# 编译与运行问题修复总结（四）— 2026-07-28

## 问题 1：EEZ Studio 导出头文件 `lvgl.h` 包含路径错误

### 现象

```
fatal error: lvgl/lvgl.h: No such file or directory
```

### 根因

EEZ Studio 导出的 6 个头文件使用了 `#include <lvgl/lvgl.h>`，但项目通过 `-DLV_CONF_INCLUDE_SIMPLE` 编译，LVGL 头文件直接位于包含路径下，应使用 `#include "lvgl.h"`。

### 涉及文件

| 文件 | 修复 |
|------|------|
| `main/ui/styles.h` | `#include <lvgl/lvgl.h>` → `#include "lvgl.h"` |
| `main/ui/ui.h` | 同上 |
| `main/ui/images.h` | 同上 |
| `main/ui/screens.h` | 同上 |
| `main/ui/fonts.h` | 同上 |
| `main/ui/actions.h` | 同上 |

## 问题 2：`main/CMakeLists.txt` 引用空目录

### 现象

```
CMake Warning: No source files found for SRC_DIRS entry 'ui/fonts'.
```

### 修复

`main/CMakeLists.txt` 中移除 `ui/fonts` 的 `SRC_DIRS` 和 `INCLUDE_DIRS` 条目。

## 问题 3：`create_screens()` 字体语法错误

### 现象

```c
lv_theme_t *theme = lv_theme_default_init(..., LV_FONT_DEFAULTui_font_1111);
```

`LV_FONT_DEFAULT` 和 `ui_font_1111` 两个标识符被错误地连接在一起（无逗号或取地址符），这是无效 C 语法。

### 修复

```c
lv_theme_t *theme = lv_theme_default_init(..., &ui_font_1111);
```

使用 `&ui_font_1111` 作为主题默认字体。

## 问题 4：`screens.c` 缺少 `get_var_*()` 函数定义

### 现象

```
error: implicit declaration of function 'get_var_soc'
error: implicit declaration of function 'get_var_ready'
...
```

### 根因

EEZ Studio 导出的 `screens.c` 的 `tick_*` 函数中调用了 `get_var_soc()`、`get_var_lv()`、`get_var_ready()`、`get_var_speed()` 等 11 个 `get_var_*()` 函数，但这些函数未在任何地方定义。

### 修复

在 `screens.c` 顶部添加 forward declarations，并在文件末尾添加 stub 实现（返回 0 或空字符串），后续可由 CAN 数据桥接替换。

## 问题 5：白屏 — PSRAM 绘图缓冲区分配失败

### 现象

```
I (3454) lvgl_demo: LVGL draw buf0: 0x491c9a84, buf1: 0    ← buf1 为 NULL!
```

### 根因

`lv_port_disp_init()` 试图从 PSRAM 分配 3 个大型缓冲区：
- `lvgl_buf[0]` = 1920×1080×4 = **8MB**
- `lvgl_buf[1]` = 1920×1080×4 = **8MB** ❌ 分配失败
- `g_rot_buf` = 1080×1920×2 = **4MB**

PSRAM 池仅 ~22MB，加上系统已有开销，第二个 8MB 连续块无法分配，`buf1=NULL` 导致 LVGL 渲染异常。

### 修复 — `main/APP/lvgl_demo.c`

```c
// 改为单全屏缓冲 + full_refresh=1
pixel_count = hor_res * ver_res;
lvgl_buf[0] = heap_caps_malloc(pixel_count * sizeof(uint32_t), MALLOC_CAP_SPIRAM);
lvgl_buf[1] = NULL;  // 不分配第二个缓冲区
disp_drv.full_refresh = 1;
```

## 问题 6：白屏 — LVGL 内部堆不足导致 `lv_mem_buf_get` 死锁

### 现象

```
E (244751) task_wdt:  - lvgl_timer (CPU 0/1)
--- 0x4803394a: lv_mem_buf_get at lv_mem.c:311
A2 : 0x00001e00    ← 请求 7680 字节 (1920×4)
```

`lvgl_timer` 任务卡在 `lv_mem_buf_get()` 超过 60 秒，触发任务看门狗。

### 根因

`sdkconfig` 中 `CONFIG_LV_MEM_SIZE_KILOBYTES=32`，LVGL 内部堆仅 **32KB**。渲染 1920×1080 全屏 ARGB8888 背景图时，LVGL 需要分配一行 7680 字节的临时缓冲区，32KB 堆碎片化后无法满足，`lv_mem_buf_get` 进入死锁。

### 修复 — `sdkconfig`

```
# 修改前
# CONFIG_LV_MEM_CUSTOM is not set
CONFIG_LV_MEM_SIZE_KILOBYTES=32

# 修改后
CONFIG_LV_MEM_CUSTOM=y
CONFIG_LV_MEM_SIZE_KILOBYTES=256
```

`CONFIG_LV_MEM_CUSTOM=y` 让 LVGL 使用系统的 `malloc/free`（可分配 PSRAM），不再受 32KB 内部堆限制。

## 问题 7：`lv_scr_load_anim` 导致白屏

### 现象

AMI 启动屏幕使用 `lv_scr_load_anim()` 做淡入动画，但动画需要 `lv_timer_handler` 反复调用才能完成。

### 修复 — `main/ui/ui.c`

```c
// 修改前
lv_scr_load_anim(screen, LV_SCR_LOAD_ANIM_FADE_IN, 200, 0, false);

// 修改后
lv_scr_load(screen);
```

直接切换屏幕，无需动画驱动。

---

## 当前状态（2026-07-28）

| 项目 | 状态 |
|------|------|
| LVGL 版本 | **8.4.0** |
| 屏幕分辨率 | **1920×1080** |
| 显示 | AMI 启动屏（Logo 背景图）、Driver View、Autonomous 三屏 |
| KEY1 切换 | Driver View ↔ Autonomous |
| CAN 数据桥 | **待接入**（`get_var_*` stub 就位） |
| 编译 | **通过** ✅ |
| 固件大小 | ~10MB / 14MB 分区，29% 剩余 |
| PSRAM 使用 | 1×8MB LVGL 缓冲 + 1×4MB 旋转缓冲 = 12MB |

### 关键配置

| 配置 | 值 | 说明 |
|------|-----|------|
| `CONFIG_LV_MEM_CUSTOM` | `y` | LVGL 使用系统 malloc（PSRAM） |
| `CONFIG_LV_MEM_SIZE` | 256 KB | 内部堆安全余量 |
| `lvgl_timer` 栈 | 8192 | 防软件渲染栈溢出 |
| `disp_drv.full_refresh` | 1 | 全屏刷新，兼容旋转缓冲 |
| 绘图缓冲 | 单缓冲 × 全屏 | 避免 PSRAM 不足 + 部分刷新死锁 |
