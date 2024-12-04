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

#endif // !__BUS_SPI_H__
