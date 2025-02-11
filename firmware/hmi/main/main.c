#include "freertos/FreeRTOS.h"

#include "utils.h"

#include "bus/spi.h"
#include "bus/uart.h"

#include "peripherals/buttons.h"
#include "peripherals/encoder.h"
#include "peripherals/led.h"
#include "peripherals/lcd.h"
#include "peripherals/sd.h"
#include "server/server.h"
#include "control/load.h"

/** Definitions */
#define MODULE_NAME "main"

void app_main(void)
{
  LOG_PROLOG

  vTaskDelay(2000 / portTICK_PERIOD_MS);

  /** BUS initialization */
  // spi_init();
  uart_init();

  // /** Board peripherals initialization */
  // led_init();
  // buttons_init();
  // encoder_init();
  // lcd_init();
  // sd_init();

  // /** Control initialization */


  // /** Starts main UI */
  // lcd_start(index_ui_window);

  // /** Starts interrupts */
  // buttons_activate();

  while (1) {
    vTaskDelay(500 / portTICK_PERIOD_MS);
    h_load_state.control.enable = 1U;
    vTaskDelay(500 / portTICK_PERIOD_MS);
    h_load_state.control.enable = 0U;
  }

  LOG_EPILOG
}
