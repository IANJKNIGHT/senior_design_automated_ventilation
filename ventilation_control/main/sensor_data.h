#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include <stdint.h>

typedef struct {
    uint8_t temperature;
    uint8_t humidity;
    int pwm_duty;
    float power;
} sensor_data_t;

extern sensor_data_t g_sensor_data;

#endif // SENSOR_DATA_H