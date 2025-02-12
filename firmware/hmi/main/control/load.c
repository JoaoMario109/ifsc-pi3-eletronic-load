#include <stdio.h>

#include "common.h"
#include "utils.h"

#include "bus/spi.h"

#include "control/load.h"
#include "control/menu.h"
#include "control/stream.h"

#include "peripherals/buttons.h"
#include "peripherals/encoder.h"
#include "peripherals/lcd.h"
#include "peripherals/led.h"

#include "ui/index.h"
#include "ui/menu.h"
#include "ui/stream.h"

/** Definitions */

#define MODULE_NAME "control.load"

/** Control state handler */
load_state_t h_load_state = {0};

/** Globals */
bool h_control_load_active = true;
bool h_control_enable_active = true;

static int pulse_counter = 0;
uint32_t *actual_set_point = &(h_load_state.control.cc.value_milli);

/** Prototypes */

static void reload_setpoint_to_encoder(void);
static void update_enabled_status(void);
static void update_setpoint_multiplier(void);
static void apply_pending_irq(void);
static void update_encoder_steps(void);
static void update_load_state(void);
static void control_load_loop();
static void check_screen_switch(void);
static void load_task(void *pvParameters);
static void enable_task(void *pvParameters);

/**
 * @brief Initialize load control module
 * @return void
 */
void load_init(void)
{
  LOG_PROLOG

  encoder_start();

  load_prepare();

  xTaskCreate(load_task, "load", LOAD_TASK_STACK_SIZE, NULL, 1, NULL);
  xTaskCreate(enable_task, "en", LOAD_TASK_STACK_SIZE, NULL, 1, NULL);

  LOG_EPILOG
}

/**
 * @brief Generic preparations for load control used when resuming from other screen
 * @return void
 */
void load_prepare(void)
{
  buttons_activate();

  /** Clear all pending IRQ and pulse counter unit */
  clear_all_pending_irq();
  pcnt_unit_clear_count(h_pcnt_unit);

  /** Set the UI src to the display */
  lcd_load_ui(h_scr_ui_index);

  /** Activate control loops */
  h_control_load_active = true;
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
    h_value_voltage_spinbox, h_load_state.measurement.cv_milli / 100U
  );
  lv_spinbox_set_value(
    h_value_resistance_spinbox, h_load_state.measurement.cr_milli / 100U
  );
  lv_spinbox_set_value(
    h_value_power_spinbox, h_load_state.measurement.cp_milli / 100U
  );

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

static void control_load_loop()
{
  /** Get changes on pulse counter */
  pcnt_unit_get_count(h_pcnt_unit, &pulse_counter);
  encoder_clear();

  /** Apply changes needed and clear pulse count if needed */
  apply_pending_irq();

  update_encoder_steps();

  update_load_state();

  /** Updates the control point */
  *actual_set_point = (uint32_t)lv_spinbox_get_value(h_value_spinbox);
}

static void check_screen_switch(void)
{
  /** This controls the flow between screens */
  if (h_button_enc_long_press) {
    /** Deactivate load loop to avoid conflicts */
    h_control_load_active = false;

    /** Reset long press tracker */
    h_button_enc_long_press = false;
    h_button_enc_last = 0;

    /** Switch to menu screen */
    menu_prepare();
  }
}

static void enable_task(void *pvParameters)
{
  while (1)
  {
    if (h_control_enable_active)
    {
      /** For enable trick detection */
      button_en_update();

      if (h_button_en_rising) {
        lvgl_mutex_lock(-1);
        update_enabled_status();
        lvgl_mutex_unlock();
      }
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

static void load_task(void *pvParameters)
{
  while (1)
  {
    /** Only run this task if activated */
    if (h_control_load_active)
    {
      /** For long press detection */
      button_enc_update();

      if (lvgl_mutex_lock(250)) {
        /** Check if need to switch to other screen */
        check_screen_switch();
        control_load_loop();
        lvgl_mutex_unlock();
      }
    }
    vTaskDelay(pdMS_TO_TICKS(LOAD_TASK_DELAY));
  }
}
