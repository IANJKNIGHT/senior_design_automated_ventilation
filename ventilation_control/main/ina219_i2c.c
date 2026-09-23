#include <stdio.h>
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "sensor_data.h"

#define I2C_MASTER_SCL_IO           8        // GPIO pin for SCL
#define I2C_MASTER_SDA_IO           9        // GPIO pin for SDA
#define I2C_MASTER_NUM              I2C_NUM_0  // I2C port number
#define I2C_MASTER_FREQ_HZ          100000     // I2C master clock frequency
#define I2C_MASTER_TX_BUF_DISABLE   0          // Master does not need buffer
#define I2C_MASTER_RX_BUF_DISABLE   0

#define INA219_ADDR                 0x41       // Default I2C address
#define INA219_REG_CURRENT          0x04       // Current register address
#define INA219_REG_CALIBRATION      0x05       // Calibration register address

void init_i2c(void)
{
    i2c_config_t i2c_cfg = {
        .mode = I2C_MODE_MASTER,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000, // <--- Add this (100 kHz)
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2c_cfg));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));
}

static esp_err_t ina219_write_register(uint8_t reg, uint16_t value) {
    uint8_t write_buf[3] = { reg, (uint8_t)(value >> 8), (uint8_t)(value & 0xFF) };
    return i2c_master_write_to_device(I2C_MASTER_NUM, INA219_ADDR, write_buf, sizeof(write_buf), pdMS_TO_TICKS(1000));
}

static esp_err_t ina219_read_register(uint8_t reg, uint16_t *value) {
    uint8_t data[2];
    esp_err_t err = i2c_master_write_read_device(I2C_MASTER_NUM, INA219_ADDR, &reg, 1, data, 2, pdMS_TO_TICKS(1000));
    if (err == ESP_OK) {
        *value = (uint16_t)((data[0] << 8) | data[1]);
    }
    return err;
}

void ina219_task(void *pvParameters) {
    // Write 32V / 2A calibration value (4096) to Calibration Register (0x05)
    uint16_t cal_val = 4096;
    ina219_write_register(INA219_REG_CALIBRATION, cal_val);

    while (1) {
        uint16_t raw_current;
        if (ina219_read_register(INA219_REG_CURRENT, &raw_current) == ESP_OK) {
            int16_t signed_current = (int16_t)raw_current;
            // With 4096 calibration and 0.1 ohm shunt, Current LSB = 100uA (0.1 mA) per bit
            float current_mA = signed_current * 0.1f;
            g_sensor_data.power = current_mA * 12;
            printf("Current: %.2f mA", current_mA);
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void sweep_i2c(void) {
    uint8_t address;
    esp_err_t ret;
    uint8_t dummy_reg = 0x00; // Probe register 0x00

    printf("Scanning I2C bus...\n");
    for (address = 1; address < 127; address++) {
        // Write 1 byte to probe device ACK
        ret = i2c_master_write_to_device(I2C_NUM_0, address, &dummy_reg, 1, pdMS_TO_TICKS(50));
        if (ret == ESP_OK) {
            printf("Found device at 0x%02X\n", address);
        }
    }
    printf("I2C scan complete.\n");
}