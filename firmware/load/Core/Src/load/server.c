#include "stdint.h"

#include "usart.h"

#include "load/server.h"
#include "load/common.h"

/** TX / RX Buffers */
uint8_t uart_tx_buffer[UART_TX_BUFFER_SIZE];
uint8_t uart_rx_buffer[UART_RX_BUFFER_SIZE];

/** Control vars */
volatile uint16_t data_parsed = 0U;
volatile uint16_t data_received = 0U;

volatile uint16_t tx_available = 0U;

volatile uint8_t *tx_data_head = uart_tx_buffer;

volatile server_tx_state_t tx_state = SERVER_TX_IDLE;

/** Forward declaration */

static void server_process_tx()
{

}

static void server_process_rx()
{
	while (data_parsed != data_received) {
		/** Consume byte */
		uint8_t byte = uart_rx_buffer[data_parsed];

		data_parsed = (data_parsed + 1) % UART_RX_BUFFER_SIZE;
	}
}

/** Impl */

void server_init()
{
	HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart_rx_buffer, UART_RX_BUFFER_SIZE);
}

void server_update()
{

}

/** Interruptions */

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	/** We only care abut uart1 */
	if (huart != &huart1)
	{
		return;
	}

	/** Reset TX buffer */
	if (tx_state == SERVER_TX_HALF_COMPLETE) {
		tx_available = UART_TX_BUFFER_SIZE;
	}
	tx_state = SERVER_TX_IDLE;
}

void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart)
{
	/** We only care abut uart1 */
	if (huart != &huart1)
	{
		return;
	}

	if (tx_state == SERVER_TX_STARTED) {
		tx_state = SERVER_TX_HALF_COMPLETE;
	}
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	/** We only care abut uart1 */
	if (huart != &huart1)
	{
		return;
	}

	data_received = Size;
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	/** We only care abut uart1 */
	if (huart != &huart1)
	{
		return;
	}

	server_init();
}