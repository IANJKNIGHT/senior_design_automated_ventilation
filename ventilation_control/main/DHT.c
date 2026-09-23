#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "esp_log.h"
#include "sensor_data.h"

#define DHT11_PIN GPIO_NUM_27

static const char *TAG = "DHT11";

static esp_err_t read_dht11(uint8_t *humidity, uint8_t *temperature) {
    uint8_t data[5] = {0};

    // Phase 1: Send Start Signal (Pull down for ~20ms)
    // OUT OF CRITICAL SECTION so vTaskDelay works normally
    gpio_set_direction(DHT11_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DHT11_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(20));

    // Disable interrupts ONLY for the microsecond-level timing section
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    portENTER_CRITICAL(&mux);

    // Phase 2: Pull high for 30us and switch to input with pull-up
    gpio_set_level(DHT11_PIN, 1);
    esp_rom_delay_us(30);
    gpio_set_direction(DHT11_PIN, GPIO_MODE_INPUT);
    gpio_pullup_en(DHT11_PIN);

    // Phase 3: Wait for DHT response (Low ~80us, High ~80us)
    uint16_t timeout = 0;
    while (gpio_get_level(DHT11_PIN) == 1) {
        if (++timeout > 120) {
            portEXIT_CRITICAL(&mux);
            return ESP_FAIL;
        }
        esp_rom_delay_us(1);
    }

    timeout = 0;
    while (gpio_get_level(DHT11_PIN) == 0) {
        if (++timeout > 120) {
            portEXIT_CRITICAL(&mux);
            return ESP_FAIL;
        }
        esp_rom_delay_us(1);
    }

    timeout = 0;
    while (gpio_get_level(DHT11_PIN) == 1) {
        if (++timeout > 120) {
            portEXIT_CRITICAL(&mux);
            return ESP_FAIL;
        }
        esp_rom_delay_us(1);
    }

    // Phase 4: Read 40 bits of data
    for (int i = 0; i < 40; i++) {
        timeout = 0;
        while (gpio_get_level(DHT11_PIN) == 0) {
            if (++timeout > 100) {
                portEXIT_CRITICAL(&mux);
                return ESP_FAIL;
            }
            esp_rom_delay_us(1);
        }
        
        esp_rom_delay_us(35);
        if (gpio_get_level(DHT11_PIN) == 1) {
            data[i / 8] |= (1 << (7 - (i % 8)));
            
            timeout = 0;
            while (gpio_get_level(DHT11_PIN) == 1) {
                if (++timeout > 100) {
                    portEXIT_CRITICAL(&mux);
                    return ESP_FAIL;
                }
                esp_rom_delay_us(1);
            }
        }
    }

    portEXIT_CRITICAL(&mux);

    // Phase 5: Checksum verification
    if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
        *humidity = data[0];
        *temperature = data[2];
        return ESP_OK;
    }

    return ESP_ERR_INVALID_CRC;
}

void dht11_task(void *pvParameters) {
    gpio_reset_pin(DHT11_PIN);
    gpio_pullup_en(DHT11_PIN);

    while (1) {
        uint8_t hum = 0, temp = 0;
        
        // Call read without outer critical section locking vTaskDelay
        esp_err_t res = read_dht11(&hum, &temp);

        if (res == ESP_OK) {
            // Update shared struct
            g_sensor_data.humidity = hum;
            g_sensor_data.temperature = temp;
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}