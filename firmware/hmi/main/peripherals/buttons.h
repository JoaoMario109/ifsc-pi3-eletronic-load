#ifndef __PERIPHERALS_BUTTONS_H__
#define __PERIPHERALS_BUTTONS_H__

#include <stdbool.h>

/** Handlers */
extern volatile bool h_pending_button_cc;
extern volatile bool h_pending_button_cv;
extern volatile bool h_pending_button_cr;
extern volatile bool h_pending_button_cp;
extern volatile bool h_pending_button_enc;

/** Enable is trick because of not being able to deal ok with IRQ */
extern volatile bool h_button_en;
extern volatile bool h_button_en_last;
extern volatile bool h_button_en_rising;
extern volatile bool h_button_en_falling;

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

/**
 * @brief Update enable buttons state
 * @return void
 */
void button_en_update();

#endif /** !__PERIPHERALS_BUTTONS_H__ */
