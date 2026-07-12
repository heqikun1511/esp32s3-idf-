#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl/lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_MENU = 1,
    SCREEN_ID_AMI_MENU = 2,
    _SCREEN_ID_LAST = 2
};

typedef struct _objects_t {
    lv_obj_t *menu;
    lv_obj_t *ami_menu;
    lv_obj_t *obj0;
    lv_obj_t *obj1;
    lv_obj_t *label_speed;
    lv_obj_t *indicator1;
    lv_obj_t *table;
    lv_obj_t *obj2;
    lv_obj_t *rpm_value;
    lv_obj_t *bestlap;
    lv_obj_t *currentlap;
    lv_obj_t *bestlap_time;
    lv_obj_t *bestlap_time_1;
    lv_obj_t *battery_bar;
    lv_obj_t *battery_main;
    lv_obj_t *obj3;
    lv_obj_t *battery_head;
    lv_obj_t *battery_value;
    lv_obj_t *cpu;
    lv_obj_t *cpu_value;
    lv_obj_t *fps;
    lv_obj_t *fps_value;
    lv_obj_t *obj4;
    lv_obj_t *obj5;
    lv_obj_t *logo_name;
    lv_obj_t *logoname_shadow;
} objects_t;

extern objects_t objects;

void create_screen_menu();
void tick_screen_menu();

void create_screen_ami_menu();
void tick_screen_ami_menu();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/