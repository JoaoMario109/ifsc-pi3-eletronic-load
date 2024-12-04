#ifndef __CONTROL_LOAD_H__
#define __CONTROL_LOAD_H__

#include <stdint.h>
#include <stdbool.h>

/** General Config */

#define LOAD_TASK_STACK_SIZE (3 * 1024)
#define LOAD_TASK_PRIORITY 2
#define LOAD_TASK_DELAY 25

#define TRANSMIT_TASK_STACK_SIZE (4 * 1024)
#define TRANSMIT_TASK_PRIORITY 3
#define TRANSMIT_TASK_DELAY 200

/** Handlers */
extern uint16_t h_current_value;
extern bool h_load_enabled;

/** Prototypes */

/**
 * @brief Initialize load control module
 * @return void
 */
void control_init(void);

/**
 * @brief Start load control task
 * @return void
 */
void control_start_task(void);

#endif // !__CONTROL_LOAD_H__
