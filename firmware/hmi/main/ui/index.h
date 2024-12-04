#ifndef __UI_INDEX_H__
#define __UI_INDEX_H__

#include "lvgl.h"

/** Handlers */
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
 *
 * @param disp Display handler
 * @return void
 */
void index_ui_window(lv_disp_t *disp);

#endif // !__UI_INDEX_H__
