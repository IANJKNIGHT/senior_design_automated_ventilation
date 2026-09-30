#ifndef FAN_CONTROL_H
#define FAN_CONTROL_H

#include "freertos/FreeRTOS.h"
#include "pwm.h"

void sweep_fan_task(void *pvParameters);

#endif // DHT_H