#include "ui_compat.h"
#include "ui/vars.h"

#include <string.h>

/*
 * EEZ Studio native-variable fallbacks.  The current EEZ project references
 * these getters but does not export their C implementations.  Keep the
 * defaults here so regenerated UI files remain small and the firmware links.
 */
const char *get_var_ready(void) { return ""; }
const char *get_var_temp_motor(void) { return "0"; }
const char *get_var_temp_inv(void) { return "0"; }
const char *get_var_speed(void) { return "000"; }
int32_t get_var_brake_pedal_pressure(void) { return 0; }
int32_t get_var_accell_pedal_pressure(void) { return 0; }
int32_t get_var_soc(void) { return 0; }
int32_t get_var_lv(void) { return 0; }
const char *get_var_soc_____(void) { return "0%"; }
const char *get_var_lv___v_(void) { return "0V"; }
const char *get_var__mission__(void) { return "MISSION"; }

static int32_t ami_as_state_code;

int32_t get_var_ami_as_state_code(void) { return ami_as_state_code; }
void set_var_ami_as_state_code(int32_t value) { ami_as_state_code = value; }

#define DEFINE_AMI_STRING_VAR(name, initial_value)                         \
    static char ami_##name[64] = initial_value;                            \
    const char *get_var_ami_##name(void) { return ami_##name; }            \
    void set_var_ami_##name(const char *value)                             \
    {                                                                      \
        const char *source = value != NULL ? value : "";                   \
        strncpy(ami_##name, source, sizeof(ami_##name) - 1);               \
        ami_##name[sizeof(ami_##name) - 1] = '\0';                         \
    }

DEFINE_AMI_STRING_VAR(as_state_text, "OFF")
DEFINE_AMI_STRING_VAR(test_timer, "00:00")
DEFINE_AMI_STRING_VAR(main_action, "")
DEFINE_AMI_STRING_VAR(main_detail, "")
DEFINE_AMI_STRING_VAR(main_note, "")
DEFINE_AMI_STRING_VAR(ebs_state, "UNKNOWN")
DEFINE_AMI_STRING_VAR(ts_state, "UNKNOWN")
DEFINE_AMI_STRING_VAR(brake_state, "UNKNOWN")
DEFINE_AMI_STRING_VAR(steering_state, "UNKNOWN")
DEFINE_AMI_STRING_VAR(last_command, "NONE")
DEFINE_AMI_STRING_VAR(trigger_source, "NONE")
DEFINE_AMI_STRING_VAR(reset_required, "NO")
DEFINE_AMI_STRING_VAR(reset_instruction, "")
DEFINE_AMI_STRING_VAR(footer_status, "")
DEFINE_AMI_STRING_VAR(data_age, "--")
