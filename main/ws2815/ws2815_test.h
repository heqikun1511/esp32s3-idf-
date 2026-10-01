#pragma once

/* Runs the GPIO13 WS2815B test and keeps every configured LED at full yellow. */
void ws2815_full_yellow_test(void);

/* Starts the 16-LED RPM bar; rpm_source is updated by the CAN receive path. */
void ws2815_rpm_bar_start(volatile int *rpm_source);

/* Holds GPIO13 at a steady 3.3 V for board-level wiring verification. */
void gpio13_high_level_test(void);
