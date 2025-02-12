#ifndef __CONTROL_MENU_H__
#define __CONTROL_MENU_H__

/** General Config */
#define MENU_TASK_STACK_SIZE 5096U
#define MENU_TASK_DELAY 50U

/** Handlers */

/** Prototypes */

/**
 * @brief Initialize menu control module
 * @return void
 */
void menu_init(void);

/**
 * @brief Generic preparations for menu control used when resuming from other screen
 * @return void
 */
void menu_prepare(void);

#endif /** !__CONTROL_MENU_H__ */
