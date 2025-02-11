#include "ui/index.h"

/** Handlers */

lv_obj_t *h_led_enable;

lv_obj_t *h_value_spinbox;
lv_obj_t *h_value_mode_label;

lv_obj_t *h_value_current_spinbox;
lv_obj_t *h_value_voltage_spinbox;
lv_obj_t *h_value_resistance_spinbox;
lv_obj_t *h_value_power_spinbox;

/** Forward Decl */

static void init_enable_led(lv_obj_t *scr);

static void init_value_spinbox(lv_obj_t *scr);
static void init_value_mode_label(lv_obj_t *scr);

static void init_value_current_spinbox(lv_obj_t *scr);
static void init_value_voltage_spinbox(lv_obj_t *scr);
static void init_value_resistance_spinbox(lv_obj_t *scr);
static void init_value_power_spinbox(lv_obj_t *scr);

static void init_display_label_spinbox(
    lv_obj_t *scr,
    lv_obj_t *spinbox,
    const char *label,
    int8_t label_x,
    int8_t label_y,
    int8_t spinbox_x,
    int8_t spinbox_y);
static void lv_spinbox_show_cursor(lv_obj_t *spinbox, bool en);

/**
 * @brief Main index UI of the load, inital window
 *
 * @param disp Display handler
 * @return void
 */
void index_ui_window(lv_disp_t *disp)
{
  lv_obj_t *scr = lv_disp_get_scr_act(disp);

  init_enable_led(scr);

  init_value_spinbox(scr);
  init_value_mode_label(scr);

  init_value_current_spinbox(scr);
  init_value_voltage_spinbox(scr);
  init_value_resistance_spinbox(scr);
  init_value_power_spinbox(scr);
}

/** Implementations */

static void init_enable_led(lv_obj_t *scr)
{
  lv_obj_t *en_label = lv_label_create(scr);
  lv_label_set_text(en_label, "EN");
  lv_obj_align(en_label, LV_ALIGN_BOTTOM_LEFT, 5, -35);

  h_led_enable = lv_led_create(scr);
  lv_obj_set_size(h_led_enable, 20, 20);
  lv_obj_align(h_led_enable, LV_ALIGN_BOTTOM_LEFT, 5, -10);
  lv_led_set_brightness(h_led_enable, 255);
  lv_led_set_color(h_led_enable, lv_palette_main(LV_PALETTE_RED));
  lv_led_off(h_led_enable);
}

static void init_value_mode_label(lv_obj_t *scr)
{
  h_value_mode_label = lv_label_create(scr);
  lv_label_set_text(h_value_mode_label, "CC");
  lv_obj_align(h_value_mode_label, LV_ALIGN_TOP_LEFT, 15, 15);
}

static void init_value_spinbox(lv_obj_t *scr)
{
  h_value_spinbox = lv_spinbox_create(scr);
  lv_spinbox_set_range(h_value_spinbox, 0, 20000);
  lv_spinbox_set_digit_format(h_value_spinbox, 5, 2);
  lv_obj_set_width(h_value_spinbox, 111);
  lv_obj_align(h_value_spinbox, LV_ALIGN_TOP_RIGHT, -2, 5);
}

static void init_display_label_spinbox(
    lv_obj_t *scr,
    lv_obj_t *spinbox,
    const char *label,
    int8_t label_x,
    int8_t label_y,
    int8_t spinbox_x,
    int8_t spinbox_y)
{
  lv_obj_t *current_label = lv_label_create(scr);
  lv_label_set_text(current_label, label);
  lv_obj_align(current_label, LV_ALIGN_RIGHT_MID, label_x, label_y);

  spinbox = lv_spinbox_create(scr);

  lv_spinbox_set_range(spinbox, 0, 20000);
  lv_spinbox_set_digit_format(spinbox, 3, 2);
  lv_obj_set_width(spinbox, 48);
  lv_obj_align(spinbox, LV_ALIGN_RIGHT_MID, spinbox_x, spinbox_y);
  lv_spinbox_show_cursor(spinbox, false);
}

static void init_value_current_spinbox(lv_obj_t *scr)
{
  init_display_label_spinbox(scr, h_value_current_spinbox, "C", -53, 2, -2, 2);
}

static void init_value_voltage_spinbox(lv_obj_t *scr)
{
  init_display_label_spinbox(scr, h_value_voltage_spinbox, "V", -116, 2, -65, 2);
}

static void init_value_resistance_spinbox(lv_obj_t *scr)
{
  init_display_label_spinbox(scr, h_value_voltage_spinbox, "R", -53, 42, -2, 42);
}

static void init_value_power_spinbox(lv_obj_t *scr)
{
  init_display_label_spinbox(scr, h_value_voltage_spinbox, "P", -116, 42, -65, 42);
}

void lv_spinbox_show_cursor(lv_obj_t *spinbox, bool en)
{
  lv_obj_set_style_text_color(spinbox, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_text_color(spinbox, (en) ? lv_color_white() : lv_color_black(), LV_PART_CURSOR);
  lv_obj_set_style_bg_opa(spinbox, (en) ? 255 : 0, LV_PART_CURSOR);
}
