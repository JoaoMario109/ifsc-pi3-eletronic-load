#include "common.h"
#include "control/load.h"

#include "peripherals/buttons.h"
#include "peripherals/encoder.h"
#include "peripherals/lcd.h"

#include "bus/i2c.h"

#include "ui/index.h"

#include "esp_log.h"

/** Task handlers */
TaskHandle_t h_load_task;
TaskHandle_t h_transmit_task;

uint8_t bytes[6] = {0};

/** Handlers */
uint16_t h_main_value;
bool h_load_enabled;

/** Forward Decl */

static void load_main_task(void *arg);
static void load_transmit_task(void *arg);

/**
 * @brief Initialize load control module
 * @return void
 */
void control_init(void)
{
    h_main_value = 0;
    h_load_enabled = false;
}

/**
 * @brief Start load control task
 * @return void
 */
void control_start_task(void)
{
    xTaskCreate(
        load_main_task, "LOAD", LOAD_TASK_STACK_SIZE, NULL, LOAD_TASK_PRIORITY, &h_load_task
    );
    configASSERT(h_load_task);
    xTaskCreate(
        load_transmit_task, "TRAN", TRANSMIT_TASK_STACK_SIZE, NULL, TRANSMIT_TASK_PRIORITY, &h_transmit_task
    );
    configASSERT(h_transmit_task);
}

/** Implementations */

static void load_transmit_task(void *arg)
{
    while (1)
    {
        lvgl_mutex_lock(250);
        bytes[0] = h_load_enabled;
        bytes[1] = (uint8_t)((h_main_value >> 8) & 0xFF);
        bytes[2] = (uint8_t)(h_main_value & 0xFF);
        i2c_send_bytes_to_load(bytes, 6);
        lvgl_mutex_unlock();

        vTaskDelay(pdMS_TO_TICKS(TRANSMIT_TASK_DELAY));
    }
}

static void load_main_task(void *arg)
{
    int pulse_count = 0;
    while (1)
    {
        update_buttons();

        if (lvgl_mutex_lock(250)) {

            if (h_button_en_rising) {
                h_load_enabled = !h_load_enabled;

                if (h_load_enabled) {
                    lv_led_on(h_led_enable);
                } else {
                    lv_led_off(h_led_enable);
                }
            }

            pcnt_unit_get_count(h_pcnt_unit, &pulse_count);
            encoder_clear();

            if (pulse_count > 0) {
                for (int i = 0; i < pulse_count; i += 2) {
                    lv_spinbox_increment(h_value_spinbox);
                }
            } else {
                for (int i = 0; i > pulse_count; i -= 2) {
                    lv_spinbox_decrement(h_value_spinbox);
                }
            }

            if (h_button_enc_rising) {
                if (lv_spinbox_get_step(h_value_spinbox) > 1000) {
                    lv_spinbox_set_cursor_pos(h_value_spinbox, 0);
                } else {
                    lv_spinbox_step_prev(h_value_spinbox);
                }
            }

            h_main_value = (uint16_t)lv_spinbox_get_value(h_value_spinbox);
            lvgl_mutex_unlock();
        }

        vTaskDelay(pdMS_TO_TICKS(LOAD_TASK_DELAY));
    }
}
