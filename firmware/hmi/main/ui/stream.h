#ifndef __UI_STREAM_H__
#define __UI_STREAM_H__

#include "lvgl.h"

/** General Config */

/** Chart is 5 min */
#define STREAM_CHART_INTERVAL_S 1
/** 300 points */
#define STREAM_CHART_POINTS 300

/** Handlers */

extern lv_obj_t *h_scr_ui_stream;

extern lv_obj_t *h_stream_led_enable;
extern lv_obj_t *h_stream_mode_label;

extern lv_obj_t *h_stream_desired_spinbox;
extern lv_obj_t *h_stream_measured_spinbox;

extern lv_obj_t *h_stream_chart;
extern lv_chart_series_t *h_stream_series_current;
extern lv_chart_series_t *h_stream_series_voltage;

/** Prototypes */

/**
 * @brief Stream UI of the load, inital window
 * @return void
 */
void ui_stream_window();

void ui_stream_window_open_msg();

void ui_stream_window_close_msg();

void add_chart_point(uint32_t current, uint32_t voltage);

#endif /** !__UI_STREAM_H__ */
