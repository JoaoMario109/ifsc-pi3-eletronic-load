#include "driver/gpio.h"
#include "driver/spi_master.h"

#include "bus/spi.h"
#include "common.h"
#include "utils.h"

/** Definitions */

#define MODULE_NAME "bus.spi"

/** Resource semaphores */
SemaphoreHandle_t h_spi_bus_mutex;

/**
 * @brief Init default board SPI bus using maximum transfer size needed by LVGL
 * @return void
 */
void spi_init(void)
{
  LOG_PROLOG

  spi_bus_config_t buscfg = {
      .sclk_io_num = GPIO_SPI_SCK,
      .mosi_io_num = GPIO_SPI_MOSI,
      .miso_io_num = GPIO_SPI_MISO,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = MAX_SPI_TRANSFER_SIZE,
  };
  ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

  h_spi_bus_mutex = xSemaphoreCreateRecursiveMutex();
  assert(h_spi_bus_mutex);

  LOG_EPILOG
}

/**
 * @brief Tries to lock the SPI bus access mutex
 *
 * @param timeout_ms Timeout in milliseconds to wait for the mutex, if -1, wait indefinitely
 * @return true Lock acquired
 * @return false Lock not acquired
 */
bool spi_mutex_lock(int timeout_ms)
{
  const TickType_t timeout_ticks = (timeout_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
  return xSemaphoreTakeRecursive(h_spi_bus_mutex, timeout_ticks) == pdTRUE;
}

/**
 * @brief Unlocks the SPI bus access mutex
 *
 * @return void
 */
void spi_mutex_unlock(void)
{
  xSemaphoreGiveRecursive(h_spi_bus_mutex);
}
