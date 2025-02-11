#include "driver/gpio.h"

#include "common.h"
#include "utils.h"
#include "peripherals/led.h"

/** Definitions */

#define MODULE_NAME "peripherals.led"

/** Handlers */
bool h_led_on;

/**
 * @brief Set up default led configuration
 * @return void
 */
void led_init(void)
{
  LOG_PROLOG

  ESP_ERROR_CHECK(gpio_set_direction(GPIO_LED_EN, GPIO_MODE_OUTPUT));
  set_led_enable(false);

  LOG_EPILOG
}

/**
 * @brief Set led enable
 * @param en If true, turn on the led, otherwise turn off
 * @return void
 */
void set_led_enable(bool en)
{
  h_led_on = en;
  ESP_ERROR_CHECK(gpio_set_level(GPIO_LED_EN, h_led_on));
}
