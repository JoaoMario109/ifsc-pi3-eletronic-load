#include "ui/menu.h"

/** Handlers */

lv_obj_t *h_scr_ui_menu;

lv_obj_t *h_menu_list;
lv_obj_t *h_selected_item = NULL;

/** Globals */
uint8_t btn_counter = 0U;

/**
 * @brief Creates the menu window
 * @return void
 */
void ui_menu_window()
{
  h_scr_ui_menu = lv_obj_create(NULL);

  h_menu_list = lv_list_create(h_scr_ui_menu);
  lv_obj_set_size(h_menu_list, 128, 120);
  lv_obj_center(h_menu_list);

  /*Add buttons to the list*/
  lv_list_add_btn(h_menu_list, LV_SYMBOL_FILE, "Stream");
  btn_counter++;
  lv_list_add_btn(h_menu_list, LV_SYMBOL_SETTINGS, "Limits");
  btn_counter++;
  lv_list_add_btn(h_menu_list, LV_SYMBOL_CLOSE, "Exit");
  btn_counter++;
}

/**
 * @brief Reset the menu index
 * @return void
 */
void ui_menu_reset_index()
{
  if (h_selected_item == NULL) {
    return;
  }

  lv_obj_clear_state(h_selected_item, LV_STATE_PRESSED);
  h_selected_item = NULL;
}

/**
 * @brief Increment the selected item
 * @return void
 */
void ui_menu_item_inc()
{
  if(h_selected_item == NULL) {
    h_selected_item = lv_obj_get_child(h_menu_list, 0);
    lv_obj_add_state(h_selected_item, LV_STATE_PRESSED);
    return;
  }

  uint32_t index = lv_obj_get_index(h_selected_item);

  if(index < btn_counter - 1U) {
    lv_obj_clear_state(h_selected_item, LV_STATE_PRESSED);
    h_selected_item = lv_obj_get_child(h_menu_list, index + 1);
    lv_obj_add_state(h_selected_item, LV_STATE_PRESSED);
  }
}

/**
 * @brief Decrement the selected item
 * @return void
 */
void ui_menu_item_dec()
{
  if(h_selected_item == NULL) {
    h_selected_item = lv_obj_get_child(h_menu_list, 0);
    lv_obj_add_state(h_selected_item, LV_STATE_PRESSED);
    return;
  }
  const uint32_t index = lv_obj_get_index(h_selected_item);

  if (index == 0) return;

  lv_obj_clear_state(h_selected_item, LV_STATE_PRESSED);
  h_selected_item = lv_obj_get_child(h_menu_list, index - 1);
  lv_obj_add_state(h_selected_item, LV_STATE_PRESSED);
}

/**
 * @brief Get the selected action
 * @return ui_menu_action_t
 */
ui_menu_action_t ui_menu_get_action()
{
  if(h_selected_item == NULL) {
    return UI_MENU_ACTION_NONE;
  }

  const uint32_t index = lv_obj_get_index(h_selected_item);

  switch(index) {
    case 0:
      return UI_MENU_ACTION_STREAM;
    case 1:
      return UI_MENU_ACTION_LIMITS;
    case 2:
      return UI_MENU_ACTION_EXIT;
    default:
      return UI_MENU_ACTION_EXIT;
  }
}
