#ifndef __CONTROL_LOAD_H__
#define __CONTROL_LOAD_H__

#include "server/server.h"

/** General Config */
#define LOAD_TASK_STACK_SIZE 5096U
#define LOAD_TASK_DELAY 100U

/** Handlers */
extern load_state_t h_load_state;

/** Use with caution to block enable check loop */
extern bool h_control_enable_active;

/** Prototypes */

/**
 * @brief Initialize load control module
 * @return void
 */
void load_init(void);

/**
 * @brief Generic preparations for load control used when resuming from other screen
 * @return void
 */
void load_prepare(void);

#endif /** !__CONTROL_LOAD_H__ */
