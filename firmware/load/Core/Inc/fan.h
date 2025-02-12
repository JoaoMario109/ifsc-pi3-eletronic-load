#ifndef FAN_H
#define FAN_H

#include "stm32f1xx_hal.h"

#define FAN_MAX_TEMPERATURE 10.0f
#define FAN_POWER_MAX_SPEED 20.0f

void fan_init(TIM_HandleTypeDef *htim);

void fan_update();

#endif // !FAN_H