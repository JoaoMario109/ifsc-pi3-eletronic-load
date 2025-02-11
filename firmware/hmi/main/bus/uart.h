#ifndef __BUS_UART_H__
#define __BUS_UART_H__

#include "driver/uart.h"

/** General Config */

#define UART_BAUD_RATE 115200U
#define UART_NUM UART_NUM_0
#define UART_MAX_RX_EVENT 5U

#define UART_TASKS_STACK_SIZE 2048U
#define UART_TRANSMIT_DELAY 100U

/** Prototypes */

/**
 * @brief Init default board UART settings
 * @return void
 */
void uart_init(void);

/**
 * @brief Tries to lock the UART bus access mutex
 *
 * @param timeout_ms Timeout in milliseconds to wait for the mutex, if -1, wait indefinitely
 * @return true Lock acquired
 * @return false Lock not acquired
 */
bool uart_mutex_lock(int timeout_ms);

/**
 * @brief Unlocks the UART bus access mutex
 *
 * @return void
 */
void uart_mutex_unlock(void);

#endif /** !__BUS_UART_H__ */
