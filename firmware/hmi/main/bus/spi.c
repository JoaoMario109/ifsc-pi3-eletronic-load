#include "common.h"
#include "bus/spi.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"

/**
 * @brief Init default board SPI bus using maximum transfer size needed by LVGL
 * @return void
 */
void spi_init(void)
{
    spi_bus_config_t buscfg = {
        .sclk_io_num = GPIO_SPI_SCK,
        .mosi_io_num = GPIO_SPI_MOSI,
        .miso_io_num = GPIO_SPI_MISO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = MAX_SPI_TRANSFER_SIZE,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
}
