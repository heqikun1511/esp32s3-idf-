#ifndef EEZ_LVGL_UI_STYLES_H
#define EEZ_LVGL_UI_STYLES_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Style: bar
lv_style_t *get_style_bar_INDICATOR_DEFAULT();
lv_style_t *get_style_bar_MAIN_DEFAULT();
void add_style_bar(lv_obj_t *obj);
void remove_style_bar(lv_obj_t *obj);

// Style: info_containers
lv_style_t *get_style_info_containers_MAIN_DEFAULT();
void add_style_info_containers(lv_obj_t *obj);
void remove_style_info_containers(lv_obj_t *obj);

// Style: text
lv_style_t *get_style_text_MAIN_DEFAULT();
void add_style_text(lv_obj_t *obj);
void remove_style_text(lv_obj_t *obj);

// Style: 1
lv_style_t *get_style_1_INDICATOR_DEFAULT();
lv_style_t *get_style_1_MAIN_DEFAULT();
void add_style_1(lv_obj_t *obj);
void remove_style_1(lv_obj_t *obj);

// Style: 2
lv_style_t *get_style_2_MAIN_DEFAULT();
void add_style_2(lv_obj_t *obj);
void remove_style_2(lv_obj_t *obj);

// Style: 3
lv_style_t *get_style_3_MAIN_DEFAULT();
void add_style_3(lv_obj_t *obj);
void remove_style_3(lv_obj_t *obj);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_STYLES_H*/