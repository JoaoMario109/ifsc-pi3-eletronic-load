#include "driver/gpio.h"
#include "esp_attr.h"

#include "common.h"
#include "utils.h"
#include "peripherals/buttons.h"

/** Definitions */

#define MODULE_NAME "peripherals.buttons"

/** Handlers */
volatile bool h_pending_button_cv = false;
volatile bool h_pending_button_cr = false;
volatile bool h_pending_button_cp = false;
volatile bool h_pending_button_en = false;
volatile bool h_pending_button_enc = false;

/** Forward Decl */
static void buttons_direction_init(void);
static void buttons_irq_init(void);
static void gpio_isr_handler(void *arg);

/**
 * @brief Set up default buttons configuration
 * @return void
 */
void buttons_init(void)
{
  LOG_PROLOG

  buttons_direction_init();
  buttons_irq_init();

  LOG_EPILOG
}

/**
 * @brief Enable buttons interrupts allowing to detect button press
 * @return void
 */
void buttons_activate(void)
{
  LOG_PROLOG

  gpio_intr_enable(GPIO_BTN_CC);
  gpio_intr_enable(GPIO_BTN_CV);
  gpio_intr_enable(GPIO_BTN_CR);
  gpio_intr_enable(GPIO_BTN_CP);
  gpio_intr_enable(GPIO_BTN_EN);
  gpio_intr_enable(GPIO_ENCODER_BTN);

  LOG_EPILOG
}

/**
 * @brief Disable buttons interrupts
 * @return void
 */
void buttons_deactivate(void)
{
  LOG_PROLOG

  gpio_intr_disable(GPIO_BTN_CC);
  gpio_intr_disable(GPIO_BTN_CV);
  gpio_intr_disable(GPIO_BTN_CR);
  gpio_intr_disable(GPIO_BTN_CP);
  gpio_intr_disable(GPIO_BTN_EN);
  gpio_intr_disable(GPIO_ENCODER_BTN);

  LOG_EPILOG
}

/** Implementations */
static void buttons_direction_init(void)
{
  LOG_PROLOG

  ESP_ERROR_CHECK(gpio_set_direction(GPIO_BTN_CC, GPIO_MODE_INPUT));

  ESP_ERROR_CHECK(gpio_set_direction(GPIO_BTN_CC, GPIO_MODE_INPUT));
  ESP_ERROR_CHECK(gpio_set_direction(GPIO_BTN_CV, GPIO_MODE_INPUT));
  ESP_ERROR_CHECK(gpio_set_direction(GPIO_BTN_CR, GPIO_MODE_INPUT));
  ESP_ERROR_CHECK(gpio_set_direction(GPIO_BTN_CP, GPIO_MODE_INPUT));
  ESP_ERROR_CHECK(gpio_set_direction(GPIO_BTN_EN, GPIO_MODE_INPUT));

  ESP_ERROR_CHECK(gpio_set_direction(GPIO_ENCODER_BTN, GPIO_MODE_INPUT));
  ESP_ERROR_CHECK(gpio_set_pull_mode(GPIO_ENCODER_BTN, GPIO_PULLUP_ONLY));

  LOG_EPILOG
}

static void buttons_irq_init(void)
{
  LOG_PROLOG

  ESP_ERROR_CHECK(gpio_install_isr_service(0));

  ESP_ERROR_CHECK(gpio_set_intr_type(GPIO_BTN_CC, GPIO_INTR_POSEDGE));
  ESP_ERROR_CHECK(gpio_isr_handler_add(GPIO_BTN_CC, gpio_isr_handler, (void*)GPIO_BTN_CC));

  ESP_ERROR_CHECK(gpio_set_intr_type(GPIO_BTN_CV, GPIO_INTR_POSEDGE));
  ESP_ERROR_CHECK(gpio_isr_handler_add(GPIO_BTN_CV, gpio_isr_handler, (void*)GPIO_BTN_CV));

  ESP_ERROR_CHECK(gpio_set_intr_type(GPIO_BTN_CR, GPIO_INTR_POSEDGE));
  ESP_ERROR_CHECK(gpio_isr_handler_add(GPIO_BTN_CR, gpio_isr_handler, (void*)GPIO_BTN_CR));

  ESP_ERROR_CHECK(gpio_set_intr_type(GPIO_BTN_CP, GPIO_INTR_POSEDGE));
  ESP_ERROR_CHECK(gpio_isr_handler_add(GPIO_BTN_CP, gpio_isr_handler, (void*)GPIO_BTN_CP));

  ESP_ERROR_CHECK(gpio_set_intr_type(GPIO_BTN_EN, GPIO_INTR_POSEDGE));
  ESP_ERROR_CHECK(gpio_isr_handler_add(GPIO_BTN_EN, gpio_isr_handler, (void*)GPIO_BTN_EN));

  ESP_ERROR_CHECK(gpio_set_intr_type(GPIO_ENCODER_BTN, GPIO_INTR_POSEDGE));
  ESP_ERROR_CHECK(gpio_isr_handler_add(GPIO_ENCODER_BTN, gpio_isr_handler, (void*)GPIO_ENCODER_BTN));

  LOG_EPILOG
}

/** Interrupt */
static void IRAM_ATTR gpio_isr_handler(void *arg)
{
  uint32_t gpio_num = (uint32_t)arg;

  switch (gpio_num)
  {
    case GPIO_BTN_CV:
      h_pending_button_cv = true;
      break;
    case GPIO_BTN_CR:
      h_pending_button_cr = true;
      break;
    case GPIO_BTN_CP:
      h_pending_button_cp = true;
      break;
    case GPIO_BTN_EN:
      h_pending_button_en = true;
      break;
    case GPIO_ENCODER_BTN:
      h_pending_button_enc = true;
      break;
    default:
      break;
  }
}
