#ifndef __PERIPHERALS_LCD_H__
#define __PERIPHERALS_LCD_H__

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "lvgl.h"

/** General Config */

#define LCD_PIXEL_CLOCK_HZ (40 * 1000 * 1000)
#define LCD_CMD_BITS 8
#define LCD_PARAM_BITS 8

#define LCD_H_RES 160
#define LCD_V_RES 128

#define LVGL_TICK_PERIOD_MS 100
#define LVGL_TASK_MAX_DELAY_MS 300
#define LVGL_TASK_MIN_DELAY_MS 50
#define LVGL_TASK_STACK_SIZE (25 * 1024)
#define LVGL_TASK_PRIORITY 1

/** Typedefs */
typedef void (*ui_fn_t)(lv_disp_t *disp);

/** Handlers */
extern lv_disp_draw_buf_t h_disp_buf;
extern lv_disp_drv_t h_disp_drv;
extern lv_disp_t *h_disp;
extern esp_lcd_panel_io_handle_t h_io_handle;
extern esp_lcd_panel_handle_t h_panel_handle;

/** Prototypes */

/**
 * @brief Set up the LCD basic configuration and LVGL configuration
 * @return void
 */
void lcd_init(void);

/**
 * @brief Load a UI screen to the LCD
 * @return void
 */
void lcd_load_ui(lv_obj_t *ui_screen);

/**
 * @brief Locks the LVGL mutex due to the LVGL APIs are not thread-safe
 * @param timeout_ms Timeout in milliseconds to wait for the mutex, if -1, wait indefinitely
 * @return bool
 */
bool lvgl_mutex_lock(int timeout_ms);

/**
 * @brief Unlocks the LVGL mutex
 * @return void
 */
void lvgl_mutex_unlock(void);

#endif /** !__PERIPHERALS_LCD_H__ */
