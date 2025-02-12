#ifndef __UI_MENU_H__
#define __UI_MENU_H__

#include "lvgl.h"

/** Enum */
typedef enum ui_menu_action
{
  UI_MENU_ACTION_STREAM = 0U,
  UI_MENU_ACTION_LIMITS,
  UI_MENU_ACTION_EXIT,
  UI_MENU_ACTION_NONE,
} ui_menu_action_t;

/** Handlers */

extern lv_obj_t *h_scr_ui_menu;

/** Prototypes */

/**
 * @brief Creates the menu window
 * @return void
 */
void ui_menu_window();

/**
 * @brief Reset the menu index
 * @return void
 */
void ui_menu_reset_index();

/**
 * @brief Increment the selected item
 * @return void
 */
void ui_menu_item_inc();

/**
 * @brief Decrement the selected item
 * @return void
 */
void ui_menu_item_dec();

/**
 * @brief Get the selected action
 * @return ui_menu_action_t
 */
ui_menu_action_t ui_menu_get_action();

#endif /** !__UI_MENU_H__ */
