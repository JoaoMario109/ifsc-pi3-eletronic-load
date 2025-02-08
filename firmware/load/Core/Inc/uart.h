#ifndef UART_H
#define UART_H

#include "mcp4725.h"
#include "control.h"
#include "stm32f1xx_hal.h"

void uart_init(UART_HandleTypeDef *huart_rx, mcp4725_t *dac_handler, control_t *control);

void uart_run(void);

#endif
