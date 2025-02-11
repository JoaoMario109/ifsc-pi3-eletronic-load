#include <stdio.h>
#include <string.h>

#include "server.h"

/**
 * @brief Calculate the checksum of a given data buffer.
 *
 * @param data Pointer to the data buffer
 * @param size Size of the data buffer
 * @return uint32_t Checksum of the data buffer
 */
uint32_t calculate_checksum(void *data, uint32_t size)
{
  uint32_t checksum = 0;
  uint8_t *data_ptr = (uint8_t *)data;

  for (uint32_t i = 0; i < size; i++) {
    checksum += data_ptr[i];
  }

  return checksum;
}

/**
 * @brief Prepare the data to be sent.
 *
 * @param data Data to be sent
 * @param tx_buffer Buffer to store the data
 */
void prepare_tx_data(TX_DATA_TYPE *data, uint8_t *tx_buffer)
{
  TX_DATA_WRAPPER *tx_wrap = (TX_DATA_WRAPPER*)tx_buffer;

  tx_wrap->magic_word = TX_MAGIC_WORD;
  memcpy(&(tx_wrap->data), data, sizeof(TX_DATA_TYPE));
  tx_wrap->checksum = calculate_checksum(tx_buffer, sizeof(TX_MAGIC_WORD) + sizeof(TX_DATA_TYPE));;
}

/**
 * @brief Receive the data sent by the other side.
 *
 * @param rx_buffer Buffer containing the received data
 * @param data Data to be extracted from the buffer
 * @return int 0 if the data is valid, -1 otherwise
 */
int receive_rx_data(uint8_t *rx_buffer, RX_DATA_TYPE *data)
{
  RX_DATA_WRAPPER *rx_wrap = (RX_DATA_WRAPPER*)rx_buffer;

  if (rx_wrap->magic_word != RX_MAGIC_WORD)
  {
    return -1;
  }

  uint32_t calculated_checksum = calculate_checksum(rx_buffer, sizeof(RX_MAGIC_WORD) + sizeof(RX_DATA_TYPE));
  if (rx_wrap->checksum != calculated_checksum)
  {
    return -1;
  }

  memcpy(data, &(rx_wrap->data), sizeof(RX_DATA_TYPE));

  return 0;
}
