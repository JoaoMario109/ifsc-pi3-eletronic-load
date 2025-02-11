#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdbool.h>

/** Hardware Mapping */

/** -- Encoder */
#define GPIO_ENCODER_BTN 33
#define GPIO_ENCODER_DT 25
#define GPIO_ENCODER_CK 26

/** -- Panel Buttons */
#define GPIO_BTN_CC 36
#define GPIO_BTN_CV 39
#define GPIO_BTN_CR 34
#define GPIO_BTN_CP 35

#define GPIO_BTN_EN 2

/** -- LEDs */
#define GPIO_LED_EN 4

/** Communication */

#define GPIO_UART_TX 1
#define GPIO_UART_RX 3

#define GPIO_SPI_MOSI 23
#define GPIO_SPI_MISO 19
#define GPIO_SPI_SCK 18

/** -- SD card */
#define GPIO_SPI_SD_CS 5

/** -- LCD */
#define GPIO_LCD_CS 17
#define GPIO_LCD_DC 15
#define GPIO_LCD_RST 16

#endif /** !__COMMON_H__ */
