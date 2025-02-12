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
#include "esp_timer.h"

/** Definitions */

#define MODULE_NAME "control.stream"

/** Handlers */
bool h_control_stream_active = false;

/** Globals */
bool msg_opened = false;
unsigned long opened_msg_time = 0;

load_mode_t h_target_mode = NONE;
uint32_t h_target_point = 0;
uint32_t h_target_delay = 0;

uint32_t h_stream_mode = 0;

uint32_t h_target_chart_point = 0;

bool h_stream_complete = false;
FILE *h_stream_file = NULL;

/** Prototypes */
static void control_stream_loop();
static void check_screen_switch(void);
static void navigate_to_load(void);
static void stream_task(void *pvParameters);
static void open_stream_file();
static void close_stream_file();
static void update_enabled_status(void);

/**
 * @brief Initialize menu control module
 * @return void
 */
void stream_init(void)
{
  LOG_PROLOG

  xTaskCreate(stream_task, "stream", STREAM_TASK_STACK_SIZE, NULL, 1, NULL);

  LOG_EPILOG
}

/**
 * @brief Generic preparations for menu control used when resuming from other screen
 * @return void
 */
void stream_prepare(void)
{
  /** Enable only encoder */
  buttons_deactivate();

  /** Clear all pending IRQ and pulse counter unit */
  clear_all_pending_irq();
  pcnt_unit_clear_count(h_pcnt_unit);

  /** Set the UI src to the display */
  lcd_load_ui(h_scr_ui_stream);

  spi_mutex_lock(-1);
  sd_mount();
  open_stream_file();
  spi_mutex_unlock();

  if (h_stream_file == NULL) {
    navigate_to_load();
    return;
  }

  /** Disables enable task */
  h_control_enable_active = false;

  /** Activate control loops */
  h_control_stream_active = true;
}

/** Implementations */

static void navigate_to_load(void)
{
  /** Deactivate load loop to avoid conflicts */
  h_control_stream_active = false;

  /** Reset long press tracker */
  h_button_enc_long_press = false;
  h_button_enc_last = 0;

  /** Reactivate enable tracking */
  h_control_enable_active = true;

  /** Reset message tracker */
  msg_opened = false;
  opened_msg_time = 0;

  spi_mutex_lock(-1);
  close_stream_file();
  sd_unmount();
  spi_mutex_unlock();

  lvgl_mutex_lock(-1);
  for (uint16_t i = 0; i < STREAM_CHART_POINTS; i++)
  {
    lv_chart_set_next_value(h_stream_chart, h_stream_series_current, 0);
    lv_chart_set_next_value(h_stream_chart, h_stream_series_voltage, 0);
  }
  lvgl_mutex_unlock();

  /** Switch to menu screen */
  load_prepare();
}

static void open_stream_file()
{
  h_stream_complete = false;
  h_target_mode = NONE;
  h_target_point = 0;
  h_target_delay = 0;

  h_stream_file = fopen(STREAM_DATA_FILE, "r");
}

static void close_stream_file()
{
  if (h_stream_file == NULL) {
    return;
  }

  fclose(h_stream_file);
  h_stream_file = NULL;
}

static void update_enabled_status(void)
{
  if (h_load_state.control.enable) {
    lv_led_on(h_stream_led_enable);
  } else {
    lv_led_off(h_stream_led_enable);
  }
  set_led_enable(h_load_state.control.enable);
}

static void parse_line()
{
  /** Two letter plus null terminator */
  char code[3];
  /** Storage for point and delay */
  uint32_t point, delay;

  if (h_stream_file == NULL)
    return;

  if (fscanf(h_stream_file, "%2s,%lu,%lu", code, &point, &delay) == 3)
  {
    if (strcmp(code, "CC") == 0)
    {
      h_load_state.control.mode = CC;
      h_load_state.control.cc.value_milli = point;
      lv_label_set_text(h_stream_mode_label, "CC");
    }
    else if (strcmp(code, "CV") == 0)
    {
      h_load_state.control.mode = CV;
      h_load_state.control.cv.value_milli = point;
      lv_label_set_text(h_stream_mode_label, "CV");
    }
    else if (strcmp(code, "CP") == 0)
    {
      h_load_state.control.mode = CP;
      h_load_state.control.cp.value_milli = point;
      lv_label_set_text(h_stream_mode_label, "CP");
    }
    else if (strcmp(code, "CR") == 0)
    {
      h_load_state.control.mode = CR;
      h_load_state.control.cr.value_milli = point;
      lv_label_set_text(h_stream_mode_label, "CR");
    }

    if (strcmp(code, "EN") == 0)
    {
      h_load_state.control.enable = point;
      update_enabled_status();
    } else {
      lv_spinbox_set_value(
        h_stream_desired_spinbox, point / 100U
      );
    }

    h_target_delay = (unsigned long)(esp_timer_get_time() / 1000ULL) + delay;
  }
  else
  {
    navigate_to_load();
  }
}

static void control_stream_loop()
{
  const unsigned long current_time = (unsigned long)(esp_timer_get_time() / 1000ULL);

  lvgl_mutex_lock(-1);

  if (current_time >= h_target_delay)
  {
    spi_mutex_lock(-1);
    parse_line();
    spi_mutex_unlock();
  }

  switch (h_load_state.control.mode)
  {
    case CC:
      lv_spinbox_set_value(
        h_stream_measured_spinbox, h_load_state.measurement.cc_milli / 100U
      );
      break;
    case CV:
      lv_spinbox_set_value(
        h_stream_measured_spinbox, h_load_state.measurement.cv_milli / 100U
      );
      break;
    case CR:
      lv_spinbox_set_value(
        h_stream_measured_spinbox, h_load_state.measurement.cr_milli / 100U
      );
      break;
    case CP:
      lv_spinbox_set_value(
        h_stream_measured_spinbox, h_load_state.measurement.cp_milli / 100U
      );
      break;
    default:
      break;
  }

  if (current_time >= h_target_chart_point)
  {
    add_chart_point(
      h_load_state.measurement.cc_milli,
      h_load_state.measurement.cv_milli
    );

    h_target_chart_point = current_time + STREAM_CHART_INTERVAL_S * 1000U;
  }

  lvgl_mutex_unlock();
}

static void check_screen_switch(void)
{
  /** This controls the flow between screens */
  if (!msg_opened && h_button_en_rising) {
    msg_opened = true;
    opened_msg_time = (unsigned long)(esp_timer_get_time() / 1000ULL);
    h_button_en_rising = false;
    ui_stream_window_open_msg();
  }

  if (msg_opened) {
    if (h_button_en_rising) {
      msg_opened = false;
      ui_stream_window_close_msg();
      navigate_to_load();
    }

    if (((unsigned long)(esp_timer_get_time() / 1000ULL) - opened_msg_time) > MSG_OPEN_TIME) {
      msg_opened = false;
      opened_msg_time = 0;
      ui_stream_window_close_msg();
    }
  }
}

static void stream_task(void *pvParameters)
{
  while (1)
  {
    if (h_control_stream_active)
    {
      button_en_update();

      if (lvgl_mutex_lock(250)) {
        /** Check if need to switch to other screen */
        check_screen_switch();
        control_stream_loop();
        lvgl_mutex_unlock();
      }
    }
    vTaskDelay(pdMS_TO_TICKS(STREAM_TASK_DELAY));
  }
}
