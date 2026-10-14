#include "vehicle_can.h"

#include <string.h>

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"

static vehicle_can_snapshot_t s_snapshot;
static portMUX_TYPE s_vehicle_can_lock = portMUX_INITIALIZER_UNLOCKED;

static uint16_t le_u16(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static int16_t le_i16(const uint8_t *p)
{
    return (int16_t)le_u16(p);
}

/* CRC-8/SAE-J1850: poly 0x1D, init 0xFF, xorout 0xFF, no reflection.
 * It covers the 11-bit ID (two bytes) plus data bytes 0..6. */
static uint8_t crc8_update(uint8_t crc, uint8_t value)
{
    crc ^= value;
    for (unsigned bit = 0; bit < 8; ++bit) {
        crc = (crc & 0x80U) ? (uint8_t)((crc << 1) ^ 0x1DU) : (uint8_t)(crc << 1);
    }
    return crc;
}

static uint8_t vehicle_can_crc(uint32_t can_id, const uint8_t *data)
{
    uint8_t crc = 0xFFU;
    crc = crc8_update(crc, (uint8_t)(can_id >> 8));
    crc = crc8_update(crc, (uint8_t)can_id);
    for (unsigned i = 0; i < 7; ++i) crc = crc8_update(crc, data[i]);
    return (uint8_t)(crc ^ 0xFFU);
}

static void update_meta(vehicle_can_frame_meta_t *meta, uint8_t counter, uint32_t now_ms)
{
    if (meta->valid && counter != (uint8_t)((meta->alive_counter + 1U) & 0x0FU)) {
        meta->counter_error_count++;
    }
    meta->valid = true;
    meta->alive_counter = counter;
    meta->accepted_count++;
    meta->last_rx_ms = now_ms;
}

void vehicle_can_init(void)
{
    taskENTER_CRITICAL(&s_vehicle_can_lock);
    memset(&s_snapshot, 0, sizeof(s_snapshot));
    taskEXIT_CRITICAL(&s_vehicle_can_lock);
}

bool vehicle_can_receive(uint32_t can_id, bool is_extended, const uint8_t *data, uint8_t len)
{
    if (is_extended || !data || can_id < VEHICLE_CAN_ID_VCU_SAFETY_STATUS ||
        can_id > VEHICLE_CAN_ID_THERMAL_STATUS) return false;
    if (len != 8) {
        taskENTER_CRITICAL(&s_vehicle_can_lock);
        s_snapshot.rejected_dlc_count++;
        taskEXIT_CRITICAL(&s_vehicle_can_lock);
        return false;
    }
    if (vehicle_can_crc(can_id, data) != data[7]) {
        taskENTER_CRITICAL(&s_vehicle_can_lock);
        s_snapshot.rejected_crc_count++;
        taskEXIT_CRITICAL(&s_vehicle_can_lock);
        return false;
    }

    const uint32_t now_ms = (uint32_t)(esp_timer_get_time() / 1000);
    const uint8_t counter = data[6] >> 4;
    taskENTER_CRITICAL(&s_vehicle_can_lock);
    switch (can_id) {
    case VEHICLE_CAN_ID_VCU_SAFETY_STATUS:
        s_snapshot.safety.vehicle_state = data[0]; s_snapshot.safety.status_bits = data[1];
        s_snapshot.safety.fault_bits = data[2]; s_snapshot.safety.warning_bits = data[3];
        s_snapshot.safety.asms_state = data[4]; s_snapshot.safety.ts_state = data[5];
        update_meta(&s_snapshot.safety.meta, counter, now_ms); break;
    case VEHICLE_CAN_ID_PEDAL_BRAKE_STATUS:
        s_snapshot.pedal_brake.apps_percent = data[0] * 0.5f;
        s_snapshot.pedal_brake.brake_pedal_percent = data[1] * 0.5f;
        s_snapshot.pedal_brake.brake_front_pressure_bar = le_u16(&data[2]) * 0.1f;
        s_snapshot.pedal_brake.brake_rear_pressure_bar = le_u16(&data[4]) * 0.1f;
        s_snapshot.pedal_brake.status_bits = data[6] & 0x0FU;
        update_meta(&s_snapshot.pedal_brake.meta, counter, now_ms); break;
    case VEHICLE_CAN_ID_VEHICLE_DYNAMICS:
        s_snapshot.dynamics.vehicle_speed_kph = le_u16(&data[0]) * 0.01f;
        s_snapshot.dynamics.steering_angle_deg = le_i16(&data[2]) * 0.1f;
        s_snapshot.dynamics.yaw_rate_dps = le_i16(&data[4]) * 0.01f;
        update_meta(&s_snapshot.dynamics.meta, counter, now_ms); break;
    case VEHICLE_CAN_ID_AS_STATUS:
        s_snapshot.autonomous.as_state = data[0]; s_snapshot.autonomous.mission = data[1];
        s_snapshot.autonomous.steering_state = data[2]; s_snapshot.autonomous.brake_state = data[3];
        s_snapshot.autonomous.trigger_source = data[4]; s_snapshot.autonomous.fault_code = data[5];
        update_meta(&s_snapshot.autonomous.meta, counter, now_ms); break;
    case VEHICLE_CAN_ID_EBS_STATUS:
        s_snapshot.ebs.state = data[0]; s_snapshot.ebs.status_bits = data[1];
        s_snapshot.ebs.target_pressure_bar = le_u16(&data[2]) * 0.1f;
        s_snapshot.ebs.actual_pressure_bar = le_u16(&data[4]) * 0.1f;
        update_meta(&s_snapshot.ebs.meta, counter, now_ms); break;
    case VEHICLE_CAN_ID_BMS_PACK_STATUS:
        s_snapshot.bms.pack_voltage_v = le_u16(&data[0]) * 0.1f;
        s_snapshot.bms.pack_current_a = le_i16(&data[2]) * 0.1f;
        s_snapshot.bms.soc_percent = data[4] * 0.5f;
        s_snapshot.bms.max_cell_temp_c = (int8_t)data[5] - 40;
        update_meta(&s_snapshot.bms.meta, counter, now_ms); break;
    case VEHICLE_CAN_ID_LV_STATUS:
        s_snapshot.lv.voltage_v = le_u16(&data[0]) * 0.01f;
        s_snapshot.lv.current_a = le_i16(&data[2]) * 0.01f;
        s_snapshot.lv.soc_percent = data[4] == 0xFFU ? -1.0f : data[4] * 0.5f;
        s_snapshot.lv.status_bits = data[5]; update_meta(&s_snapshot.lv.meta, counter, now_ms); break;
    case VEHICLE_CAN_ID_IMD_STATUS:
        s_snapshot.imd.insulation_resistance_kohm = le_u16(&data[0]);
        s_snapshot.imd.state = data[2]; s_snapshot.imd.status_bits = data[3];
        s_snapshot.imd.threshold_kohm = le_u16(&data[4]);
        update_meta(&s_snapshot.imd.meta, counter, now_ms); break;
    case VEHICLE_CAN_ID_THERMAL_STATUS:
        s_snapshot.thermal.bms_max_temp_c = (int8_t)data[0] - 40;
        s_snapshot.thermal.bms_min_temp_c = (int8_t)data[1] - 40;
        s_snapshot.thermal.coolant_temp_c = (int8_t)data[2] - 40;
        s_snapshot.thermal.ambient_temp_c = (int8_t)data[3] - 40;
        s_snapshot.thermal.fan_command_percent = data[4] * 0.5f;
        s_snapshot.thermal.status_bits = data[5];
        update_meta(&s_snapshot.thermal.meta, counter, now_ms); break;
    default:
        taskEXIT_CRITICAL(&s_vehicle_can_lock);
        return false;
    }
    taskEXIT_CRITICAL(&s_vehicle_can_lock);
    return true;
}

void vehicle_can_get_snapshot(vehicle_can_snapshot_t *snapshot)
{
    if (!snapshot) return;
    taskENTER_CRITICAL(&s_vehicle_can_lock);
    *snapshot = s_snapshot;
    taskEXIT_CRITICAL(&s_vehicle_can_lock);
}
