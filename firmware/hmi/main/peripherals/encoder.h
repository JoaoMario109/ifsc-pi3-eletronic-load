#ifndef __PERIPHERALS_ENCODER_H__
#define __PERIPHERALS_ENCODER_H__

#include "driver/pulse_cnt.h"

/** General Config */
#define ENCODER_COUNTER_MAX 100
#define ENCODER_COUNTER_MIN -100

#define ENCODER_GLITCH_FILTER_NS 1000

/** Handlers */
extern pcnt_unit_handle_t h_pcnt_unit;
extern pcnt_channel_handle_t h_pcnt_chan_a;
extern pcnt_channel_handle_t h_pcnt_chan_b;

/** Prototypes */

/**
 * @brief Set up the encoder pulse counter unit
 * @return void
 */
void encoder_init(void);

/**
 * @brief Clear and start pulse counter unit
 * @return void
 */
void encoder_start(void);

/**
 * @brief Stop pulse counter unit
 * @return void
 */
void encoder_stop(void);

/**
 * @brief Get the current encoder count
 * @return void
 */
void encoder_clear(void);

#endif // !__PERIPHERALS_ENCODER_H__
