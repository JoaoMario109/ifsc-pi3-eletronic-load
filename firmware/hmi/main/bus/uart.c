#include <stdint.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/gpio.h"

#include "bus/uart.h"
#include "common.h"
#include "utils.h"
#include "control/load.h"

/** Definitions */

#define MODULE_NAME "bus.uart"

/**  Handlers */
uint8_t h_uart_tx_buffer[TX_BUFFER_SIZE];
uint8_t h_uart_rx_buffer[RX_BUFFER_SIZE];
static QueueHandle_t uart_rx_event_queue;
SemaphoreHandle_t h_uart_bus_mutex;

/** Forward Decl */
static void uart_rx_task(void *pvParameters);
static void uart_tx_task(void *pvParameters);

/**
 * @brief Init default board UART settings
 * @return void
 */
void uart_init(void)
{
  LOG_PROLOG

  const uart_config_t uart_config = {
    .baud_rate = UART_BAUD_RATE,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .source_clk = UART_SCLK_DEFAULT,
  };

  //UART_HW_FIFO_LEN

  uart_driver_install(
    UART_NUM, 2 * (TX_BUFFER_SIZE + 10), 2 * RX_BUFFER_SIZE, UART_MAX_RX_EVENT, &uart_rx_event_queue, 0
  );
  uart_param_config(UART_NUM, &uart_config);

  uart_set_pin(UART_NUM, GPIO_UART_TX, GPIO_UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

  uart_enable_pattern_det_baud_intr(UART_NUM, (char)(RX_MAGIC_WORD >> 24U), sizeof(RX_MAGIC_WORD), 9, 0, 0);
  uart_pattern_queue_reset(UART_NUM, UART_MAX_RX_EVENT);

  xTaskCreate(uart_rx_task, "uart_rx_task", UART_TASKS_STACK_SIZE, NULL, 2, NULL);
  xTaskCreate(uart_tx_task, "uart_tx_task", UART_TASKS_STACK_SIZE, NULL, 2, NULL);

  LOG_EPILOG
}

static void uart_rx_task(void *pvParameters)
{
  uart_event_t event;
  while (1) {
    if (xQueueReceive(uart_rx_event_queue, (void *)&event, (TickType_t)portMAX_DELAY)) {
      switch (event.type) {
        case UART_FIFO_OVF:
        case UART_BUFFER_FULL:
          uart_flush_input(UART_NUM);
          xQueueReset(uart_rx_event_queue);
          break;
        case UART_PATTERN_DET:
          int pos = uart_pattern_pop_pos(UART_NUM);
          if (pos != -1) {
            /** Break the checksum to avoid trash */
            h_uart_rx_buffer[RX_BUFFER_SIZE - 1] = 0U;
            //uart_read_bytes(UART_NUM, h_uart_rx_buffer + pos, RX_BUFFER_SIZE, UART_TRANSMIT_DELAY / portTICK_PERIOD_MS);
            //receive_rx_data(h_uart_rx_buffer, &h_load_state);
          }
          uart_flush_input(UART_NUM);
          break;
        default:
            break;
        }
    }
  }

  vTaskDelete(NULL);
}

static void uart_tx_task(void *pvParameters)
{
  while (1)
  {
    prepare_tx_data(&(h_load_state.control), h_uart_tx_buffer);
    uart_write_bytes_with_break(UART_NUM, h_uart_tx_buffer, TX_BUFFER_SIZE, 50);

    vTaskDelay(UART_TRANSMIT_DELAY / portTICK_PERIOD_MS);
  }
}

/**
 * @brief Tries to lock the UART bus access mutex
 *
 * @param timeout_ms Timeout in milliseconds to wait for the mutex, if -1, wait indefinitely
 * @return true Lock acquired
 * @return false Lock not acquired
 */
bool uart_mutex_lock(int timeout_ms)
{
  const TickType_t timeout_ticks = (timeout_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
  return xSemaphoreTakeRecursive(h_uart_bus_mutex, timeout_ticks) == pdTRUE;
}

/**
 * @brief Unlocks the UART bus access mutex
 *
 * @return void
 */
void uart_mutex_unlock(void)
{
  xSemaphoreGiveRecursive(h_uart_bus_mutex);
}
