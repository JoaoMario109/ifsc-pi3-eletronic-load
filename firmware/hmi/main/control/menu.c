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

/** Handlers */
bool h_control_menu_active = false;

/** Globals */
static int pulse_counter = 0;

/** Prototypes */
static void control_menu_loop();
static void navigate_to_load(void);
static void navigate_to_stream(void);
static void check_screen_switch(void);
static void menu_task(void *pvParameters);

/**
 * @brief Initialize menu control module
 * @return void
 */
void menu_init(void)
{
  LOG_PROLOG

  xTaskCreate(menu_task, "menu", MENU_TASK_STACK_SIZE, NULL, 1, NULL);

  LOG_EPILOG
}

/**
 * @brief Generic preparations for menu control used when resuming from other screen
 * @return void
 */
void menu_prepare(void)
{
  /** Enable only encoder */
  buttons_deactivate();
  button_enc_activate();

  /** Clear all pending IRQ and pulse counter unit */
  clear_all_pending_irq();
  pcnt_unit_clear_count(h_pcnt_unit);

  ui_menu_reset_index();

  /** Set the UI src to the display */
  lcd_load_ui(h_scr_ui_menu);

  /** Activate control loops */
  h_control_menu_active = true;
}

/** Implementations */

static void update_encoder_steps(void)
{
  if (pulse_counter > 0) {
    for (int i = 0; i < pulse_counter; i += 2) {
      ui_menu_item_inc();
    }
  } else {
    for (int i = 0; i > pulse_counter; i -= 2) {
      ui_menu_item_dec();
    }
  }
}

static void control_menu_loop()
{
  /** Get changes on pulse counter */
  pcnt_unit_get_count(h_pcnt_unit, &pulse_counter);
  encoder_clear();

  update_encoder_steps();

  if (h_pending_button_enc) {
    h_pending_button_enc = false;

    ui_menu_action_t action = ui_menu_get_action();
    switch (action) {
      case UI_MENU_ACTION_STREAM:
        navigate_to_stream();
        break;
      case UI_MENU_ACTION_LIMITS:
        break;
      case UI_MENU_ACTION_EXIT:
        navigate_to_load();
        break;
      default:
        break;
    }
  }
}

static void navigate_to_load(void)
{
  /** Deactivate load loop to avoid conflicts */
  h_control_menu_active = false;

  /** Reset long press tracker */
  h_button_enc_long_press = false;
  h_button_enc_last = 0;

  button_enc_deactivate();

  /** Switch to menu screen */
  load_prepare();
}

static void navigate_to_stream(void)
{
  /** Deactivate load loop to avoid conflicts */
  h_control_menu_active = false;

  /** Reset long press tracker */
  h_button_enc_long_press = false;
  h_button_enc_last = 0;

  button_enc_deactivate();

  /** Switch to menu screen */
  stream_prepare();
}

static void check_screen_switch(void)
{
  /** This controls the flow between screens */
  if (h_button_enc_long_press) {
    navigate_to_load();
  }
}

static void menu_task(void *pvParameters)
{
  while (1)
  {
    if (h_control_menu_active)
    {
      /** For long press detection */
      button_enc_update();

      if (lvgl_mutex_lock(250)) {
        /** Check if need to switch to other screen */
        check_screen_switch();
        control_menu_loop();
        lvgl_mutex_unlock();
      }
    }
    vTaskDelay(pdMS_TO_TICKS(MENU_TASK_DELAY));
  }
}
