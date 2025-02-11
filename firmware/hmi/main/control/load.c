#include "common.h"
#include "control/load.h"
#include "utils.h"

#include "peripherals/buttons.h"
#include "peripherals/encoder.h"
#include "peripherals/lcd.h"
#include "peripherals/led.h"

#include "bus/spi.h"

#include "ui/index.h"

#include <stdio.h>

/** Definitions */

#define MODULE_NAME "control.load"

/** Control state handler */
load_state_t h_load_state = {0};

/** Tasks */
TaskHandle_t h_task_control;

/** Globals */
static int pulse_counter = 0;
uint32_t *actual_set_point = &(h_load_state.control.cc.value_milli);

/** Prototypes */

static void reload_setpoint_to_encoder(void);
static void update_enabled_status(void);
static void update_setpoint_multiplier(void);
static void apply_pending_irq(void);
static void update_encoder_steps(void);
static void update_load_state(void);
static void control_task(void *pvParameters);
static void enable_task(void *pvParameters);

/**
 * @brief Initialize load control module
 * @return void
 */
void control_init(void)
{
  LOG_PROLOG

  xTaskCreate(control_task, "ctrl", CONTROL_TASK_STACK_SIZE, NULL, 1, NULL);
  xTaskCreate(enable_task, "en", CONTROL_TASK_STACK_SIZE, NULL, 1, NULL);

  LOG_EPILOG
}

/** Implementations */

static void reload_setpoint_to_encoder(void)
{
  pulse_counter = 0;
  lv_spinbox_set_value(h_value_spinbox, *actual_set_point);
}

static void update_enabled_status(void)
{
  h_load_state.control.enable = !h_load_state.control.enable;
  if (h_load_state.control.enable) {
    lv_led_on(h_led_enable);
  } else {
    lv_led_off(h_led_enable);
  }
  set_led_enable(h_load_state.control.enable);
}

static void update_setpoint_multiplier(void)
{
  if (lv_spinbox_get_step(h_value_spinbox) > 1000) {
    lv_spinbox_set_cursor_pos(h_value_spinbox, 0);
  } else {
    lv_spinbox_step_prev(h_value_spinbox);
  }
}

static void apply_pending_irq(void)
{
  const load_mode_t current_mode = h_load_state.control.mode;

  if (h_pending_button_cc) {
    h_load_state.control.mode = CC;
    actual_set_point = &(h_load_state.control.cc.value_milli);
    h_pending_button_cc = false;
  }
  if (h_pending_button_cv) {
    h_load_state.control.mode = CV;
    actual_set_point = &(h_load_state.control.cv.value_milli);
    h_pending_button_cv = false;
  }
  if (h_pending_button_cp) {
    h_load_state.control.mode = CP;
    actual_set_point = &(h_load_state.control.cp.value_milli);
    h_pending_button_cp = false;
  }
  if (h_pending_button_cr) {
    h_load_state.control.mode = CR;
    actual_set_point = &(h_load_state.control.cr.value_milli);
    h_pending_button_cr = false;
  }

  /** Case of change reload set point to encoder */
  if (current_mode != h_load_state.control.mode) {
    reload_setpoint_to_encoder();
  }

  if (h_pending_button_enc) {
    update_setpoint_multiplier();
    h_pending_button_enc = false;
  }
}

static void update_encoder_steps(void)
{
  if (pulse_counter > 0) {
    for (int i = 0; i < pulse_counter; i += 2) {
      lv_spinbox_increment(h_value_spinbox);
    }
  } else {
    for (int i = 0; i > pulse_counter; i -= 2) {
      lv_spinbox_decrement(h_value_spinbox);
    }
  }
}

static void update_load_state(void)
{
  lv_spinbox_set_value(
    h_value_current_spinbox, h_load_state.measurement.cc_milli / 100U
  );
  lv_spinbox_set_value(
    h_value_voltage_spinbox, h_load_state.measurement.cv_milli / 100
  );
  lv_spinbox_set_value(
    h_value_resistance_spinbox, h_load_state.measurement.cr_milli / 100
  );
  lv_spinbox_set_value(
    h_value_power_spinbox, h_load_state.measurement.cp_milli / 100
  );

  spi_mutex_lock(-1);

  /** Open file /sdcard/meas.csv with create or append and store measurements */
  FILE *file = fopen("/sdcard/meas.csv", "a");
  if (file == NULL) {
    return;
  }

  fprintf(
    file, "%ld,%ld,%ld,%ld\n",
    h_load_state.measurement.cc_milli,
    h_load_state.measurement.cv_milli,
    h_load_state.measurement.cr_milli,
    h_load_state.measurement.cp_milli
  );

  fclose(file);

  spi_mutex_unlock();

  switch (h_load_state.control.mode)
  {
  case CC:
    lv_label_set_text(h_value_mode_label, "CC");
    break;
  case CV:
    lv_label_set_text(h_value_mode_label, "CV");
    break;
  case CR:
    lv_label_set_text(h_value_mode_label, "CR");
    break;
  case CP:
    lv_label_set_text(h_value_mode_label, "CP");
    break;
  default:
    break;
  }
}

static void enable_task(void *pvParameters)
{
  while (1)
  {
    button_en_update();

    if (h_button_en_rising) {
      lvgl_mutex_lock(-1);
      update_enabled_status();
      lvgl_mutex_unlock();
    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

static void control_task(void *pvParameters)
{
  encoder_start();

  while (1)
  {
    if (lvgl_mutex_lock(250)) {
      /** Get changes on pulse counter */
      pcnt_unit_get_count(h_pcnt_unit, &pulse_counter);
      encoder_clear();

      /** Apply changes needed and clear pulse count if needed */
      apply_pending_irq();

      update_encoder_steps();

      update_load_state();

      /** Updates the control point */
      *actual_set_point = (uint32_t)lv_spinbox_get_value(h_value_spinbox);

      lvgl_mutex_unlock();
    }
    vTaskDelay(pdMS_TO_TICKS(CONTROL_TASK_DELAY));
  }
}
