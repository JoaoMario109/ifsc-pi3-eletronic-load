#include "common.h"
#include "peripherals/encoder.h"

#include "driver/gpio.h"
#include "esp_log.h"

/** Handlers */
pcnt_unit_handle_t h_pcnt_unit;
pcnt_channel_handle_t h_pcnt_chan_a;
pcnt_channel_handle_t h_pcnt_chan_b;

/** Forward Decl */
static void encoder_init_counter(void);
static void encoder_init_channels(void);
static void encoder_init_actions(void);

/**
 * @brief Set up the encoder pulse counter unit
 * @return void
 */
void encoder_init(void)
{
    encoder_init_counter();
    encoder_init_channels();
    encoder_init_actions();
}

/**
 * @brief Clear and start pulse counter unit
 * @return void
 */
void encoder_start(void)
{
    ESP_ERROR_CHECK(pcnt_unit_enable(h_pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(h_pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_start(h_pcnt_unit));
}

/**
 * @brief Stop pulse counter unit
 * @return void
 */
void encoder_stop(void)
{
    ESP_ERROR_CHECK(pcnt_unit_stop(h_pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_disable(h_pcnt_unit));
}

/**
 * @brief Get the current encoder count
 * @return void
 */
void encoder_clear(void)
{
    ESP_ERROR_CHECK(pcnt_unit_clear_count(h_pcnt_unit));
}

/** Implementations */

static void encoder_init_counter(void)
{
    pcnt_unit_config_t unit_config = {
        .high_limit = ENCODER_COUNTER_MAX,
        .low_limit = ENCODER_COUNTER_MIN,
    };
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &h_pcnt_unit));

    pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = ENCODER_GLITCH_FILTER_NS,
    };
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(h_pcnt_unit, &filter_config));
}

static void encoder_init_channels(void)
{
    pcnt_chan_config_t chan_a_config = {
        .edge_gpio_num = GPIO_ENCODER_CK,
        .level_gpio_num = GPIO_ENCODER_DT,
    };
    ESP_ERROR_CHECK(pcnt_new_channel(h_pcnt_unit, &chan_a_config, &h_pcnt_chan_a));

    pcnt_chan_config_t chan_b_config = {
        .edge_gpio_num = GPIO_ENCODER_DT,
        .level_gpio_num = GPIO_ENCODER_CK,
    };
    ESP_ERROR_CHECK(pcnt_new_channel(h_pcnt_unit, &chan_b_config, &h_pcnt_chan_b));
}

static void encoder_init_actions(void)
{
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(
        h_pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE
    ));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(
        h_pcnt_chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE
    ));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(
        h_pcnt_chan_b, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE
    ));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(
        h_pcnt_chan_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE
    ));
}
