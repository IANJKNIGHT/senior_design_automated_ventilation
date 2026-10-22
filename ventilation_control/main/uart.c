#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#define TXD_PIN (17)
#define RXD_PIN (18)
#define RTS_PIN (GPIO_NUM_11)// Connected to RS485 DE/RE pins tied together
#define UART_PORT (UART_NUM_1)

void init_uart(void)
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    // 1. Configure UART parameters
    uart_param_config(UART_PORT, &uart_config);

    // 2. Map GPIO pins (TX, RX, RTS, CTS)
    uart_set_pin(UART_PORT, TXD_PIN, RXD_PIN, RTS_PIN, UART_PIN_NO_CHANGE); // If you want to keep a currently allocated pin number for a specific signal, pass the macro UART_PIN_NO_CHANGE

    // 3. Install driver with TX/RX buffers
    uart_driver_install(UART_PORT, 256, 256, 0, NULL, 0);

    // 4. Set RS-485 Half-Duplex mode (Hardware automatically toggles RTS/DE line)
    // uart_set_mode(UART_PORT, UART_MODE_RS485_HALF_DUPLEX);
}

// void int_MAX_485_RX(void)
// {
//     uart_intr_config_t uart_intr = {
//         .intr_enable_mask = UART_RXFIFO_FULL_INT_ENA_M | UART_RXFIFO_TOUT_INT_ENA_M,
//     }
// }