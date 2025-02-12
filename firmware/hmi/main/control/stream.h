#ifndef __CONTROL_STREAM_H__
#define __CONTROL_STREAM_H__

#include "peripherals/sd.h"

/** General Config */
#define STREAM_TASK_STACK_SIZE 5096U
#define STREAM_TASK_DELAY 100U
#define MSG_OPEN_TIME 5000U

#define STREAM_DATA_FILE SD_MOUNT_POINT "/stream.csv"

/** Prototypes */

/**
 * @brief Initialize stream control module
 * @return void
 */
void stream_init(void);

/**
 * @brief Generic preparations for stream control used when resuming from other screen
 * @return void
 */
void stream_prepare(void);

#endif /** !__CONTROL_STREAM_H__ */
