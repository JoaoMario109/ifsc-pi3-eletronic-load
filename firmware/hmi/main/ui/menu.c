#include "ui/menu.h"

/** Handlers */

lv_obj_t *h_scr_ui_menu;

/** Forward Decl */

/**
 * @brief Creates the menu window
 * @return void
 */
void ui_menu_window()
{
  h_scr_ui_menu = lv_obj_create(NULL);
}

/** Implementations */
