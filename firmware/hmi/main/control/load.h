#ifndef __CONTROL_LOAD_H__
#define __CONTROL_LOAD_H__

#include "server/server.h"

/** General Config */

/** Handlers */
extern load_state_t h_load_state;

/** Prototypes */

/**
 * @brief Initialize load control module
 * @return void
 */
void control_init(void);

#endif /** !__CONTROL_LOAD_H__ */
