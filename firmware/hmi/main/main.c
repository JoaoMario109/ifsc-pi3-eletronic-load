#include "freertos/FreeRTOS.h"

#include "utils.h"

#include "bus/spi.h"
#include "bus/uart.h"

#include "control/load.h"
#include "control/menu.h"
#include "control/stream.h"

#include "peripherals/buttons.h"
#include "peripherals/encoder.h"
#include "peripherals/lcd.h"
#include "peripherals/led.h"
#include "peripherals/sd.h"

#include "server/server.h"

#include "ui/index.h"
#include "ui/menu.h"
#include "ui/stream.h"

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

  /** Load all UIs */
  ui_index_window();
  ui_menu_window();
  ui_stream_window();

  /** Control initialization */
  load_init();
  menu_init();
  stream_init();

  /** Activate uart by end since it starts communication with load */
  uart_init();

  while (1) {
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }

  LOG_EPILOG
}
