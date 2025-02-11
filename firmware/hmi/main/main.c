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

#include "ui/index.h"

/** Definitions */
#define MODULE_NAME "main"

void app_main(void)
{
  LOG_PROLOG

  vTaskDelay(2000 / portTICK_PERIOD_MS);

  spi_init();

  /** Board peripherals initialization */
  led_init();
  buttons_init();
  encoder_init();
  lcd_init();
  sd_init();

  /** Starts main UI */
  lcd_start(index_ui_window);

  /** Control initialization */
  control_init();

  /** Starts interrupts */
  buttons_activate();

  /** Activate uart by end since it starts communication with load */
  uart_init();

  while (1) {
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }

  LOG_EPILOG
}
