#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// enum declarations

// Flow global variables

enum FlowGlobalVariables {
    FLOW_GLOBAL_VARIABLE_AMI_AS_STATE_CODE = 0,
    FLOW_GLOBAL_VARIABLE_AMI_AS_STATE_TEXT = 1,
    FLOW_GLOBAL_VARIABLE_AMI_TEST_TIMER = 2,
    FLOW_GLOBAL_VARIABLE_AMI_MAIN_ACTION = 3,
    FLOW_GLOBAL_VARIABLE_AMI_MAIN_DETAIL = 4,
    FLOW_GLOBAL_VARIABLE_AMI_MAIN_NOTE = 5,
    FLOW_GLOBAL_VARIABLE_AMI_EBS_STATE = 6,
    FLOW_GLOBAL_VARIABLE_AMI_TS_STATE = 7,
    FLOW_GLOBAL_VARIABLE_AMI_BRAKE_STATE = 8,
    FLOW_GLOBAL_VARIABLE_AMI_STEERING_STATE = 9,
    FLOW_GLOBAL_VARIABLE_AMI_LAST_COMMAND = 10,
    FLOW_GLOBAL_VARIABLE_AMI_TRIGGER_SOURCE = 11,
    FLOW_GLOBAL_VARIABLE_AMI_RESET_REQUIRED = 12,
    FLOW_GLOBAL_VARIABLE_AMI_RESET_INSTRUCTION = 13,
    FLOW_GLOBAL_VARIABLE_AMI_FOOTER_STATUS = 14,
    FLOW_GLOBAL_VARIABLE_AMI_DATA_AGE = 15
};

// Native global variables

extern int32_t get_var_ami_as_state_code();
extern void set_var_ami_as_state_code(int32_t value);
extern const char *get_var_ami_as_state_text();
extern void set_var_ami_as_state_text(const char *value);
extern const char *get_var_ami_test_timer();
extern void set_var_ami_test_timer(const char *value);
extern const char *get_var_ami_main_action();
extern void set_var_ami_main_action(const char *value);
extern const char *get_var_ami_main_detail();
extern void set_var_ami_main_detail(const char *value);
extern const char *get_var_ami_main_note();
extern void set_var_ami_main_note(const char *value);
extern const char *get_var_ami_ebs_state();
extern void set_var_ami_ebs_state(const char *value);
extern const char *get_var_ami_ts_state();
extern void set_var_ami_ts_state(const char *value);
extern const char *get_var_ami_brake_state();
extern void set_var_ami_brake_state(const char *value);
extern const char *get_var_ami_steering_state();
extern void set_var_ami_steering_state(const char *value);
extern const char *get_var_ami_last_command();
extern void set_var_ami_last_command(const char *value);
extern const char *get_var_ami_trigger_source();
extern void set_var_ami_trigger_source(const char *value);
extern const char *get_var_ami_reset_required();
extern void set_var_ami_reset_required(const char *value);
extern const char *get_var_ami_reset_instruction();
extern void set_var_ami_reset_instruction(const char *value);
extern const char *get_var_ami_footer_status();
extern void set_var_ami_footer_status(const char *value);
extern const char *get_var_ami_data_age();
extern void set_var_ami_data_age(const char *value);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/