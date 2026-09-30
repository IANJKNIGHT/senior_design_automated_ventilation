#include "fan_control.h"
#include "sensor_data.h"

extern sensor_data_t g_sensor_data;
extern int fan_duty[NUM_FANS];
extern ledc_channel_config_t ledc_fan_channel[NUM_FANS];
extern int duty_cycle_to_bits(int duty_cycle);

void sweep_fan_task(void *pvParameters)
{
    bool fan_count_up = true;

    while (1)
    {
        // Delay 5 seconds between duty cycle steps
        vTaskDelay(pdMS_TO_TICKS(8000));

        if (fan_duty[0] < 100 && fan_count_up)
        {
            fan_duty[0] += 10;
            if (fan_duty[0] >= 100)
                fan_count_up = false;
        }
        else if (fan_duty[0] > 0 && !fan_count_up)
        {
            fan_duty[0] -= 10;
            if (fan_duty[0] <= 0)
                fan_count_up = true;
        }

        g_sensor_data.pwm_duty = fan_duty[0];
        int bit_duty = duty_cycle_to_bits(fan_duty[0]);

        ledc_set_duty(ledc_fan_channel[0].speed_mode, ledc_fan_channel[0].channel, bit_duty);
        ledc_update_duty(ledc_fan_channel[0].speed_mode, ledc_fan_channel[0].channel);
    }
}