#ifndef CONTROL_H
#define CONTROL_H

/* Stm32 HAL */
#include "stm32f1xx_hal.h"

#define CONTROL_FREQUENCY 500.0f

/**
 * @brief Initialize control
 * 
 * @return HAL_StatusTypeDef 
 */
HAL_StatusTypeDef control_init(void);

/**
 * @brief Update control
 * 
 */
void control_update(void);

/**
 * @brief Set the control current setpoint
 * 
 */
void control_set_current_setpoint(float setpoint);

/**
 * @brief Get the control current setpoint
 * 
 * @return float 
 */	
float control_get_current_setpoint(void);

float control_get_current_control_action(void);

float control_get_current_measured_value(void);

void pid_set_constants(double kp, double ki, double kd);



#endif // CONTROL_H