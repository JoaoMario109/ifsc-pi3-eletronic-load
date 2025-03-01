#ifndef __UI_INDEX_H__
#define __UI_INDEX_H__

#include "lvgl.h"

/** Handlers */

extern lv_obj_t *h_scr_ui_index;

extern lv_obj_t *h_led_enable;

extern lv_obj_t *h_value_spinbox;
extern lv_obj_t *h_value_mode_label;

extern lv_obj_t *h_value_current_spinbox;
extern lv_obj_t *h_value_voltage_spinbox;
extern lv_obj_t *h_value_resistance_spinbox;
extern lv_obj_t *h_value_power_spinbox;

/** Prototypes */

/**
 * @brief Main index UI of the load, inital window
 * @return void
 */
void ui_index_window();

void init_display_label_spinbox(
  lv_obj_t *scr,
  lv_obj_t **spinbox,
  const char *label,
  int8_t label_x,
  int8_t label_y,
  int8_t spinbox_x,
  int8_t spinbox_y
);

#endif // !__UI_INDEX_H__
