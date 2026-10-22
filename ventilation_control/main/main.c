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
// #define TEST_INA219_W_FAN
// #define TEST_SERVO_FAN_SWEEP_W_I2C
// #define MAX_485_SEND_TEST
#define MAX_485_RECEIVE_TEST

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

// UART CONSTANTS
#define UART_PORT (UART_NUM_1)

// UART Function Declarations
extern void int_uart();

// UART Variables

void app_main(void)
{
    // Initialize Hardware
    configure_fan_pwm();
    init_i2c();
    init_uart();
    init_pca9685();
    sweep_i2c();
    uint8_t send_receive = {0x01}; // 0x01 for send and 0x00 for receive
    uint8_t device_ID = {0xF0};    // 0xF0
    uint8_t duty_cycle = {0x0A};   // 10% duty cycle
    uint8_t tx_data[3] = {send_receive, device_ID, duty_cycle};
    const char test_byte = 0x55; // Test byte to send down the UART

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
        1);

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

#if defined(MAX_485_SEND_TEST)
        // uart_write_bytes(UART_PORT, (const char *)tx_data, sizeof(tx_data));
        // Transmit 1 byte down the UART fifo
        // Example transmission on the sender node
        uart_write_bytes(UART_NUM_1, "Hello RS485\n", 12);

        // Block until transmit FIFO is completely empty and hardware stop bit finishes
        uart_wait_tx_done(UART_PORT, pdMS_TO_TICKS(100));

        vTaskDelay(pdMS_TO_TICKS(500));
#endif

#if defined(MAX_485_RECEIVE_TEST)
        uint8_t rx_buffer[128];
        // Read incoming bytes with a 1000ms timeout
        int length = uart_read_bytes(UART_PORT, rx_buffer, sizeof(rx_buffer) - 1, pdMS_TO_TICKS(1000));

        if (length > 0)
        {
            rx_buffer[length] = '\0'; // Null-terminate string safely
            printf("Received from bus: %s\n", rx_buffer);
        }
        else
        {
            // Optional: Keep silent or print a heartbeat
        }
#endif
#if defined(TEST_SERVO_FAN_SWEEP_W_I2C)
        sweep_servo_step();
#endif

        // Yield execution so the idle task & other low priority tasks run
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}