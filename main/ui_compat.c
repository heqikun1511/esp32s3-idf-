#include "ui_compat.h"

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
