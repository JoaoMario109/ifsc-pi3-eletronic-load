#include <string.h>

#include "server.h"

/** Globals */

/** Byte counter for parsing */
static uint8_t current_byte = 0U;

/** Buffer to store generated message */
static uint8_t parser_msg_buffer[RX_MSG_SIZE];

/** Current parser state */
static load_parser_state_t parser_state = PARSER_WAIT_START;

/** Prototypes */
static uint32_t calculate_checksum(void *data, uint32_t size);

/**
 * @brief Parses a single byte of data from the expected message.
 *
 * @param byte Byte to be parsed
 * @return uint8_t* NULL if message is not complete, pointer to the message if complete
 */
uint8_t *parse_byte(uint8_t byte)
{
  static uint8_t magic_byte = (uint8_t)(RX_MAGIC_WORD >> 24U);

  switch (parser_state) {
    case PARSER_WAIT_START:
      if (byte == magic_byte) {
        parser_msg_buffer[current_byte++] = byte;
        if (current_byte == sizeof(RX_MAGIC_WORD)) {
          parser_state = PARSER_WAIT_DATA;
        }
      } else {
        current_byte = 0U;
      }
      break;
    case PARSER_WAIT_DATA:
      parser_msg_buffer[current_byte++] = byte;
      if (current_byte == sizeof(RX_MAGIC_WORD) + RX_DATA_SIZE) {
        parser_state = PARSER_WAIT_CS;
      }
      break;
    case PARSER_WAIT_CS:
      parser_msg_buffer[current_byte++] = byte;
      if (current_byte == RX_MSG_SIZE) {
        current_byte = 0U;
        parser_state = PARSER_WAIT_START;
        return parser_msg_buffer;
      }
      break;
    default:
      current_byte = 0U;
      parser_state = PARSER_WAIT_START;
      break;
  }

  return NULL;
}

/**
 * @brief Prepares a data struct to be transmitted.
 *
 * @param data Data struct to be transmitted
 * @param tx_buffer Buffer to store the data ready to be transmitted
 */
void tx_data(TX_DATA_TYPE *data, uint8_t *tx_buffer)
{
  TX_MSG_TYPE *tx_wrap = (TX_MSG_TYPE*)tx_buffer;

  tx_wrap->magic_word = TX_MAGIC_WORD;
  memcpy(&(tx_wrap->data), data, sizeof(TX_DATA_TYPE));
  tx_wrap->checksum = calculate_checksum(tx_buffer, sizeof(TX_MAGIC_WORD) + sizeof(TX_DATA_TYPE));;
}

/**
 * @brief Extracts the data from a received buffer.
 *
 * @param rx_buffer Buffer containing the received data
 * @param data Pointer to the struct where the data will be stored
 */
int rx_data(uint8_t *rx_buffer, RX_DATA_TYPE *data)
{
  RX_MSG_TYPE *rx_wrap = (RX_MSG_TYPE*)rx_buffer;

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

/** Implementations */

/**
 * @brief Calculate the checksum of a given data buffer.
 *
 * @param data Pointer to the data buffer
 * @param size Size of the data buffer
 * @return uint32_t Checksum of the data buffer
 */
static uint32_t calculate_checksum(void *data, uint32_t size)
{
  uint32_t checksum = 0;
  uint8_t *data_ptr = (uint8_t *)data;

  for (uint32_t i = 0; i < size; i++) {
    checksum += data_ptr[i];
  }

  return checksum;
}
