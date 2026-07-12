#ifndef EEZ_LVGL_UI_STYLES_H
#define EEZ_LVGL_UI_STYLES_H

#include <lvgl/lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Style: regular_button
lv_style_t *get_style_regular_button_MAIN_DEFAULT();
void add_style_regular_button(lv_obj_t *obj);
void remove_style_regular_button(lv_obj_t *obj);

// Style: logo
lv_style_t *get_style_logo_MAIN_DEFAULT();
void add_style_logo(lv_obj_t *obj);
void remove_style_logo(lv_obj_t *obj);

// Style: example
lv_style_t *get_style_example_MAIN_DEFAULT();
void add_style_example(lv_obj_t *obj);
void remove_style_example(lv_obj_t *obj);

// Style: lap
lv_style_t *get_style_lap_MAIN_DEFAULT();
void add_style_lap(lv_obj_t *obj);
void remove_style_lap(lv_obj_t *obj);

// Style: AMOY
lv_style_t *get_style_amoy_MAIN_DEFAULT();
void add_style_amoy(lv_obj_t *obj);
void remove_style_amoy(lv_obj_t *obj);

// Style: all
lv_style_t *get_style_all_MAIN_DEFAULT();
void add_style_all(lv_obj_t *obj);
void remove_style_all(lv_obj_t *obj);

// Style: font_size
void add_style_font_size(lv_obj_t *obj);
void remove_style_font_size(lv_obj_t *obj);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_STYLES_H*/