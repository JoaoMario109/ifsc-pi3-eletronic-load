#ifndef __PERIPHERALS_LED_H__
#define __PERIPHERALS_LED_H__

#include <stdbool.h>

/** Handlers */
extern bool h_led_on;

/** Prototypes */

/**
 * @brief Set up default led configuration
 * @return void
 */
void led_init(void);

/**
 * @brief Set led enable
 * @param en If true, turn on the led, otherwise turn off
 * @return void
 */
void set_led_enable(bool en);

#endif // !__PERIPHERALS_LED_H__
