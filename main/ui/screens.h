#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_AMI = 1,
    SCREEN_ID_MENU = 2,
    SCREEN_ID_ACC_PAGE = 3,
    SCREEN_ID_FOLL_PAGE = 4,
    SCREEN_ID_EIGHT_PAGE = 5,
    _SCREEN_ID_LAST = 5
};

typedef struct _objects_t {
    lv_obj_t *ami;
    lv_obj_t *menu;
    lv_obj_t *acc_page;
    lv_obj_t *foll_page;
    lv_obj_t *eight_page;
    lv_obj_t *driving;
    lv_obj_t *car_check;
    lv_obj_t *ebs_check;
    lv_obj_t *high_speed_follow;
    lv_obj_t *obj_8;
    lv_obj_t *line_acc;
    lv_obj_t *obj0;
    lv_obj_t *obj1;
    lv_obj_t *label_speed;
    lv_obj_t *indicator1;
    lv_obj_t *table;
    lv_obj_t *obj2;
    lv_obj_t *rpm_value;
    lv_obj_t *bestlap;
    lv_obj_t *bestlap_time;
    lv_obj_t *obj3;
    lv_obj_t *battery_value;
    lv_obj_t *cpu;
    lv_obj_t *fps;
    lv_obj_t *fps_value;
    lv_obj_t *bestlap_1;
    lv_obj_t *bestlap_time_2;
    lv_obj_t *obj4;
    lv_obj_t *obj5;
    lv_obj_t *obj6;
    lv_obj_t *obj7;
    lv_obj_t *obj8;
    lv_obj_t *obj9;
    lv_obj_t *obj10;
    lv_obj_t *obj11;
    lv_obj_t *obj12;
    lv_obj_t *obj13;
    lv_obj_t *obj14;
    lv_obj_t *obj15;
    lv_obj_t *battery_bar;
    lv_obj_t *from_acc_page_to_ami;
    lv_obj_t *obj16;
    lv_obj_t *from_foll_page_to_ami_;
    lv_obj_t *from_eight_page_to_ami_;
} objects_t;

extern objects_t objects;

void create_screen_ami();
void tick_screen_ami();

void create_screen_menu();
void tick_screen_menu();

void create_screen_acc_page();
void tick_screen_acc_page();

void create_screen_foll_page();
void tick_screen_foll_page();

void create_screen_eight_page();
void tick_screen_eight_page();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/