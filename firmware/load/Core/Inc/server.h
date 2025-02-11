#ifndef __SERVER_H__
#define __SERVER_H__

#include <stdint.h>

/** Comment this line to use in the Panel module */
#define LOAD_MODULE

/**
 * @brief The load operates with all values in milli base. It can operate in the following modes:
 *        - CC: Constant Current
 *        - CV: Constant Voltage
 *        - CR: Constant Resistance
 *        - CP: Constant Power
 *
 * The panel always transfers a whole `load_control_t` struct, and the load module transfers back a whole `load_state_t`
 * struct.
 */

/**
 * @enum load_mode
 * @brief Load operating modes.
 */
typedef enum load_mode
{
  CC = 0U, /**< Constant Current mode */
  CV,      /**< Constant Voltage mode */
  CR,      /**< Constant Resistance mode */
  CP       /**< Constant Power mode */
} load_mode_t;

/**
 * @brief Structure representing a set point with minimum and maximum limits.
 */
typedef struct set_point
{
  uint32_t value_milli;     /**< Target set point value in milli-units */
  uint32_t min_value_milli; /**< Minimum allowed value in milli-units */
  uint32_t max_value_milli; /**< Maximum allowed value in milli-units */
} set_point_t;

/**
 * @brief Structure representing control commands sent from the panel to the load.
 */
typedef struct load_control
{
  uint32_t enable;      /**< Enable flag (1: enabled, 0: disabled) */
  load_mode_t mode;    /**< Operating mode of the load */

  set_point_t cc;      /**< Constant Current set point */
  set_point_t cv;      /**< Constant Voltage set point */
  set_point_t cr;      /**< Constant Resistance set point */
  set_point_t cp;      /**< Constant Power set point */
} load_control_t;

/**
 * @brief Structure representing measurements acquired by the load.
 */
typedef struct load_measurement
{
  uint32_t cc_milli;    /**< Measured current in milli-amperes */
  uint32_t cv_milli;    /**< Measured voltage in milli-volts */
  uint32_t cr_milli;    /**< Measured resistance in milli-ohms */
  uint32_t cp_milli;    /**< Measured power in milli-watts */
  uint32_t temp_milli;  /**< Measured temperature in milli-degrees Celsius */
} load_measurement_t;

/**
 * @brief Structure representing the complete load state, sent from the load to the panel.
 */
typedef struct load_state
{
  load_control_t control;         /**< Current control settings */
  load_measurement_t measurement; /**< Measured values from the load */
} load_state_t;

/**
 * @brief Structure representing the control settings sent from the panel to the load.
 */
typedef struct panel_to_load
{
  uint32_t magic_word;  /**< Magic word to verify the struct */
  load_control_t data;  /**< Control settings */
  uint32_t checksum;    /**< Checksum of the struct */
} panel_to_load_t;

/**
 * @brief Structure representing the load state sent from the load to the panel.
 */
typedef struct load_to_panel
{
  uint32_t magic_word;  /**< Magic word to verify the struct */
  load_state_t data;    /**< Load state */
  uint32_t checksum;    /**< Checksum of the struct */
} load_to_panel_t;

/** Definitions */

#ifdef LOAD_MODULE
  #define TX_BUFFER_SIZE sizeof(load_to_panel_t)
  #define RX_BUFFER_SIZE sizeof(panel_to_load_t)

  #define TX_MAGIC_WORD 0x2B2B2B2B
  #define RX_MAGIC_WORD 0x2D2D2D2D

  #define TX_DATA_WRAPPER load_to_panel_t
  #define RX_DATA_WRAPPER panel_to_load_t

  #define TX_DATA_TYPE load_state_t
  #define RX_DATA_TYPE load_control_t
#else
  #define TX_BUFFER_SIZE sizeof(panel_to_load_t)
  #define RX_BUFFER_SIZE sizeof(load_to_panel_t)

  #define TX_DATA_WRAPPER panel_to_load_t
  #define RX_DATA_WRAPPER load_to_panel_t

  #define TX_MAGIC_WORD 0x2D2D2D2D
  #define RX_MAGIC_WORD 0x2B2B2B2B

  #define TX_DATA_TYPE load_control_t
  #define RX_DATA_TYPE load_state_t
#endif

/**
 * @brief Calculate the checksum of a given data buffer.
 *
 * @param data Pointer to the data buffer
 * @param size Size of the data buffer
 * @return uint32_t Checksum of the data buffer
 */
uint32_t calculate_checksum(void *data, uint32_t size);

/**
 * @brief Prepare the data to be sent.
 *
 * @param data Data to be sent
 * @param tx_buffer Buffer to store the data
 */
void prepare_tx_data(TX_DATA_TYPE *data, uint8_t *tx_buffer);

/**
 * @brief Receive the data sent by the other side.
 *
 * @param rx_buffer Buffer containing the received data
 * @param data Data to be extracted from the buffer
 * @return int 0 if the data is valid, -1 otherwise
 */
int receive_rx_data(uint8_t *rx_buffer, RX_DATA_TYPE *data);

#endif /** !__SERVER_H__ */
