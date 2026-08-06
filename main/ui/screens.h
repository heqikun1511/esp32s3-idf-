#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_AMI = 1,
    SCREEN_ID_AUTONOMOUS = 2,
    SCREEN_ID_DRIVER_VIEW = 3,
    _SCREEN_ID_LAST = 3
};

typedef struct _objects_t {
    lv_obj_t *ami;
    lv_obj_t *autonomous;
    lv_obj_t *driver_view;
    lv_obj_t *line_acc;
    lv_obj_t *eight;
    lv_obj_t *high_foll;
    lv_obj_t *ebs_1;
    lv_obj_t *check;
    lv_obj_t *driving;
    lv_obj_t *obj0;
    lv_obj_t *bfp_bar;
    lv_obj_t *batt;
    lv_obj_t *km_label_2;
    lv_obj_t *km_label_1;
    lv_obj_t *lv_bar_1;
    lv_obj_t *km_label_3;
    lv_obj_t *hv_bar_3;
    lv_obj_t *ts;
    lv_obj_t *motor_tem;
    lv_obj_t *__;
    lv_obj_t *km_label_5;
    lv_obj_t *v_3;
    lv_obj_t *motor;
    lv_obj_t *lv_volt;
    lv_obj_t *rotating;
    lv_obj_t *rotating_speed;
    lv_obj_t *v_4;
    lv_obj_t *ts_1;
    lv_obj_t *ts_volt;
    lv_obj_t *motor_1;
    lv_obj_t *lv_a;
    lv_obj_t *imd;
    lv_obj_t *obj1;
    lv_obj_t *speed_label_3;
    lv_obj_t *km_label_6;
    lv_obj_t *ts_6;
    lv_obj_t *ts_a;
    lv_obj_t *v;
    lv_obj_t *v_1;
    lv_obj_t *obj2;
    lv_obj_t *v_2;
    lv_obj_t *v_5;
    lv_obj_t *v_6;
    lv_obj_t *v_7;
    lv_obj_t *ebs;
    lv_obj_t *obj3;
    lv_obj_t *temp;
    lv_obj_t *obj4;
    lv_obj_t *volt;
    lv_obj_t *obj5;
    lv_obj_t *ams;
    lv_obj_t *obj6;
    lv_obj_t *as;
    lv_obj_t *obj7;
    lv_obj_t *r2d;
    lv_obj_t *r2d_1;
    lv_obj_t *sbw;
    lv_obj_t *obj8;
    lv_obj_t *ebh;
    lv_obj_t *obj9;
    lv_obj_t *ts_2;
    lv_obj_t *obj10;
    lv_obj_t *ready_label;
    lv_obj_t *middle_container;
    lv_obj_t *temp_motor_container;
    lv_obj_t *tempmotor_label;
    lv_obj_t *lap_times_container;
    lv_obj_t *laptime_label;
    lv_obj_t *lastlap_label;
    lv_obj_t *temp_inv_container;
    lv_obj_t *temp_inv_label;
    lv_obj_t *speed_container;
    lv_obj_t *speed_label;
    lv_obj_t *km_label;
    lv_obj_t *speed_label_2;
    lv_obj_t *km_label_4;
    lv_obj_t *brake_acell_presure_container;
    lv_obj_t *brake_presure_bar;
    lv_obj_t *accellerator_presure_bar;
    lv_obj_t *hv_bar;
    lv_obj_t *lv_bar;
    lv_obj_t *hv_label;
    lv_obj_t *lv_label;
    lv_obj_t *obj11;
    lv_obj_t *obj12;
    lv_obj_t *obj13;
    lv_obj_t *hv_bar_2;
    lv_obj_t *obj14;
    lv_obj_t *obj15;
    lv_obj_t *brake_presure_container_1;
    lv_obj_t *brake_presure_bar_5;
    lv_obj_t *brake_presure_bar_6;
    lv_obj_t *obj16;
} objects_t;

extern objects_t objects;

void create_screen_ami();
void tick_screen_ami();

void create_screen_autonomous();
void tick_screen_autonomous();

void create_screen_driver_view();
void tick_screen_driver_view();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/
