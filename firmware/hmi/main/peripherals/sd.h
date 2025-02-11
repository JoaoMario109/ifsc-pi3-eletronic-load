#ifndef __PERIPHERALS_SD_H__
#define __PERIPHERALS_SD_H__

/** General Config */

/**
 * @brief Allocation unit size for SD card (in bytes)
 */
#define SD_ALLOCATION_UNIT (16 * 1024)

/**
 * @brief Maximum number of files that can be opened simultaneously
 */
#define SD_MAX_FILES 5

/**
 * @brief SPI communication frequency for the SD card (in kHz)
 */
#define SD_SPI_FREQUENCY_KHZ 8000

/**
 * @brief Mount point for the SD card file system
 */
#define SD_MOUNT_POINT "/sdcard"

/** Prototypes */

/**
 * @brief Initializes the SD card
 * @return void
 */
void sd_init(void);

/**
 * @brief Mounts the SD card
 * @return void
 */
void sd_mount(void);

/**
 * @brief Unmounts the SD card
 * @return void
 */
void sd_unmount(void);

#endif /** !__PERIPHERALS_SD_H__ */
