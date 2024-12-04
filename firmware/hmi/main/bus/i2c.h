#ifndef __BUS_I2C_H__
#define __BUS_I2C_H__

#include "driver/i2c.h"
#include "driver/i2c_master.h"

/** Handlers */
extern i2c_master_bus_handle_t h_i2c0;

/** General Config */
#define LOAD_MODULE_SLAVE_ADDR 0x12U
#define I2C_CLOCK_SPEED 10000U

/** Prototypes */

/**
 * @brief Init default board I2C bus connecting to Load module
 * @return void
 */
void i2c_init(void);

/**
 * @brief Send a bytes chunk to the Load module
 *
 * @param bytes Pointer to the bytes
 * @param size Size of the bytes array
 * @return void
 */
void i2c_send_bytes_to_load(void* bytes, uint16_t size);

#endif // !__BUS_I2C_H__
