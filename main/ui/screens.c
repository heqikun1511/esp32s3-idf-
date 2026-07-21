#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>

objects_t objects;

//
// Event handlers
//

lv_obj_t *tick_value_change_obj;

//
// Screens
//

void create_screen_ami() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.ami = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 1920, 1080);
    lv_obj_set_style_image_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xf5f5f5), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_image_src(obj, &img_ami_ui_board, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // DRIVING
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.driving = obj;
            lv_obj_set_pos(obj, 686, 87);
            lv_obj_set_size(obj, 205, 264);
            lv_obj_add_event_cb(obj, action_to_menu, LV_EVENT_CLICKED, (void *)0);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 346, 26);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "Button");
                }
            }
        }
        {
            // car_check
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.car_check = obj;
            lv_obj_set_pos(obj, 505, 406);
            lv_obj_set_size(obj, 202, 270);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // EBS_CHECK
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.ebs_check = obj;
            lv_obj_set_pos(obj, 686, 723);
            lv_obj_set_size(obj, 185, 268);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x8e2424), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // HIGH_SPEED_FOLLOW
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.high_speed_follow = obj;
            lv_obj_set_pos(obj, 1036, 736);
            lv_obj_set_size(obj, 217, 255);
            lv_obj_add_event_cb(obj, action_to_foll_page, LV_EVENT_CLICKED, (void *)0);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // 8
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects._8 = obj;
            lv_obj_set_pos(obj, 1219, 420);
            lv_obj_set_size(obj, 224, 256);
            lv_obj_add_event_cb(obj, action_to_eight_page, LV_EVENT_CLICKED, (void *)0);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // LINE_ACC
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.line_acc = obj;
            lv_obj_set_pos(obj, 1042, 87);
            lv_obj_set_size(obj, 221, 264);
            lv_obj_add_event_cb(obj, action_to_acc_page, LV_EVENT_CLICKED, (void *)0);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 346, 26);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "Button");
                }
            }
        }
    }
    
    tick_screen_ami();
}

void tick_screen_ami() {
}

void create_screen_menu() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.menu = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 1920, 1080);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // bestlap_time
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.bestlap_time = obj;
            lv_obj_set_pos(obj, 760, 1011);
            lv_obj_set_size(obj, 200, 50);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x180303), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_40, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_width(obj, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_ofs_x(obj, 25, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_ofs_y(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xba2b2b), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "00:00:00");
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj0 = obj;
            lv_obj_set_pos(obj, -46, 905);
            lv_obj_set_size(obj, 1920, 175);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xb62121), LV_PART_MAIN | LV_STATE_SCROLLED);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x135791), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // lastlap
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.lastlap = obj;
                    lv_obj_set_pos(obj, 1028, 13);
                    lv_obj_set_size(obj, 216, 63);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_36, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "LastLap");
                }
                {
                    // lastlap_time
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.lastlap_time = obj;
                    lv_obj_set_pos(obj, 1099, 91);
                    lv_obj_set_size(obj, 200, 50);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x180303), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_40, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_shadow_width(obj, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_shadow_ofs_x(obj, 25, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_shadow_ofs_y(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xba2b2b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "00:00:00");
                }
                {
                    // Lap
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.lap = obj;
                    lv_obj_set_pos(obj, 1348, 13);
                    lv_obj_set_size(obj, 216, 63);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_40, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "Lap\n");
                }
                {
                    // lastlap_time_1
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.lastlap_time_1 = obj;
                    lv_obj_set_pos(obj, 1382, 91);
                    lv_obj_set_size(obj, 321, 50);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x180303), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_shadow_width(obj, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_shadow_ofs_x(obj, 25, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_shadow_ofs_y(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xba2b2b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "00:00:00");
                }
                {
                    // lastlap_time_2
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.lastlap_time_2 = obj;
                    lv_obj_set_pos(obj, 784, 89);
                    lv_obj_set_size(obj, 200, 50);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x180303), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_40, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_shadow_width(obj, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_shadow_ofs_x(obj, 25, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_shadow_ofs_y(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xba2b2b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "00:00:00");
                }
            }
        }
        {
            // bestlap
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.bestlap = obj;
            lv_obj_set_pos(obj, 670, 933);
            lv_obj_set_size(obj, 216, 63);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_36, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "BestLap");
        }
        {
            // battery_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.battery_value = obj;
            lv_obj_set_pos(obj, 1670, 940);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x1a1616), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "00%");
        }
        {
            lv_obj_t *obj = lv_imagebutton_create(parent_obj);
            lv_obj_set_pos(obj, 77, 898);
            lv_obj_set_size(obj, 454, 173);
            lv_obj_set_style_bg_image_src(obj, &img_logo, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // battery_bar
            lv_obj_t *obj = lv_bar_create(parent_obj);
            objects.battery_bar = obj;
            lv_obj_set_pos(obj, 1694, 85);
            lv_obj_set_size(obj, 47, 800);
            lv_bar_set_value(obj, 25, LV_ANIM_OFF);
            lv_obj_set_style_transform_skew_y(obj, 0, LV_PART_MAIN | LV_STATE_SCROLLED);
            lv_obj_set_style_transform_skew_x(obj, 0, LV_PART_MAIN | LV_STATE_SCROLLED);
            lv_obj_set_style_pad_column(obj, 0, LV_PART_MAIN | LV_STATE_SCROLLED);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x04589a), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_image_create(parent_obj);
            lv_obj_set_pos(obj, 1687, 890);
            lv_obj_set_size(obj, 60, 75);
            lv_image_set_src(obj, &img_batt);
        }
        {
            // KM/H_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.km_h_value = obj;
            lv_obj_set_pos(obj, 1537, 838);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xf8f3f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "0000");
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj1 = obj;
            lv_obj_set_pos(obj, 1187, 95);
            lv_obj_set_size(obj, 414, 402);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x1a3bc3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x080101), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj2 = obj;
                    lv_obj_set_pos(obj, 612, -211);
                    lv_obj_set_size(obj, 123, 60);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xf1efef), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_shadow_color(obj, lv_color_hex(0xe9e1e1), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_shadow_width(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x1a1212), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_grad_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xf3eeee), LV_PART_MAIN | LV_STATE_SCROLLED);
                    lv_label_set_text_static(obj, "0000\n");
                }
            }
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj3 = obj;
            lv_obj_set_pos(obj, 1341, 192);
            lv_obj_set_size(obj, 100, 30);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x101010), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_color(obj, lv_color_hex(0xe9e1e1), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_width(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xf3eeee), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_grad_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xf3eeee), LV_PART_MAIN | LV_STATE_SCROLLED);
            lv_label_set_text_static(obj, "RPM");
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj4 = obj;
            lv_obj_set_pos(obj, 150, 95);
            lv_obj_set_size(obj, 596, 692);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x1a3bc3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x080101), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // speed
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.speed = obj;
                    lv_obj_set_pos(obj, 211, 189);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x1026ff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "000");
                }
                {
                    lv_obj_t *obj = lv_scale_create(parent_obj);
                    objects.obj5 = obj;
                    lv_obj_set_pos(obj, 92, 40);
                    lv_obj_set_size(obj, 338, 403);
                    lv_scale_set_mode(obj, LV_SCALE_MODE_ROUND_OUTER);
                    lv_scale_set_range(obj, 0, 180);
                    lv_scale_set_angle_range(obj, 270);
                    lv_scale_set_rotation(obj, 135);
                    lv_scale_set_total_tick_count(obj, 31);
                    lv_scale_set_major_tick_every(obj, 5);
                    lv_scale_set_label_show(obj, true);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x00e9db), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_arc_color(obj, lv_color_hex(0x1171d1), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_height(obj, 445, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_line_color(obj, lv_color_hex(0x0bf1c1), LV_PART_ITEMS | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_24, LV_PART_ITEMS | LV_STATE_DEFAULT);
                    lv_obj_set_style_line_color(obj, lv_color_hex(0xe9e108), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                }
            }
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj6 = obj;
            lv_obj_set_pos(obj, 385, 426);
            lv_obj_set_size(obj, 100, 30);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x101010), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_color(obj, lv_color_hex(0xe9e1e1), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_width(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xf3eeee), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_grad_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xf3eeee), LV_PART_MAIN | LV_STATE_SCROLLED);
            lv_label_set_text_static(obj, "KM/H\n");
        }
        {
            lv_obj_t *obj = lv_arc_create(parent_obj);
            objects.obj7 = obj;
            lv_obj_set_pos(obj, 298, 192);
            lv_obj_set_size(obj, 271, 360);
            lv_arc_set_range(obj, 0, 120);
            lv_arc_set_value(obj, 25);
            lv_arc_set_bg_start_angle(obj, 180);
            lv_arc_set_bg_end_angle(obj, 0);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_rounded(obj, false, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_width(obj, 25, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_color(obj, lv_color_hex(0xa1a1a1), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_width(obj, 23, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_width(obj, 24, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
        }
        {
            // RPM
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.rpm = obj;
            lv_obj_set_pos(obj, 1330, 254);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x1026ff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "0000");
        }
        {
            lv_obj_t *obj = lv_image_create(parent_obj);
            lv_obj_set_pos(obj, 834, 192);
            lv_obj_set_size(obj, 252, 256);
            lv_obj_set_style_bg_image_src(obj, &img_ns, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.obj8 = obj;
            lv_obj_set_pos(obj, 192, 682);
            lv_obj_set_size(obj, 32, 32);
            lv_led_set_color(obj, lv_color_hex(0x0000ff));
            lv_led_set_brightness(obj, 255);
        }
        {
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.obj9 = obj;
            lv_obj_set_pos(obj, 415, 682);
            lv_obj_set_size(obj, 32, 32);
            lv_led_set_color(obj, lv_color_hex(0x0000ff));
            lv_led_set_brightness(obj, 255);
        }
        {
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.obj10 = obj;
            lv_obj_set_pos(obj, 654, 682);
            lv_obj_set_size(obj, 32, 32);
            lv_led_set_color(obj, lv_color_hex(0x0000ff));
            lv_led_set_brightness(obj, 255);
        }
        {
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.obj11 = obj;
            lv_obj_set_pos(obj, 1378, 424);
            lv_obj_set_size(obj, 32, 32);
            lv_led_set_color(obj, lv_color_hex(0xff0000));
            lv_led_set_brightness(obj, 255);
        }
        {
            lv_obj_t *obj = lv_image_create(parent_obj);
            lv_obj_set_pos(obj, 1410, 744);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_fire);
        }
        {
            lv_obj_t *obj = lv_image_create(parent_obj);
            lv_obj_set_pos(obj, 375, 522);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_warn);
            lv_obj_set_style_image_opa(obj, 125, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj12 = obj;
            lv_obj_set_pos(obj, 403, 800);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x252121), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_34, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "current");
        }
        {
            // current_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.current_value = obj;
            lv_obj_set_pos(obj, 549, 841);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xdcdcdc), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_40, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "00MA");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj13 = obj;
            lv_obj_set_pos(obj, 86, 800);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x252121), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_34, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "voltage");
        }
        {
            // current_value_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.current_value_1 = obj;
            lv_obj_set_pos(obj, 224, 846);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xdcdcdc), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_40, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "00V");
        }
    }
    
    tick_screen_menu();
}

void tick_screen_menu() {
}

void create_screen_acc_page() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.acc_page = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 1920, 1080);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x959595), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_image_recolor(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // from_acc_page_to_AMI
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.from_acc_page_to_ami = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 221, 97);
            lv_obj_add_event_cb(obj, action_to_menu, LV_EVENT_CLICKED, (void *)0);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "Button");
                }
            }
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj14 = obj;
            lv_obj_set_pos(obj, 786, 468);
            lv_obj_set_size(obj, 130, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_36, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x1c1717), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "");
        }
        {
            lv_obj_t *obj = lv_imagebutton_create(parent_obj);
            lv_obj_set_pos(obj, 564, 811);
            lv_obj_set_size(obj, 792, 260);
            lv_obj_set_style_bg_image_src(obj, &img_logo, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 851, 428);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "drag_racing");
        }
    }
    
    tick_screen_acc_page();
}

void tick_screen_acc_page() {
}

void create_screen_foll_page() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.foll_page = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 1920, 1080);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xa9a9a9), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // from_foll_page_to_AMI_
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.from_foll_page_to_ami_ = obj;
            lv_obj_set_pos(obj, 0, 1);
            lv_obj_set_size(obj, 342, 115);
            lv_obj_add_event_cb(obj, action_to_menu, LV_EVENT_CLICKED, (void *)0);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "Button");
                }
            }
        }
        {
            lv_obj_t *obj = lv_imagebutton_create(parent_obj);
            lv_obj_set_pos(obj, 640, 675);
            lv_obj_set_size(obj, 792, 260);
            lv_obj_set_style_bg_image_src(obj, &img_logo, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 915, 423);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "high-speed tracing");
        }
    }
    
    tick_screen_foll_page();
}

void tick_screen_foll_page() {
}

void create_screen_eight_page() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.eight_page = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 1920, 1080);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x929292), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // from_eight_page_to_AMI_
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.from_eight_page_to_ami_ = obj;
            lv_obj_set_pos(obj, 13, 8);
            lv_obj_set_size(obj, 342, 115);
            lv_obj_add_event_cb(obj, action_to_menu, LV_EVENT_CLICKED, (void *)0);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "Button");
                }
            }
        }
        {
            lv_obj_t *obj = lv_imagebutton_create(parent_obj);
            lv_obj_set_pos(obj, 528, 671);
            lv_obj_set_size(obj, 792, 260);
            lv_obj_set_style_bg_image_src(obj, &img_logo, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 770, 491);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "figure-eight loop");
        }
    }
    
    tick_screen_eight_page();
}

void tick_screen_eight_page() {
}

typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_ami,
    tick_screen_menu,
    tick_screen_acc_page,
    tick_screen_foll_page,
    tick_screen_eight_page,
};
void tick_screen(int screen_index) {
    if (screen_index >= 0 && screen_index < 5) {
        tick_screen_funcs[screen_index]();
    }
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen(screenId - 1);
}

//
// Fonts
//

ext_font_desc_t fonts[] = {
#if LV_FONT_MONTSERRAT_8
    { "MONTSERRAT_8", &lv_font_montserrat_8 },
#endif
#if LV_FONT_MONTSERRAT_10
    { "MONTSERRAT_10", &lv_font_montserrat_10 },
#endif
#if LV_FONT_MONTSERRAT_12
    { "MONTSERRAT_12", &lv_font_montserrat_12 },
#endif
#if LV_FONT_MONTSERRAT_14
    { "MONTSERRAT_14", &lv_font_montserrat_14 },
#endif
#if LV_FONT_MONTSERRAT_16
    { "MONTSERRAT_16", &lv_font_montserrat_16 },
#endif
#if LV_FONT_MONTSERRAT_18
    { "MONTSERRAT_18", &lv_font_montserrat_18 },
#endif
#if LV_FONT_MONTSERRAT_20
    { "MONTSERRAT_20", &lv_font_montserrat_20 },
#endif
#if LV_FONT_MONTSERRAT_22
    { "MONTSERRAT_22", &lv_font_montserrat_22 },
#endif
#if LV_FONT_MONTSERRAT_24
    { "MONTSERRAT_24", &lv_font_montserrat_24 },
#endif
#if LV_FONT_MONTSERRAT_26
    { "MONTSERRAT_26", &lv_font_montserrat_26 },
#endif
#if LV_FONT_MONTSERRAT_28
    { "MONTSERRAT_28", &lv_font_montserrat_28 },
#endif
#if LV_FONT_MONTSERRAT_30
    { "MONTSERRAT_30", &lv_font_montserrat_30 },
#endif
#if LV_FONT_MONTSERRAT_32
    { "MONTSERRAT_32", &lv_font_montserrat_32 },
#endif
#if LV_FONT_MONTSERRAT_34
    { "MONTSERRAT_34", &lv_font_montserrat_34 },
#endif
#if LV_FONT_MONTSERRAT_36
    { "MONTSERRAT_36", &lv_font_montserrat_36 },
#endif
#if LV_FONT_MONTSERRAT_38
    { "MONTSERRAT_38", &lv_font_montserrat_38 },
#endif
#if LV_FONT_MONTSERRAT_40
    { "MONTSERRAT_40", &lv_font_montserrat_40 },
#endif
#if LV_FONT_MONTSERRAT_42
    { "MONTSERRAT_42", &lv_font_montserrat_42 },
#endif
#if LV_FONT_MONTSERRAT_44
    { "MONTSERRAT_44", &lv_font_montserrat_44 },
#endif
#if LV_FONT_MONTSERRAT_46
    { "MONTSERRAT_46", &lv_font_montserrat_46 },
#endif
#if LV_FONT_MONTSERRAT_48
    { "MONTSERRAT_48", &lv_font_montserrat_48 },
#endif
};

//
// Color themes
//

uint32_t active_theme_index = 0;

//
//
//

void create_screens() {

// Set default LVGL theme
    lv_display_t *dispp = lv_display_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
    lv_display_set_theme(dispp, theme);
    
    // Initialize screens
    // Create screens
    create_screen_ami();
    create_screen_menu();
    create_screen_acc_page();
    create_screen_foll_page();
    create_screen_eight_page();
}