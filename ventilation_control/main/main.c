#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/ledc.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "sensor_data.h"

#define TAG "PWM Example"
#define NUM_FANS 3

// define tests
#define TEST_INA219_W_FAN
// #define TEST_SERVO_FAN_SWEEP_W_I2C

// Global shared struct definition
sensor_data_t g_sensor_data = {0, 0, 10, 0.0f};

// DHT Function Declarations
extern void dht11_task(void *pvParameters);

// DHT Global Variables
uint8_t *humidity;
uint8_t *temperature;

// INA219 - I2C Function Declarations 
extern void init_i2c(void);
extern void sweep_i2c(void);
extern void ina219_task(void *pvParameters);

// INA219 - I2C Global Variables
float power;

// PCA9685 - I2C Function Declarations
extern void pca9685_set_pwm(uint8_t channel, uint16_t on, uint16_t off);
extern void pca9685_write_reg(uint8_t reg, uint8_t value);
extern void pca9685_set_freq(float freq);
extern void init_pca9685();

// PWM Global Variables
int fan_duty[NUM_FANS] = {10, 10, 10};
int servo_duty[2*NUM_FANS] = {5, 5, 5, 5, 5, 5}; // Assuming two servos per fan for a total of 6 servo channels
int fan_gpio[NUM_FANS] = {2, 4, 5}; // Example GPIOs for fans
int servo_gpio[2*NUM_FANS] = {12, 13, 14, 15, 16, 17}; // Example GPIOs for servos
ledc_timer_config_t ledc_fan_timer[NUM_FANS];
ledc_channel_config_t ledc_fan_channel[NUM_FANS];
ledc_timer_config_t ledc_servo_timer[2*NUM_FANS];
ledc_channel_config_t ledc_servo_channel[2*NUM_FANS];

// PWM Function Declarations
extern int duty_cycle_to_bits(int duty_cycle);
extern void configure_fan_pwm(void);
extern void configure_servo_pwm(void);

// UART Configuration
#define UART_TX_PIN  GPIO_NUM_1  // ESP32 TX pin (connects to PC RX)
#define UART_RX_PIN  GPIO_NUM_3  // ESP32 RX pin (connects to PC RX - unused but required)
#define UART_NO_PIN  (-1)        // No pin for RTS/CTS
#define UART_BAUD    115200

// UART Send Function
void uart_send_data(const char *data);
void init_uart(void);

void app_main(void)
{
    
    configure_fan_pwm();
    init_i2c();
    sweep_i2c();
#if defined(TEST_SERVO_FAN_SWEEP_W_I2C)
    uint16_t min_count_x = 205;
    uint16_t max_count_x = 410;
    uint16_t step_size = 20;
    uint16_t x_servo_channel = 0; // Assuming we're controlling the first servo channel for the sweep
    uint16_t y_servo_channel = 1; // Assuming we're controlling the second servo channel for the sweep
    configure_servo_pwm();
    init_pca9685();
    pca9685_set_freq(50); // Set frequency to 50Hz for servo control
#endif

#if defined(TEST_INA219_W_FAN)
    init_uart();
    xTaskCreate(ina219_task, "ina219_task", 4096, NULL, 5, NULL);
    xTaskCreate(dht11_task, "dht11_task", 4096, NULL, 5, NULL);
#endif

    // Main loop
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(5000));

        if (fan_duty[0] < 100)
        {
            fan_duty[0] += 10;
        }
        else
        {
            fan_duty[0] = 0;
        }

        int bit_duty = duty_cycle_to_bits(fan_duty[0]);
        ledc_set_duty(ledc_fan_channel[0].speed_mode, ledc_fan_channel[0].channel, bit_duty);
        ledc_update_duty(ledc_fan_channel[0].speed_mode, ledc_fan_channel[0].channel);
#if defined(TEST_INA219_W_FAN)
    // Print CSV Header once before the loop (optional)
    static bool header_printed = false;
    if (!header_printed) {
        printf("Temperature,Humidity,PWM_Duty,Power\n");
        header_printed = true;
    }

    // Print CSV Row in while(1)
    printf("%u,%u,%d,%.2f\n",
           g_sensor_data.temperature,
           g_sensor_data.humidity,
           g_sensor_data.pwm_duty,
           g_sensor_data.power);
#endif
#if defined(TEST_SERVO_FAN_SWEEP_W_I2C)
        uint16_t max_count_y = max_count_x; 
        bool y_direction_up = true; // Flag to toggle Y sweep direction

        // Step through X axis
        for (uint16_t count_x = min_count_x; count_x <= max_count_x; count_x += step_size)
        {
            pca9685_set_pwm(x_servo_channel, 0, count_x);
            vTaskDelay(pdMS_TO_TICKS(100)); // Small delay for X to move

            if (y_direction_up)
            {
                // Sweep Y from min to max
                for (uint16_t count_y = min_count_x; count_y <= max_count_y; count_y += step_size)
                {
                    pca9685_set_pwm(y_servo_channel, 0, count_y);
                    vTaskDelay(pdMS_TO_TICKS(100)); // Reduced from 500ms for smoother speed
                }
            }
            else
            {
                // Sweep Y from max to min
                for (uint16_t count_y = max_count_y; count_y >= min_count_x; count_y -= step_size)
                {
                    pca9685_set_pwm(y_servo_channel, 0, count_y);
                    vTaskDelay(pdMS_TO_TICKS(100));
                    if (count_y < min_count_x + step_size) break;
                }
            }

            // Flip Y direction for the next X step
            y_direction_up = !y_direction_up;
        }

        // Return X back to start position cleanly
        for (uint16_t count_x = max_count_x; count_x >= min_count_x; count_x -= step_size)
        {
            pca9685_set_pwm(x_servo_channel, 0, count_x);
            vTaskDelay(pdMS_TO_TICKS(50));
            if (count_x < min_count_x + step_size) break;
        }
#endif

    }
}
