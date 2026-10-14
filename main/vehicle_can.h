#pragma once

#include <stdbool.h>
#include <stdint.h>

/* AMI receive-only vehicle network. These are 11-bit standard CAN IDs.
 * Existing motor-controller 29-bit frames remain in bsp_can.h. */
#define VEHICLE_CAN_ID_VCU_SAFETY_STATUS  0x140U
#define VEHICLE_CAN_ID_PEDAL_BRAKE_STATUS 0x141U
#define VEHICLE_CAN_ID_VEHICLE_DYNAMICS   0x142U
#define VEHICLE_CAN_ID_AS_STATUS          0x143U
#define VEHICLE_CAN_ID_EBS_STATUS         0x144U
#define VEHICLE_CAN_ID_BMS_PACK_STATUS    0x180U
#define VEHICLE_CAN_ID_LV_STATUS          0x181U
#define VEHICLE_CAN_ID_IMD_STATUS         0x182U
#define VEHICLE_CAN_ID_THERMAL_STATUS     0x183U

typedef struct {
    bool valid;
    uint8_t alive_counter;
    uint16_t accepted_count;
    uint16_t counter_error_count;
    uint32_t last_rx_ms;
} vehicle_can_frame_meta_t;

typedef struct {
    vehicle_can_frame_meta_t meta;
    uint8_t vehicle_state, status_bits, fault_bits, warning_bits, asms_state, ts_state;
} vehicle_can_safety_t;

typedef struct {
    vehicle_can_frame_meta_t meta;
    float apps_percent, brake_pedal_percent;
    float brake_front_pressure_bar, brake_rear_pressure_bar;
    uint8_t status_bits;
} vehicle_can_pedal_brake_t;

typedef struct {
    vehicle_can_frame_meta_t meta;
    float vehicle_speed_kph, steering_angle_deg, yaw_rate_dps;
} vehicle_can_dynamics_t;

typedef struct {
    vehicle_can_frame_meta_t meta;
    uint8_t as_state, mission, steering_state, brake_state, trigger_source, fault_code;
} vehicle_can_as_t;

typedef struct {
    vehicle_can_frame_meta_t meta;
    uint8_t state, status_bits;
    float target_pressure_bar, actual_pressure_bar;
} vehicle_can_ebs_t;

typedef struct {
    vehicle_can_frame_meta_t meta;
    float pack_voltage_v, pack_current_a, soc_percent;
    int8_t max_cell_temp_c;
} vehicle_can_bms_t;

typedef struct {
    vehicle_can_frame_meta_t meta;
    float voltage_v, current_a, soc_percent;
    uint8_t status_bits;
} vehicle_can_lv_t;

typedef struct {
    vehicle_can_frame_meta_t meta;
    uint16_t insulation_resistance_kohm, threshold_kohm;
    uint8_t state, status_bits;
} vehicle_can_imd_t;

typedef struct {
    vehicle_can_frame_meta_t meta;
    int8_t bms_max_temp_c, bms_min_temp_c, coolant_temp_c, ambient_temp_c;
    float fan_command_percent;
    uint8_t status_bits;
} vehicle_can_thermal_t;

typedef struct {
    vehicle_can_safety_t safety;
    vehicle_can_pedal_brake_t pedal_brake;
    vehicle_can_dynamics_t dynamics;
    vehicle_can_as_t autonomous;
    vehicle_can_ebs_t ebs;
    vehicle_can_bms_t bms;
    vehicle_can_lv_t lv;
    vehicle_can_imd_t imd;
    vehicle_can_thermal_t thermal;
    uint32_t rejected_dlc_count;
    uint32_t rejected_crc_count;
} vehicle_can_snapshot_t;

void vehicle_can_init(void);

/* Returns true only for a valid, recognised standard frame. Extended motor
 * frames and unknown IDs are deliberately ignored by this receive-only AMI. */
bool vehicle_can_receive(uint32_t can_id, bool is_extended, const uint8_t *data, uint8_t len);
void vehicle_can_get_snapshot(vehicle_can_snapshot_t *snapshot);
