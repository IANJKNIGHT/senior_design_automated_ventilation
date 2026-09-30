// #include <stdio.h>
// #include <string.h>
// #include "driver/uart.h"
// #include "driver/gpio.h"
// #include <stdlib.h>
// #include "hal/misc.h"
// #include "hal/uart_types.h"
// #include "soc/uart_reg.h"
// #include "soc/uart_struct.h"
// #include "soc/system_struct.h"
// #include "soc/system_reg.h"
// #include "soc/dport_access.h"
// #include "esp_attr.h"
// #include "hal/assert.h"
// #include "esp_intr_alloc.h"
// #include "esp_cpu.h"
// #include "esp_private/rtc_ctrl.h"
// #include "esp_private/critical_section.h"
// #include "soc/interrupts.h"
// #include "soc/soc_caps.h"
// #include "soc/gpio_sig_map.h"  // For GPIO matrix
// #include "soc/periph_defs.h"   // For peripheral signal definitions

// #define RS485_TX_MODE() gpio_put(RS485_RE_DE_PIN, 1)
// #define RS485_RX_MODE() gpio_put(RS485_RE_DE_PIN, 0)

// // Define this node's network identity (Change to 0x02 for the second board)
// #define MY_NODE_ID 0x01

// #define UART_NUM UART_NUM_0
// #define RS485_RE_DE_PIN 6
// #define RS485_RXD 44
// #define UART_TX_PIN 43
// #define UART_RX_PIN 44
// #define NODE_LED_PIN 25

// // Circular/Packet processing storage adapted from your STEP4 structures
// extern uint8_t rx_packet_buf[3];
// extern volatile int rx_idx;
// extern volatile bool packet_ready;

// void init_node_hw();

// void node_uart_rx_handler()
// {
//     // 1. Acknowledge and clear the interrupt source flags
//     uint32_t int_st = READ_PERI_REG(UART_INT_ST(UART_NUM));

//     // 2. Continuous loop: Check the Flag Register (FR) RXFE (Receive FIFO Empty) bit.
//     // While the hardware register contains data, keep extracting it!
//     if (int_st & UART_RXFIFO_FULL_INT_ST)
//     {
//         // FIFO full interrupt
//         while (READ_PERI_REG(UART_STATUS(UART_NUM)) & UART_RXFIFO_CNT)
//         {
//             uint8_t c = (uint8_t)READ_PERI_REG(UART_FIFO(UART_NUM));
//             if (!packet_ready)
//             {
//                 rx_packet_buf[rx_idx] = c;
//                 rx_idx++;

//                 if (rx_idx == 3)
//                 {
//                     packet_ready = true; // Signal main loop that a complete packet arrived
//                 }
//             }
//             if (packet_ready)
//             {
//                 printf("Packet received: 0x%02X 0x%02X 0x%02X\n", rx_packet_buf[0], rx_packet_buf[1], rx_packet_buf[2]);
//                 // Crucial: Reset tracking parameters back to zero!
//                 rx_idx = 0;
//                 packet_ready = false;
//             }
//         }
//     }
//     // Clear interrupt flag
//     WRITE_PERI_REG(UART_INT_CLR(UART_NUM), UART_RXFIFO_FULL_INT_CLR);
// }

// void init_node_hw()
// {
//     // 1. Configure UART parameters (using ESP-IDF driver for setup)
//     uart_config_t uart_config = {
//         .baud_rate = 115200,
//         .data_bits = UART_DATA_8_BITS,
//         .parity = UART_PARITY_DISABLE,
//         .stop_bits = UART_STOP_BITS_1,
//         .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
//     };

//     // Set UART parameters
//     uart_param_config(UART_NUM, &uart_config);

//     // 2. Configure pins using GPIO matrix
//     int tx_signal = U0TXD_OUT_IDX;  // UART0 TX signal
//     int rx_signal = U0RXD_IN_IDX;   // UART0 RX signal

//     gpio_set_direction(UART_TX_PIN, GPIO_MODE_OUTPUT);
//     gpio_matrix_out(UART_TX_PIN, tx_signal, false, false);

//     gpio_set_direction(UART_RX_PIN, GPIO_MODE_INPUT);
//     gpio_matrix_in(UART_RX_PIN, rx_signal, false);

//     // Install UART driver (handles FIFO and buffers)
//     uart_driver_install(UART_NUM, 256, 0, 0, NULL, 0);

//     // 2. Configure Interrupts (Low-level)
//     // Enable RX FIFO Full interrupt
//     WRITE_PERI_REG(UART_INT_ENA(UART_NUM), UART_RXFIFO_FULL_INT_ENA);

//     // Clear any existing interrupts
//     WRITE_PERI_REG(UART_INT_CLR(UART_NUM), 0x7FF);

//     // Register ISR
//     esp_intr_alloc(ETS_UART0_INTR_SOURCE, ESP_INTR_FLAG_IRAM, node_uart_rx_handler, NULL, NULL);

//     // 4. Enable Interrupt
//     esp_intr_enable(ETS_UART1_INTR_SOURCE);
// }

#include "uart.h"

// Define global packet buffer here

#define RS485_UART_PORT UART_NUM_0
#define RS485_RE_DE_PIN 6
#define RS485_TXD_PIN GPIO_NUM_43
#define RS485_RXD_PIN GPIO_NUM_44

uint8_t rx_packet_buf[3] = {0};
volatile bool sweep_active = false;

void RS485_Set_Mode(bool is_tx)
{
    gpio_set_level(RS485_RE_DE_PIN, is_tx ? 1 : 0);
}

void init_node_hw(void)
{
    // Configure RE/DE pin for RS485 direction control
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << RS485_RE_DE_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&io_conf);
    RS485_Set_Mode(false); // Default to RX mode

    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_param_config(RS485_UART_PORT, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(RS485_UART_PORT, RS485_TXD_PIN, RS485_RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(RS485_UART_PORT, 256, 0, 0, NULL, 0));
}

// Background Task to monitor incoming command packets without blocking the sweep
void rs485_rx_task(void *pvParameters)
{
    uint8_t temp_buf[64];
    while (1)
    {
        int len = uart_read_bytes(RS485_UART_PORT, temp_buf, sizeof(temp_buf), pdMS_TO_TICKS(50));
        if (len > 0)
        {
            for (int i = 0; i < len; i++)
            {
                // Command protocol: 0xA5 triggers START, 0x5A triggers PAUSE
                if (temp_buf[i] == 0xA5)
                {
                    sweep_active = true;
                    ESP_LOGI("RS485", "Command Received: SWEEP START");
                }
                else if (temp_buf[i] == 0x5A)
                {
                    sweep_active = false;
                    ESP_LOGI("RS485", "Command Received: SWEEP PAUSE");
                }
                else
                {
                    sweep_active = true;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}