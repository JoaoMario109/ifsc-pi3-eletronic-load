#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>

#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#include "common.h"
#include "utils.h"
#include "bus/spi.h"
#include "peripherals/sd.h"

/** Definitions */

#define MODULE_NAME "peripherals.sd"

/** Handlers */
sdmmc_card_t *h_sd_card;
const char *h_mount_point = SD_MOUNT_POINT;

/** Forward Decl */

/**
 * @brief Mounts the SD card
 * @return void
 */
void sd_mount()
{
  LOG_PROLOG

  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
    .format_if_mount_failed = false,
    .max_files = SD_MAX_FILES,
    .allocation_unit_size = SD_ALLOCATION_UNIT
  };

  sdmmc_host_t sd_host = SDSPI_HOST_DEFAULT();
  sd_host.max_freq_khz = SD_SPI_FREQUENCY_KHZ;

  sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
  slot_config.gpio_cs = GPIO_SPI_SD_CS;
  slot_config.host_id = sd_host.slot;

  ESP_ERROR_CHECK(esp_vfs_fat_sdspi_mount(h_mount_point, &sd_host, &slot_config, &mount_config, &h_sd_card));

  LOG_EPILOG
}

/**
 * @brief Unmounts the SD card
 * @return void
 */
void sd_unmount()
{
  LOG_PROLOG

  if (h_sd_card == NULL) {
    return;
  }

  esp_vfs_fat_sdcard_unmount(h_mount_point, h_sd_card);

  LOG_EPILOG
}

/** Implementations */
