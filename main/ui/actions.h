#ifndef EEZ_LVGL_UI_EVENTS_H
#define EEZ_LVGL_UI_EVENTS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void action_to_menu(lv_event_t * e);
extern void action_to_acc(lv_event_t * e);
extern void action_to_acc_page(lv_event_t * e);
extern void action_to_eight_page(lv_event_t * e);
extern void action_to_foll_page(lv_event_t * e);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_EVENTS_H*/