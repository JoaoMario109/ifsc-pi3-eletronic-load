#ifndef __PERIPHERALS_BUTTONS_H__
#define __PERIPHERALS_BUTTONS_H__

#include <stdbool.h>

/** General config */
#define BTN_ENC_LONG_PRESS_MS 1500U

/** Handlers */
extern volatile bool h_pending_button_cc;
extern volatile bool h_pending_button_cv;
extern volatile bool h_pending_button_cr;
extern volatile bool h_pending_button_cp;
extern volatile bool h_pending_button_enc;

/** Enable is trick because of not being able to deal ok with IRQ */
extern bool h_button_en;
extern bool h_button_en_last;
extern bool h_button_en_rising;
extern bool h_button_en_falling;

/** We keep a ref for long press */
extern bool h_button_enc;
extern unsigned long h_button_enc_last;
extern bool h_button_enc_long_press;

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
 * @brief Enable encoder button interrupt
 * @return void
 */
void button_enc_activate(void);

/**
 * @brief Disable encoder button interrupt
 * @return void
 */
void button_enc_deactivate(void);

/**
 * @brief Clear all pending IRQ
 * @return void
 */
void clear_all_pending_irq(void);

/**
 * @brief Update enable buttons state
 * @return void
 */
void button_en_update();

/**
 * @brief Update encoder button state
 * @return void
 */
void button_enc_update();

#endif /** !__PERIPHERALS_BUTTONS_H__ */
