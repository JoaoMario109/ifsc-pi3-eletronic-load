#include "common.h"
#include "peripherals/buttons.h"

#include "driver/gpio.h"
#include "esp_log.h"

/** Handlers */
bool h_button_cc = false;
bool h_button_cv = false;
bool h_button_cr = false;
bool h_button_cp = false;
bool h_button_en = false;
bool h_button_enc = false;

bool h_button_cc_last = false;
bool h_button_cv_last = false;
bool h_button_cr_last = false;
bool h_button_cp_last = false;
bool h_button_en_last = false;
bool h_button_enc_last = false;

bool h_button_cc_rising = false;
bool h_button_cv_rising = false;
bool h_button_cr_rising = false;
bool h_button_cp_rising = false;
bool h_button_en_rising = false;
bool h_button_enc_rising = false;

bool h_button_cc_falling = false;
bool h_button_cv_falling = false;
bool h_button_cr_falling = false;
bool h_button_cp_falling = false;
bool h_button_en_falling = false;
bool h_button_enc_falling = false;

/** Forward Decl */
static void buttons_direction_init(void);

/**
 * @brief Set up default buttons configuration
 * @return void
 */
void buttons_init(void)
{
    buttons_direction_init();
    update_buttons();
}

/**
 * @brief Update state of all button handlers
 * @return void
 */
void update_buttons(void)
{
    h_button_cc = gpio_get_level(GPIO_BTN_CC);
    h_button_cv = gpio_get_level(GPIO_BTN_CV);
    h_button_cr = gpio_get_level(GPIO_BTN_CR);
    h_button_cp = gpio_get_level(GPIO_BTN_CP);
    h_button_en = gpio_get_level(GPIO_BTN_EN);
    h_button_enc = gpio_get_level(GPIO_ENCODER_BTN);

    h_button_cc_rising = h_button_cc && !h_button_cc_last;
    h_button_cv_rising = h_button_cv && !h_button_cv_last;
    h_button_cr_rising = h_button_cr && !h_button_cr_last;
    h_button_cp_rising = h_button_cp && !h_button_cp_last;
    h_button_en_rising = h_button_en && !h_button_en_last;
    h_button_enc_rising = h_button_enc && !h_button_enc_last;

    h_button_cc_falling = !h_button_cc && h_button_cc_last;
    h_button_cv_falling = !h_button_cv && h_button_cv_last;
    h_button_cr_falling = !h_button_cr && h_button_cr_last;
    h_button_cp_falling = !h_button_cp && h_button_cp_last;
    h_button_en_falling = !h_button_en && h_button_en_last;
    h_button_enc_falling = !h_button_enc && h_button_enc_last;

    h_button_cc_last = h_button_cc;
    h_button_cv_last = h_button_cv;
    h_button_cr_last = h_button_cr;
    h_button_cp_last = h_button_cp;
    h_button_en_last = h_button_en;
    h_button_enc_last = h_button_enc;
}

/** Implementations */
static void buttons_direction_init(void)
{
    ESP_ERROR_CHECK(gpio_set_direction(GPIO_BTN_CC, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(gpio_set_direction(GPIO_BTN_CV, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(gpio_set_direction(GPIO_BTN_CR, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(gpio_set_direction(GPIO_BTN_CP, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(gpio_set_direction(GPIO_BTN_EN, GPIO_MODE_INPUT));

    ESP_ERROR_CHECK(gpio_set_direction(GPIO_ENCODER_BTN, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(gpio_set_pull_mode(GPIO_ENCODER_BTN, GPIO_PULLUP_ONLY));
}
