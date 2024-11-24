#ifndef UART_H
#define UART_H

#include "stm32f1xx_hal.h"

void uart_init(UART_HandleTypeDef *huart_rx);

void uart_run(void);

#endif
