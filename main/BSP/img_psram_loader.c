#include <string.h>
#include "esp_log.h"
#include "lvgl.h"
#include "img_psram_loader.h"

/* lodepng PNG 解码器 (通过 lvgl 的 src 包含路径) */
#include "libs/lodepng/lodepng.h"

/* PNG 压缩数据 (由 convert_images_to_png.py 生成) */
#include "png_images.h"

/* 各图片的 LVGL 描述符 (去掉了 const, 允许修改 .data 指针) */
extern lv_image_dsc_t img_ami_ui_board;
extern lv_image_dsc_t img_foll_picture;
extern lv_image_dsc_t img_eight_picture;
extern lv_image_dsc_t img_acc_picture;

static const char *TAG = "img_psram";

/* ---- 图片条目 ---- */
typedef struct {
    lv_image_dsc_t *desc;    /* LVGL 描述符 (.data 将在解码后更新) */
    const png_image_t *png;  /* PNG 压缩数据 */
    const char *name;
} img_entry_t;

/* 注意: 解码所有 1920x1080 大图需要 ~24MB PSRAM.
 * 如果 PSRAM 不足, 部分解码会失败 (lodepng error 83).
 * 此时 LVGL 内置解码器会按需补偿解码. */
static img_entry_t s_images[] = {
    { &img_ami_ui_board,   &png_images[0], "ami_ui_board"   },
    { &img_foll_picture,   &png_images[1], "foll_picture"   },
    { &img_eight_picture,  &png_images[2], "eight_picture"  },
    { &img_acc_picture,    &png_images[3], "acc_picture"    },
};

static const int s_img_count = sizeof(s_images) / sizeof(s_images[0]);

int img_psram_load_all(void)
{
    int success_count = 0;

    for (int i = 0; i < s_img_count; i++) {
        img_entry_t *entry = &s_images[i];
        lv_image_dsc_t *desc = entry->desc;
        const png_image_t *png = entry->png;

        ESP_LOGI(TAG, "Decoding '%s' PNG (%zu bytes) to RGB888 in PSRAM...",
                 entry->name, png->size);

        /* 用 lodepng 解码 PNG
         * lodepng_decode24 内部使用 malloc() 分配 RGB888 缓冲区.
         * 因 CONFIG_SPIRAM_USE_MALLOC=y, 大块内存会分配到 PSRAM,
         * 所以 decoded 指针直接指向 PSRAM, 无需二次拷贝. */
        unsigned char *decoded = NULL;
        unsigned decoded_w = 0, decoded_h = 0;
        unsigned err = lodepng_decode24(&decoded, &decoded_w, &decoded_h,
                                        png->data, png->size);
        if (err || !decoded) {
            ESP_LOGE(TAG, "lodepng_decode24 failed for '%s': error %u",
                     entry->name, err);
            continue;
        }

        /* 更新描述符的宽高 (从解码结果获取, 保持一致性) */
        desc->header.w = decoded_w;
        desc->header.h = decoded_h;
        desc->header.stride = decoded_w * 3;
        desc->data_size = (uint32_t)decoded_w * decoded_h * 3;

        /* 更新颜色格式为 RGB888 (否则 LVGL 仍视为 UNKNOWN, 会尝试 PNG 解码而失败) */
        desc->header.cf = LV_COLOR_FORMAT_RGB888;

        /* 直接使用 lodepng 分配的缓冲区 (已在 PSRAM 中) */
        desc->data = decoded;

        ESP_LOGI(TAG, "'%s' decoded: %p (%ux%u RGB888)",
                 entry->name, decoded, decoded_w, decoded_h);

        success_count++;
    }

    ESP_LOGI(TAG, "Done: %d / %d images decoded to PSRAM", success_count, s_img_count);
    return success_count;
}
