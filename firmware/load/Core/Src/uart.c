#include "uart.h"
#include "utils.h"

/** Local load state handler */
load_state_t h_load_state;

/** Local uart control and handler */
static UART_HandleTypeDef *uart;
static uint8_t uart_dma_busy = 0U;
static uint16_t uart_buffer_parsed = 0U;

/** Buffers */
static uint8_t uart_rx_buffer[RX_MSG_SIZE * 2];
static uint8_t uart_tx_buffer[TX_MSG_SIZE];

/* UART initialization */
void uart_init(UART_HandleTypeDef *huart_rx)
{
  uart = huart_rx;

  if (HAL_UARTEx_ReceiveToIdle_DMA(uart, uart_rx_buffer, RX_MSG_SIZE * 2) != HAL_OK)
  {
    LOG_ERROR("UART receive error\n");
  }
}

void uart_transmit(void)
{
  if (uart_dma_busy)
  {
    return;
  }

  tx_data(&(h_load_state.measurement), uart_tx_buffer);

  if (HAL_UART_Transmit_DMA(uart, uart_tx_buffer, TX_MSG_SIZE) == HAL_OK)
  {
    uart_dma_busy = 1U;
  }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  /** We only care about our uart */
  if (huart != uart)
  {
    return;
  }

  uart_dma_busy = 0U;
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
  /** We only care about our uart */
  if (huart != uart)
  {
    return;
  }

  while (uart_buffer_parsed != Size) {
    /* Print all the buffer */
    uint8_t *msg = parse_byte(uart_rx_buffer[uart_buffer_parsed]);

    if (msg != NULL) {
      int res = rx_data(msg, &(h_load_state.control));
      if (res < 0) {
        LOG_ERROR("RX data error\n");
      }
    }
    uart_buffer_parsed = (uart_buffer_parsed + 1) % ((RX_MSG_SIZE * 2U) + 1);
  }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  /** We only care about our uart */
  if (huart != uart)
  {
    return;
  }

  if (HAL_UARTEx_ReceiveToIdle_DMA(huart, uart_rx_buffer, RX_MSG_SIZE * 2) != HAL_OK)
  {
    LOG_ERROR("UART receive error\n");
  }
}
