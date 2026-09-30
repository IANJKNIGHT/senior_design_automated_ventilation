#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "DHT.h"
#include "fan_control.h"
#include "ina219_i2c.h"
#include "pwm.h"
#include "sensor_data.h"
#include "servo_direction.h"
#include "uart.h"

#define TAG "PWM Example"

// Define tests
#define TEST_INA219_W_FAN
#define TEST_SERVO_FAN_SWEEP_W_I2C

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
extern void init_pca9685(void);

// PWM Global Variables
int fan_duty[NUM_FANS] = {10, 10, 10, 10};
int fan_gpio[NUM_FANS] = {2, 4, 5, 6};
ledc_timer_config_t ledc_fan_timer[NUM_FANS];
ledc_channel_config_t ledc_fan_channel[NUM_FANS];

// PWM Function Declarations
extern int duty_cycle_to_bits(int duty_cycle);
extern void configure_fan_pwm(void);

// Servo Control Function Declarations
extern void sweep_servo_step(void);

// UART Function Declarations
extern void init_node_hw(void);

// UART Variables
extern uint8_t rx_packet_buf[3];
volatile int rx_idx;
volatile bool packet_ready;
extern volatile bool sweep_active;

void app_main(void)
{
    // Initialize Hardware
    configure_fan_pwm();
    init_i2c();
    init_node_hw();
    init_pca9685();
    sweep_i2c();

#if defined(TEST_SERVO_FAN_SWEEP_W_I2C)
    pca9685_set_freq(50); // Set frequency to 50Hz for servo control
#endif

    // CREATE TASKS ONCE AT STARTUP
    xTaskCreatePinnedToCore(
        sweep_fan_task,
        "sweep_fan_task",
        4096,
        NULL,
        5,
        NULL,
        1
    );

#if defined(TEST_INA219_W_FAN)
    xTaskCreate(ina219_task, "ina219_task", 4096, NULL, 5, NULL);
    xTaskCreate(dht11_task, "dht11_task", 4096, NULL, 5, NULL);

    printf("Temperature,Humidity,PWM_Duty,Power\n");
#endif

    // Main background telemetry loop
    while (1)
    {
#if defined(TEST_INA219_W_FAN)
        printf("%u,%u,%d,%.2f\n",
               g_sensor_data.temperature,
               g_sensor_data.humidity,
               g_sensor_data.pwm_duty,
               g_sensor_data.power);
#endif

#if defined(TEST_SERVO_FAN_SWEEP_W_I2C)
        sweep_servo_step();
#endif

        // Yield execution so the idle task & other low priority tasks run
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}