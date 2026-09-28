#include "servo_direction.h"

#include <stdio.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MIN_COUNT 205
#define MAX_COUNT 410
#define STEP_SIZE 20

extern void pca9685_set_pwm(uint8_t channel, uint16_t on, uint16_t off);

static uint16_t x_servo_channel = 0;
static uint16_t y_servo_channel = 1;

void sweep_servo_step(void)
{
    for (int pos = MIN_COUNT; pos < MAX_COUNT; pos += STEP_SIZE)
    {
        pca9685_set_pwm(x_servo_channel, 0, pos);
        pca9685_set_pwm(y_servo_channel, 0, pos);
        vTaskDelay(pdMS_TO_TICKS(250));
    }
    for (int pos = MAX_COUNT; pos > MIN_COUNT; pos -= STEP_SIZE)
    {
        pca9685_set_pwm(x_servo_channel, 0, pos);
        pca9685_set_pwm(y_servo_channel, 0, pos);
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}