#include <stdio.h>
#include "driver/i2c.h"


#define PCA9685_ADDR 0x40
#define I2C_MASTER_SCL_IO           8        // GPIO pin for SCL
#define I2C_MASTER_SDA_IO           9        // GPIO pin for SDA
#define I2C_MASTER_NUM              I2C_NUM_0
#define PCA9685_MODE1 0x00
#define PCA9685_MODE2 0x01
#define PCA9685_PRESCALE 0xFE

void pca9685_set_pwm(uint8_t channel, uint16_t on, uint16_t off)
{
    // Calculate the base register address for the selected channel
    uint8_t base_reg = 0x06 + (4 * channel);

    uint8_t buf[5];
    buf[0] = base_reg;
    buf[1] = (uint8_t)(on & 0xFF);         // ON Low byte
    buf[2] = (uint8_t)((on >> 8) & 0xFF);  // ON High byte
    buf[3] = (uint8_t)(off & 0xFF);        // OFF Low byte
    buf[4] = (uint8_t)((off >> 8) & 0xFF); // OFF High byte

    // Write all 5 bytes sequentially using Auto-Increment
    i2c_master_write_to_device(I2C_MASTER_NUM, PCA9685_ADDR, buf, sizeof(buf), pdMS_TO_TICKS(1000));
}

void pca9685_write_reg(uint8_t reg, uint8_t value)
{
    uint8_t buf[2] = {reg, value};
    // Send 2 bytes to the PCA9685 address (0x40)
    i2c_master_write_to_device(I2C_MASTER_NUM, PCA9685_ADDR, buf, sizeof(buf), pdMS_TO_TICKS(1000));
}

void pca9685_set_freq(float freq)
{
    // 1. Calculate the prescale value using the 25 MHz internal clock formula
    float prescale_val = (25000000.0 / (4096.0 * freq)) - 1.0;
    uint8_t prescale = (uint8_t)(prescale_val + 0.5); // Round to the nearest whole integer

    // 2. Read the current Mode 1 state or explicitly set the SLEEP bit (Bit 4) high
    // The chip MUST be in sleep mode to modify the PRE_SCALE register.
    pca9685_write_reg(PCA9685_MODE1, 0x10);
    vTaskDelay(pdMS_TO_TICKS(2)); // Short pause to let the oscillator stop cleanly

    // 3. Write the calculated prescale byte
    pca9685_write_reg(PCA9685_PRESCALE, prescale);

    // 4. Wake up from sleep (clear SLEEP bit, keep Auto-Increment bit 5 enabled)
    pca9685_write_reg(PCA9685_MODE1, 0x20); // 0x20 = Auto-Increment enabled, Sleep disabled
    vTaskDelay(pdMS_TO_TICKS(5));            // Wait for oscillator to stabilize

    // 5. Restart PWM channels (Bit 7 = RESTART, Bit 5 = Auto-Increment)
    pca9685_write_reg(PCA9685_MODE1, 0xA0);
}

void init_pca9685()
{
    // 1. Restart / Wake up the chip by setting Mode Register 1 (0x00) to 0x20
    // 0x20 enables the Auto-Increment feature so we can write consecutive bytes cleanly.
    pca9685_write_reg(0x00, 0x20);
    vTaskDelay(pdMS_TO_TICKS(5));; // Wait for the internal oscillator to stabilize
}