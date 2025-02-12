#include "ui/stream.h"

#include "ui/index.h"

/** Handlers */

lv_obj_t *h_scr_ui_stream;

lv_obj_t *h_stream_led_enable;
lv_obj_t *h_stream_mode_label;

lv_obj_t *h_stream_desired_spinbox;
lv_obj_t *h_stream_measured_spinbox;

lv_obj_t *h_stream_msg_box;

lv_obj_t *h_stream_chart;
lv_chart_series_t *h_stream_series_current;
lv_chart_series_t *h_stream_series_voltage;

/** Globals */

/** Prototypes */
static void ui_stream_window_en_led(lv_obj_t *scr);
static void ui_stream_window_mode_label(lv_obj_t *scr);
static void ui_stream_window_desired(lv_obj_t *scr);
static void ui_stream_window_measured(lv_obj_t *scr);
static void ui_stream_window_chart(lv_obj_t *scr);
static void ui_stream_window_msg_box(lv_obj_t *scr);

/**
 * @brief Stream UI of the load, inital window
 * @return void
 */
void ui_stream_window()
{
  h_scr_ui_stream = lv_obj_create(NULL);

  ui_stream_window_en_led(h_scr_ui_stream);
  ui_stream_window_mode_label(h_scr_ui_stream);
  ui_stream_window_desired(h_scr_ui_stream);
  ui_stream_window_measured(h_scr_ui_stream);
  ui_stream_window_chart(h_scr_ui_stream);
  ui_stream_window_msg_box(h_scr_ui_stream);
}

void ui_stream_window_open_msg()
{
  lv_obj_clear_flag(h_stream_msg_box, LV_OBJ_FLAG_HIDDEN);
}

void ui_stream_window_close_msg()
{
  lv_obj_add_flag(h_stream_msg_box, LV_OBJ_FLAG_HIDDEN);
}

void add_chart_point(uint32_t current, uint32_t voltage)
{
  lv_chart_set_next_value(h_stream_chart, h_stream_series_current, current);
  lv_chart_set_next_value(h_stream_chart, h_stream_series_voltage, voltage);

  lv_chart_refresh(h_stream_chart);
}

/** Implementation */

static void ui_stream_window_en_led(lv_obj_t *scr)
{
  lv_obj_t *en_label = lv_label_create(scr);
  lv_label_set_text(en_label, "EN");
  lv_obj_align(en_label, LV_ALIGN_BOTTOM_LEFT, 5, -35);

  h_stream_led_enable = lv_led_create(scr);
  lv_obj_set_size(h_stream_led_enable, 20, 20);
  lv_obj_align(h_stream_led_enable, LV_ALIGN_BOTTOM_LEFT, 5, -10);
  lv_led_set_brightness(h_stream_led_enable, 255);
  lv_led_set_color(h_stream_led_enable, lv_palette_main(LV_PALETTE_RED));
  lv_led_off(h_stream_led_enable);
}

static void ui_stream_window_mode_label(lv_obj_t *scr)
{
  h_stream_mode_label = lv_label_create(scr);
  lv_label_set_text(h_stream_mode_label, "CC");
  lv_obj_align(h_stream_mode_label, LV_ALIGN_TOP_LEFT, 5, 15);
}

static void ui_stream_window_desired(lv_obj_t *scr)
{
  init_display_label_spinbox(scr, &h_stream_desired_spinbox, "S", -118, 42, -66, 42);
}

static void ui_stream_window_measured(lv_obj_t *scr)
{
  init_display_label_spinbox(scr, &h_stream_measured_spinbox, "M", -52, 42, -2, 42);
}

static void ui_stream_window_chart(lv_obj_t *scr)
{
  h_stream_chart = lv_chart_create(scr);
  lv_chart_set_update_mode(h_stream_chart, LV_CHART_UPDATE_MODE_CIRCULAR);
  lv_obj_set_style_size(h_stream_chart, 0, LV_PART_INDICATOR);
  lv_chart_set_range(h_stream_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 3000);
  lv_chart_set_range(h_stream_chart, LV_CHART_AXIS_SECONDARY_Y, 0, 30000);
  lv_obj_set_size(h_stream_chart, 130, 85);
  lv_obj_align(h_stream_chart, LV_ALIGN_TOP_RIGHT, -2, 2);

  lv_chart_set_point_count(h_stream_chart, STREAM_CHART_POINTS);
  h_stream_series_current = lv_chart_add_series(h_stream_chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
  h_stream_series_voltage = lv_chart_add_series(h_stream_chart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_SECONDARY_Y);
  for(uint32_t i = 0; i < STREAM_CHART_POINTS; i++) {
    lv_chart_set_next_value(h_stream_chart, h_stream_series_current, 0);
    lv_chart_set_next_value(h_stream_chart, h_stream_series_voltage, 0);
  }
  lv_chart_refresh(h_stream_chart);
}

static void ui_stream_window_msg_box(lv_obj_t *scr)
{
  h_stream_msg_box = lv_msgbox_create(scr, "Streaming", "EN to close / Wait 5s to continue", NULL, false);
  lv_obj_add_flag(h_stream_msg_box, LV_OBJ_FLAG_HIDDEN);
  lv_obj_center(h_stream_msg_box);
}
