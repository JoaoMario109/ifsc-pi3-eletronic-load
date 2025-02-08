#ifndef CONTROL_H
#define CONTROL_H

#include "stm32f1xx_hal.h"
#include "mcp4725.h"


typedef enum control_mode {
	CONTROL_MODE_CC = 0,
	CONTROL_MODE_CV,
	CONTROL_MODE_CP,
	CONTROL_MODE_CR,
	CONTROL_MODE_SIZE
} control_mode_t;

typedef struct
{
	float max;
	float min;
} boundary_t;

typedef struct
{
	float kp; // Proportional gain
	float ki; // Integral gain
	float kd; // Derivative gain

	float control_frequency; // Control frequency

	boundary_t integral_boundary; // Integral boundary
	boundary_t output_boundary;   // Output boundary

	float error_history[3];  // Store last error values for derivative calculation
	float output_history[3]; // Store last output values for derivative calculation
	float integral;		  // Integral term
	uint32_t history_index;		  // Index for storing error history

	float control_action;

} pid_controller_t;

typedef struct
{
	float setpoint;
	float measured_value;
	float control_action;
} control_io_t;

typedef struct
{
	pid_controller_t pid[CONTROL_MODE_SIZE];

	control_io_t io[CONTROL_MODE_SIZE];

	control_mode_t mode;

	mcp4725_t *dac;
} control_t;



HAL_StatusTypeDef control_init(control_t *control_handler, mcp4725_t *dac);

void control_update(control_t *control_handler);

void control_set_setpoint(control_t *control_handler, control_mode_t mode, float setpoint);

float control_get_setpoint(control_t *control_handler, control_mode_t mode);

void control_set_mode(control_t *control_handler, control_mode_t mode);

void control_set_constants(control_t *control_handler, control_mode_t mode, float kp, float ki, float kd);

void control_set_kp(control_t *control_handler, control_mode_t mode, float kp);
void control_set_ki(control_t *control_handler, control_mode_t mode, float ki);
void control_set_kd(control_t *control_handler, control_mode_t mode, float kd);

#endif // !CONTROL_H