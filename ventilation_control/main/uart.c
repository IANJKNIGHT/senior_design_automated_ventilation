#include <stdio.h>
#include <string.h>
#include "driver/uart.h"

#define UART_TX_PIN  1  // ESP32 TX pin (connects to PC RX)
#define UART_RX_PIN  3  // ESP32 RX pin (connects to PC RX - unused but required)
#define UART_NO_PIN  (-1)        // No pin for RTS/CTS
#define UART_BAUD    115200

void init_uart(void) {
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    uart_param_config(UART_NUM_0, &uart_config);
    uart_set_pin(UART_NUM_0, UART_TX_PIN, UART_RX_PIN, UART_NO_PIN, UART_NO_PIN);
    uart_driver_install(UART_NUM_0, 256, 0, 0, NULL, 0);
    }

void uart_send_data(const char *data) {
    uart_write_bytes(UART_NUM_0, data, strlen(data));
    uart_write_bytes(UART_NUM_0, "\n", 1);  // newline delimiter
}
