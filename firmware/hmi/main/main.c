#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"

#include "esp_sleep.h"
#include "esp_log.h"

#include "bus/spi.h"
#include "bus/i2c.h"

#include "peripherals/buttons.h"
#include "peripherals/encoder.h"
#include "peripherals/lcd.h"
#include "peripherals/led.h"

#include "ui/index.h"

#include "control/load.h"

void app_main(void)
{
    spi_init();
    i2c_init();

    led_init();
    buttons_init();
    encoder_init();
    encoder_start();

    lcd_init();
    lcd_start(index_ui_window);

    control_init();
    control_start_task();

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
