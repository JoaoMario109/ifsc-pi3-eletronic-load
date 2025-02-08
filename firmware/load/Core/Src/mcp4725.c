#include "stm32f1xx_hal.h"
#include <mcp4725.h>
#include <stdint.h>
#include <stdbool.h>

#define MCP4725_I2C_ADDR         0x60  // MCP4725 default I2C address
#define I2C_TIMEOUT_MS           1000  // I2C timeout in ms
#define CMD_DAC                  0x40
#define CMD_EEPROM               0x60
#define BIT_READY                0x80
#define MCP4725_MAX_VALUE        4095  // 12-bit DAC resolution

// Read data over I2C
HAL_StatusTypeDef read_data(mcp4725_t *dev, uint8_t *data, uint8_t size) {
	return HAL_I2C_Master_Receive(dev->i2c_handle, (dev->addr << 1), data, size, I2C_TIMEOUT_MS);
}

// Initialize MCP4725 device descriptor
HAL_StatusTypeDef mcp4725_init(mcp4725_t *dev, I2C_HandleTypeDef *i2c_handle, uint8_t addr) {
	dev->i2c_handle = i2c_handle;
	dev->addr = addr;
	return HAL_OK;
}

// Check if EEPROM is busy
HAL_StatusTypeDef mcp4725_eeprom_busy(mcp4725_t *dev, bool *busy) {
	uint8_t res;
	HAL_StatusTypeDef status = read_data(dev, &res, 1);
	if (status == HAL_OK) {
		*busy = !(res & BIT_READY);
	}
	return status;
}

// Get power mode
HAL_StatusTypeDef mcp4725_get_power_mode(mcp4725_t *dev, bool eeprom, uint8_t *mode) {
	uint8_t buf[4];
	HAL_StatusTypeDef status = read_data(dev, buf, eeprom ? 4 : 1);
	if (status == HAL_OK) {
		*mode = (eeprom ? buf[3] >> 5 : buf[0] >> 1) & 0x03;
	}
	return status;
}

// Set power mode
HAL_StatusTypeDef mcp4725_set_power_mode(mcp4725_t *dev, bool eeprom, uint8_t mode) {
	uint16_t value;
	mcp4725_get_raw_output(dev, eeprom, &value);

	uint8_t data[] = {
		(eeprom ? CMD_EEPROM : CMD_DAC) | ((mode & 3) << 1),
		(uint8_t)(value >> 4),
		(uint8_t)(value << 4)
	};
	return HAL_I2C_Master_Transmit(dev->i2c_handle, (dev->addr << 1), data, 3, I2C_TIMEOUT_MS);
}

// Get raw output
HAL_StatusTypeDef mcp4725_get_raw_output(mcp4725_t *dev, bool eeprom, uint16_t *value) {
	uint8_t buf[5];
	HAL_StatusTypeDef status = read_data(dev, buf, eeprom ? 5 : 3);
	if (status == HAL_OK) {
		*value = eeprom ? ((uint16_t)(buf[3] & 0x0F) << 8) | buf[4] : ((uint16_t)buf[0] << 4) | (buf[1] >> 4);
	}
	return status;
}

// Set raw output
HAL_StatusTypeDef mcp4725_set_raw_output(mcp4725_t *dev, uint16_t value, bool eeprom) {
	uint8_t data[] = {
		(eeprom ? CMD_EEPROM : CMD_DAC),
		(uint8_t)(value >> 4),
		(uint8_t)(value << 4)
	};
	return HAL_I2C_Master_Transmit(dev->i2c_handle, (dev->addr << 1), data, 3, I2C_TIMEOUT_MS);
}

// Get voltage output
HAL_StatusTypeDef mcp4725_get_voltage(mcp4725_t *dev, float vdd, bool eeprom, float *voltage) {
	uint16_t value;
	HAL_StatusTypeDef status = mcp4725_get_raw_output(dev, eeprom, &value);
	if (status == HAL_OK) {
		*voltage = (vdd / MCP4725_MAX_VALUE) * value;
	}
	return status;
}

// Set voltage output
HAL_StatusTypeDef mcp4725_set_voltage(mcp4725_t *dev, float vdd, float value, bool eeprom) {
	if (value < 0) {
		value = 0;
	} else if (value > vdd) {
		value = vdd;
	}

	return mcp4725_set_raw_output(dev, (uint16_t)(MCP4725_MAX_VALUE / vdd * value), eeprom);
}
