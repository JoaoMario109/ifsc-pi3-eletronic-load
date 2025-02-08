#include <stdio.h>
#include "control.h"
#include "adc.h"

static void pid_init(pid_controller_t *pid, float kp, float ki, float kd, float control_frequency, boundary_t integral_boundary, boundary_t output_boundary);

static float pid_update(pid_controller_t *pid, control_io_t *io);

static void pid_init(pid_controller_t *pid, float kp, float ki, float kd, float control_frequency, boundary_t integral_boundary, boundary_t output_boundary)
{
	pid->kp = kp;
	pid->ki = ki;
	pid->kd = kd;
	pid->control_frequency = control_frequency;

	/* Initialize internal variables */
	pid->integral = 0.0;

	for (int i = 0; i < 3; ++i)
	{
		pid->error_history[i] = 0.0;
		pid->output_history[i] = 0.0;
	}

	pid->history_index = 0;

	pid->integral_boundary = integral_boundary;

	pid->output_boundary = output_boundary;
}

/**
 * @brief PID controller update function
 * @param pid PID controller instance
 * @param io Control input/output structure
 * @return float controller output
 */
static float pid_update(pid_controller_t *pid, control_io_t *io)
{
	const float error = io->setpoint - io->measured_value;
	const float proportional = pid->kp * error;

	/* If kistory[0] and error have different signs, reset integral term */
	if (pid->error_history[0] * error < 0)
	{
		pid->integral = 0.0;

		for (int i = 0; i < 3; ++i)
		{
			pid->error_history[i] = 0.0;
			pid->output_history[i] = 0.0;
		}
	}

	/* Integral term */
	pid->integral += pid->ki * ((pid->error_history[0] + error) / 2.0) * (1.0 / pid->control_frequency);

	/* Integral boundary */
	if (pid->integral > pid->integral_boundary.max)
		pid->integral = pid->integral_boundary.max;
	else if (pid->integral < pid->integral_boundary.min)
		pid->integral = pid->integral_boundary.min;

	/* Derivative term */
	const float derivative = pid->kd * (2.0 * error - pid->error_history[0] - pid->error_history[1]);

	/* Store error history */
	pid->error_history[2] = pid->error_history[1];
	pid->error_history[1] = pid->error_history[0];
	pid->error_history[0] = error;

	/* Control action */
	float output = proportional + pid->integral + derivative;

	return output;
}

/**
 * @brief Initialize control module
 * @param dac DAC handler
 * @return HAL status
 */
HAL_StatusTypeDef control_init(control_t *control_handler, mcp4725_t *dac)
{
	control_handler->dac = dac;
	printf("Control init\n");
	const boundary_t integral_boundary = {
		.max = 0.03f,
		.min = -0.03f,
	};

	const float control_frequency = 133.0f;

	/* Initialize constant voltage PID controller */
	pid_init(
		&control_handler->pid[CONTROL_MODE_CV],
		-0.00001f,
		-0.00004f,
		-0.00023f,
		control_frequency,
		(boundary_t){
			.min = -3.00f,
			.max = 3.01f
		},
		(boundary_t){
			.min = 0.0f,
			.max = 3.3f
		}
	);

	/* Initialize constant current PID controller */
	pid_init(
		&control_handler->pid[CONTROL_MODE_CC],
		0.008f,
		0.00f,
		0.0f,
		control_frequency,
		integral_boundary,
		(boundary_t){
			.min = -0.3f,
			.max = 0.3f
		}
	);

	/* Initialize constant power PID controller */
	pid_init(
		&control_handler->pid[CONTROL_MODE_CP],
		0.0f,
		0.0f,
		0.0f,
		control_frequency,
		integral_boundary,
		(boundary_t){
			.min = 0.0f,
			.max = 3.3f
		}
	);

	/* Initialize constant resistance PID controller */
	pid_init(
		&control_handler->pid[CONTROL_MODE_CR],
		-0.1f,
		0.0f,
		0.0f,
		control_frequency,
		integral_boundary,
		(boundary_t){
			.min = 0.0f,
			.max = 3.3f
		}
	);

	control_handler->mode = CONTROL_MODE_CP;

	/* Initialize control IO */
	for (uint32_t i = 0; i < CONTROL_MODE_SIZE; ++i)
	{
		control_set_setpoint(control_handler, i, 0.0f);
		control_handler->io[i].measured_value = 0.0f;
		control_handler->io[i].control_action = 0.0f;
	}
	printf("Control init done\n");
	return HAL_OK;
}

/**
 * @brief Update control module
 */
void control_update(control_t *control_handler)
{
	control_handler->io[CONTROL_MODE_CC].measured_value = adc_get_value(ADC_INPUT_CURRENT);
	control_handler->io[CONTROL_MODE_CV].measured_value = adc_get_value(ADC_INPUT_VOLTAGE);
	control_handler->io[CONTROL_MODE_CP].measured_value = adc_get_value(ADC_INPUT_VOLTAGE) * adc_get_value(ADC_INPUT_CURRENT);
	control_handler->io[CONTROL_MODE_CR].measured_value = adc_get_value(ADC_INPUT_VOLTAGE) / adc_get_value(ADC_INPUT_CURRENT);


	if (control_handler->mode == CONTROL_MODE_CP)
	{

		static float last_current_voltage = 0.0f;

		float current_power_error = control_handler->io[CONTROL_MODE_CP].setpoint - control_handler->io[CONTROL_MODE_CP].measured_value;
		float current_voltage_derivative = control_handler->io[CONTROL_MODE_CV].measured_value - last_current_voltage;
		last_current_voltage = control_handler->io[CONTROL_MODE_CV].measured_value;

		if (current_power_error > 0.0f)
		{
			if (current_voltage_derivative <= 0.0f)
			{
				control_handler->pid[CONTROL_MODE_CP].kp = -0.005f;
				control_handler->pid[CONTROL_MODE_CP].ki = 0.0f;
				control_handler->pid[CONTROL_MODE_CP].kd = 0.0f;
			} else 
			{
				control_handler->pid[CONTROL_MODE_CP].kp = 0.005f;
				control_handler->pid[CONTROL_MODE_CP].ki = 0.0f;
				control_handler->pid[CONTROL_MODE_CP].kd = 0.0f;
			}
		} else
		{
			control_handler->pid[CONTROL_MODE_CP].kp = 0.005f;
			control_handler->pid[CONTROL_MODE_CP].ki = 0.0f;
			control_handler->pid[CONTROL_MODE_CP].kd = 0.0f;
		}
	}
	
	control_handler->io[control_handler->mode].control_action += 
		pid_update(&control_handler->pid[control_handler->mode], &control_handler->io[control_handler->mode]);
	

	
	/* Check if control action is not out of bounds */
	if (control_handler->io[control_handler->mode].control_action > control_handler->pid[control_handler->mode].output_boundary.max)
	{
		control_handler->io[control_handler->mode].control_action = control_handler->pid[control_handler->mode].output_boundary.max;
	}
	else if (control_handler->io[control_handler->mode].control_action < control_handler->pid[control_handler->mode].output_boundary.min)
	{
		control_handler->io[control_handler->mode].control_action = control_handler->pid[control_handler->mode].output_boundary.min;
	}

	float analog_setpoint = 0.0f;

	/* Calculate analog setpoint */
	analog_setpoint = control_handler->io[control_handler->mode].control_action;

	/* Analog controller setpoint is controlled for the calculated setpoint + digital control action */
	if (control_handler->mode == CONTROL_MODE_CC)
	{
		float calculated_analog_setpoint = control_handler->io[CONTROL_MODE_CC].setpoint * 0.08906093f + 0.00743f;
		analog_setpoint = calculated_analog_setpoint + control_handler->io[CONTROL_MODE_CC].control_action;
	}

	mcp4725_set_voltage(control_handler->dac, 3.3f, analog_setpoint, false);

}

void control_set_setpoint(control_t *control_handler, control_mode_t mode, float setpoint)
{
	if (mode == CONTROL_MODE_CV) {
		if (setpoint < 0.6f) {
			setpoint = 0.6f;
		} else if (setpoint > 50.0f) {
			setpoint = 50.0f;
		}
	}
	control_handler->io[mode].setpoint = setpoint;
}

float control_get_setpoint(control_t *control_handler, control_mode_t mode)
{
	return control_handler->io[mode].setpoint;
}

void control_set_mode(control_t *control_handler, control_mode_t mode)
{
	control_handler->mode = mode;
}

void control_set_constants(control_t *control_handler, control_mode_t mode, float kp, float ki, float kd)
{
	control_handler->pid[mode].kp = kp;
	control_handler->pid[mode].ki = ki;
	control_handler->pid[mode].kd = kd;
	control_handler->io[mode].control_action = 0.0f;
}

void control_set_kp(control_t *control_handler, control_mode_t mode, float kp)
{
	control_handler->pid[mode].kp = kp;

	/* Reset integral term */
	control_handler->pid[mode].integral = 0.0;

	for (int i = 0; i < 3; ++i)
	{
		control_handler->pid[mode].error_history[i] = 0.0;
		control_handler->pid[mode].output_history[i] = 0.0;
	}
}

void control_set_ki(control_t *control_handler, control_mode_t mode, float ki)
{
	control_handler->pid[mode].ki = ki;

	/* Reset integral term */
	control_handler->pid[mode].integral = 0.0;

	for (int i = 0; i < 3; ++i)
	{
		control_handler->pid[mode].error_history[i] = 0.0;
		control_handler->pid[mode].output_history[i] = 0.0;
	}
}

void control_set_kd(control_t *control_handler, control_mode_t mode, float kd)
{
	control_handler->pid[mode].kd = kd;

	/* Reset integral term */
	control_handler->pid[mode].integral = 0.0;

	for (int i = 0; i < 3; ++i)
	{
		control_handler->pid[mode].error_history[i] = 0.0;
		control_handler->pid[mode].output_history[i] = 0.0;
	}
}