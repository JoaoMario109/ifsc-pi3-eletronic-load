#ifndef __PERIPHERALS_BUTTONS_H__
#define __PERIPHERALS_BUTTONS_H__

#include <stdbool.h>

/** Handlers */
extern bool h_button_cc;
extern bool h_button_cv;
extern bool h_button_cr;
extern bool h_button_cp;
extern bool h_button_en;
extern bool h_button_enc;

extern bool h_button_cc_last;
extern bool h_button_cv_last;
extern bool h_button_cr_last;
extern bool h_button_cp_last;
extern bool h_button_en_last;
extern bool h_button_enc_last;

extern bool h_button_cc_rising;
extern bool h_button_cv_rising;
extern bool h_button_cr_rising;
extern bool h_button_cp_rising;
extern bool h_button_en_rising;
extern bool h_button_enc_rising;

extern bool h_button_cc_falling;
extern bool h_button_cv_falling;
extern bool h_button_cr_falling;
extern bool h_button_cp_falling;
extern bool h_button_en_falling;
extern bool h_button_enc_falling;

/** Prototypes */

/**
 * @brief Set up default buttons configuration
 * @return void
 */
void buttons_init(void);

/**
 * @brief Update state of all button handlers
 * @return void
 */
void update_buttons(void);

#endif // !__PERIPHERALS_BUTTONS_H__
