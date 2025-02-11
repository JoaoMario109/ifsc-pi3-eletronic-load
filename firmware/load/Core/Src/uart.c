#include "uart.h"
#include "utils.h"

static UART_HandleTypeDef *huart;
static uint8_t uart_dma_busy = 0U;

load_state_t h_load_state;

static uint8_t uart_rx_buffer[RX_BUFFER_SIZE * 2];
static uint8_t uart_tx_buffer[TX_BUFFER_SIZE];

/* UART initialization */
void uart_init(UART_HandleTypeDef *huart_rx)
{
	huart = huart_rx;

	if (HAL_UARTEx_ReceiveToIdle_DMA(huart, uart_rx_buffer, RX_BUFFER_SIZE * 2) != HAL_OK)
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

	prepare_tx_data(&h_load_state, uart_tx_buffer);

	if (HAL_UART_Transmit_DMA(huart, uart_tx_buffer, TX_BUFFER_SIZE) == HAL_OK)
	{
		uart_dma_busy = 1U;
	}
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	int res = receive_rx_data(uart_rx_buffer, &(h_load_state.control));
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	uart_dma_busy = 0U;
}


/* UART error callback */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	if (HAL_UARTEx_ReceiveToIdle_DMA(huart, uart_rx_buffer, RX_BUFFER_SIZE * 2) != HAL_OK)
	{
		LOG_ERROR("UART receive error\n");
	}
}
