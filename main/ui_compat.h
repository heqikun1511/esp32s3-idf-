#ifndef UI_COMPAT_H
#define UI_COMPAT_H

#include <stdint.h>

const char *get_var_ready(void);
const char *get_var_temp_motor(void);
const char *get_var_temp_inv(void);
const char *get_var_speed(void);
int32_t get_var_brake_pedal_pressure(void);
int32_t get_var_accell_pedal_pressure(void);
int32_t get_var_soc(void);
int32_t get_var_lv(void);
const char *get_var_soc_____(void);
const char *get_var_lv___v_(void);
const char *get_var__mission__(void);

#endif
