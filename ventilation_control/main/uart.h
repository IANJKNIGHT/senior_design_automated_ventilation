#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"

// Shared global buffer declaration (defined in uart.c)
extern uint8_t rx_packet_buf[3];

// Hardware initialization interface
void init_uart(void);

#endif // UART_H