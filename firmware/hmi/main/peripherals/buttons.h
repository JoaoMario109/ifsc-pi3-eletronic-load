#ifndef __PERIPHERALS_BUTTONS_H__
#define __PERIPHERALS_BUTTONS_H__

#include <stdbool.h>

/** Handlers */
extern volatile bool h_pending_button_cv;
extern volatile bool h_pending_button_cr;
extern volatile bool h_pending_button_cp;
extern volatile bool h_pending_button_en;
extern volatile bool h_pending_button_enc;

/** Prototypes */

/**
 * @brief Set up default buttons configuration
 * @return void
 */
void buttons_init(void);

/**
 * @brief Enable buttons interrupts allowing to detect button press
 * @return void
 */
void buttons_activate(void);

/**
 * @brief Enable buttons interrupts allowing to detect button press
 * @return void
 */
void buttons_deactivate(void);

#endif /** !__PERIPHERALS_BUTTONS_H__ */
