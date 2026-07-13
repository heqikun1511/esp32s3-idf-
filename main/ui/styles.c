#include "styles.h"
#include "images.h"
#include "fonts.h"

#include "ui.h"
#include "screens.h"

//
// Style: regular_button
//

void init_style_regular_button_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0x940c3a));
    lv_style_set_arc_color(style, lv_color_hex(0x000000));
};

lv_style_t *get_style_regular_button_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_regular_button_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_regular_button(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_regular_button_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_regular_button(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_regular_button_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: logo
//

void init_style_logo_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_arc_opa(style, 255);
};

lv_style_t *get_style_logo_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_logo_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_logo(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_logo_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_logo(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_logo_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: example
//

void init_style_example_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_text_color(style, lv_color_hex(0x212121));
    lv_style_set_text_font(style, &lv_font_montserrat_48)lv_font_montserrat_48);
};

lv_style_t *get_style_example_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_example_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_example(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_example_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_example(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_example_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: lap
//

void init_style_lap_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_text_color(style, lv_color_hex(0x32e5b0));
    lv_style_set_bg_color(style, lv_color_hex(0xffffff));
    lv_style_set_bg_opa(style, 255);
};

lv_style_t *get_style_lap_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_lap_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_lap(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_lap_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_lap(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_lap_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: AMOY
//

void init_style_amoy_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_border_color(style, lv_color_hex(0xe40a0a));
    lv_style_set_border_opa(style, 255);
    lv_style_set_text_color(style, lv_color_hex(0xeb0909));
    lv_style_set_text_opa(style, 255);
};

lv_style_t *get_style_amoy_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_amoy_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_amoy(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_amoy_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_amoy(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_amoy_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: all
//

void init_style_all_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_text_color(style, lv_color_hex(0xe4e1e1));
    lv_style_set_text_font(style, &lv_font_montserrat_48)lv_font_montserrat_48);
};

lv_style_t *get_style_all_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_all_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_all(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_all_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_all(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_all_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: font_size
//

void add_style_font_size(lv_obj_t *obj) {
    (void)obj;
};

void remove_style_font_size(lv_obj_t *obj) {
    (void)obj;
};

//
//
//

void add_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*AddStyleFunc)(lv_obj_t *obj);
    static const AddStyleFunc add_style_funcs[] = {
        add_style_regular_button,
        add_style_logo,
        add_style_example,
        add_style_lap,
        add_style_amoy,
        add_style_all,
        add_style_font_size,
    };
    add_style_funcs[styleIndex](obj);
}

void remove_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*RemoveStyleFunc)(lv_obj_t *obj);
    static const RemoveStyleFunc remove_style_funcs[] = {
        remove_style_regular_button,
        remove_style_logo,
        remove_style_example,
        remove_style_lap,
        remove_style_amoy,
        remove_style_all,
        remove_style_font_size,
    };
    remove_style_funcs[styleIndex](obj);
}