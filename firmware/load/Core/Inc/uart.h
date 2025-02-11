#ifndef UART_H
#define UART_H

#include "server.h"
#include "stm32f1xx_hal.h"

extern load_state_t h_load_state;

void uart_init(UART_HandleTypeDef *huart_rx);

void uart_transmit(void);

#endif
