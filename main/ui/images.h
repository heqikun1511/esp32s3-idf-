#ifndef EEZ_LVGL_UI_IMAGES_H
#define EEZ_LVGL_UI_IMAGES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const lv_img_dsc_t img_ami_ui_board;
extern const lv_img_dsc_t img_logo;
extern const lv_img_dsc_t img_batt;
extern const lv_img_dsc_t img_n;
extern const lv_img_dsc_t img_ns;
extern const lv_img_dsc_t img_1;
extern const lv_img_dsc_t img_fire;
extern const lv_img_dsc_t img_warn;

#ifndef EXT_IMG_DESC_T
#define EXT_IMG_DESC_T
typedef struct _ext_img_desc_t {
    const char *name;
    const lv_img_dsc_t *img_dsc;
} ext_img_desc_t;
#endif

extern const ext_img_desc_t images[8];

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_IMAGES_H*/