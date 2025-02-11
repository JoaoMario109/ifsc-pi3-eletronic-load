#ifndef __BUS_SPI_H__
#define __BUS_SPI_H__

/** General Config */
#define MAX_SPI_TRANSFER_SIZE 160 * 80 * sizeof(uint16_t)

/** Prototypes */

/**
 * @brief Init default board SPI bus using maximum transfer size needed by LVGL
 * @return void
 */
void spi_init(void);

/**
 * @brief Tries to lock the SPI bus access mutex
 *
 * @param timeout_ms Timeout in milliseconds to wait for the mutex, if -1, wait indefinitely
 * @return true Lock acquired
 * @return false Lock not acquired
 */
bool spi_mutex_lock(int timeout_ms);

/**
 * @brief Unlocks the SPI bus access mutex
 *
 * @return void
 */
void spi_mutex_unlock(void);

#endif /** !__BUS_SPI_H__ */
