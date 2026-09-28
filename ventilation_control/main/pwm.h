#ifndef PWM_H
#define PWM_H

#include <stdio.h>
#include "driver/ledc.h"
#include "esp_log.h"

#define PWM_GPIO 4
#define NUM_FANS 4
// Extern global variable for fan duty cycles
extern ledc_timer_config_t ledc_fan_timer[NUM_FANS];
extern ledc_channel_config_t ledc_fan_channel[NUM_FANS];
extern ledc_timer_config_t ledc_servo_timer[2*NUM_FANS];
extern ledc_channel_config_t ledc_servo_channel[2*NUM_FANS];
extern int fan_duty[NUM_FANS];
extern int servo_duty[2*NUM_FANS];
extern int duty_cycle_to_bits(int duty_cycle);

#endif // UART_H