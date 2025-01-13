#ifndef MCP4725_H
#define MCP4725_H

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#define MCP4725_I2C_ADDR         0x60  // Default I2C address for MCP4725
#define MCP4725_MAX_VALUE        4095  // 12-bit DAC maximum value

typedef struct {
    I2C_HandleTypeDef *i2c_handle;  // Pointer to the I2C handle
    uint16_t addr;                  // MCP4725 device address
} mcp4725_t;

/**
 * @brief Initialize MCP4725 device descriptor
 *
 * @param dev Pointer to MCP4725 descriptor
 * @param i2c_handle I2C handle
 * @param addr MCP4725 device address
 * @return HAL status code
 */
HAL_StatusTypeDef mcp4725_init(mcp4725_t *dev, I2C_HandleTypeDef *i2c_handle, uint8_t addr);

/**
 * @brief Check if MCP4725 EEPROM is busy
 *
 * @param dev Pointer to MCP4725 descriptor
 * @param busy Pointer to boolean flag for busy state
 * @return HAL status code
 */
HAL_StatusTypeDef mcp4725_eeprom_busy(mcp4725_t *dev, bool *busy);

/**
 * @brief Get MCP4725 power mode
 *
 * @param dev Pointer to MCP4725 descriptor
 * @param eeprom If true, reads EEPROM mode; otherwise reads DAC mode
 * @param mode Pointer to variable for power mode
 * @return HAL status code
 */
HAL_StatusTypeDef mcp4725_get_power_mode(mcp4725_t *dev, bool eeprom, uint8_t *mode);

/**
 * @brief Set MCP4725 power mode
 *
 * @param dev Pointer to MCP4725 descriptor
 * @param eeprom If true, writes to EEPROM; otherwise writes to DAC mode
 * @param mode Power mode to set
 * @return HAL status code
 */
HAL_StatusTypeDef mcp4725_set_power_mode(mcp4725_t *dev, bool eeprom, uint8_t mode);

/**
 * @brief Get MCP4725 raw output
 *
 * @param dev Pointer to MCP4725 descriptor
 * @param eeprom If true, reads from EEPROM; otherwise reads DAC register
 * @param value Pointer to variable for output value
 * @return HAL status code
 */
HAL_StatusTypeDef mcp4725_get_raw_output(mcp4725_t *dev, bool eeprom, uint16_t *value);

/**
 * @brief Set MCP4725 raw output
 *
 * @param dev Pointer to MCP4725 descriptor
 * @param value Output value to set (0-4095 for 12-bit resolution)
 * @param eeprom If true, writes to EEPROM; otherwise writes to DAC register
 * @return HAL status code
 */
HAL_StatusTypeDef mcp4725_set_raw_output(mcp4725_t *dev, uint16_t value, bool eeprom);

/**
 * @brief Get output voltage of MCP4725
 *
 * @param dev Pointer to MCP4725 descriptor
 * @param vdd Reference voltage in volts
 * @param eeprom If true, reads from EEPROM; otherwise reads DAC register
 * @param voltage Pointer to variable for output voltage
 * @return HAL status code
 */
HAL_StatusTypeDef mcp4725_get_voltage(mcp4725_t *dev, float vdd, bool eeprom, float *voltage);

/**
 * @brief Set output voltage of MCP4725
 *
 * @param dev Pointer to MCP4725 descriptor
 * @param vdd Reference voltage in volts
 * @param value Output voltage in volts
 * @param eeprom If true, writes to EEPROM; otherwise writes to DAC register
 * @return HAL status code
 */
HAL_StatusTypeDef mcp4725_set_voltage(mcp4725_t *dev, float vdd, float value, bool eeprom);

#endif /* MCP4725_H */
