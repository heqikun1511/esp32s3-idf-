#ifndef EEZ_LVGL_UI_IMAGES_H
#define EEZ_LVGL_UI_IMAGES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 定义在 ui/jpeg/image_descriptors.c (PNG压缩, LVGL按需解码到PSRAM) */
extern const lv_image_dsc_t img_ami_ui_board;
extern const lv_image_dsc_t img_acc_picture;
extern const lv_image_dsc_t img_foll_picture;
extern const lv_image_dsc_t img_eight_picture;

/* 定义在 ui_image_logo.c (原始ARGB8888像素, 小图直接编译进固件) */
extern const lv_image_dsc_t img_logo;

#ifndef EXT_IMG_DESC_T
#define EXT_IMG_DESC_T
typedef struct _ext_img_desc_t {
    const char *name;
    const lv_image_dsc_t *img_dsc;
} ext_img_desc_t;
#endif

extern const ext_img_desc_t images[5];

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_IMAGES_H*/