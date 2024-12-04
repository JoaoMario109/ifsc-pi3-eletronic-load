#include "control.h"
#include "adc.h"
#include <utils.h>
#include <mcp4725.h>
#include <math.h>

#define MAX_CURRENT 10.0f
#define MIN_CURRENT 0.0f

#define MAX_VOLTAGE 3.3f
#define MIN_VOLTAGE 0.0f

/* Anti windup */
#define MAX_INTEGRAL 3.0f
#define MIN_INTEGRAL -1.0f

#define MAX_DAC_VOLTAGE 3.3f
#define MIN_DAC_VOLTAGE 0.0f

extern mcp4725_t *dac;

typedef enum
{
	CONTROL_MODE_CURRENT,
	CONTROL_MODE_VOLTAGE
} control_mode_t;

/**
 * @brief pid controller main structure
 *
 */
typedef struct
{
	float kp; // Proportional gain
	float ki; // Integral gain
	float kd; // Derivative gain

	double control_frequency; // Control frequency

	double filter_coeff; // Filter coefficient

	double error_history[3];  // Store last error values for derivative calculation
	double output_history[3]; // Store last output values for derivative calculation
	double integral;		  // Integral term
	int history_index;		  // Index for storing error history

} pid_controller_t;

/**
 * @brief Controller i/o
 *
 */
typedef struct
{
	double setpoint;
	double measured_value;
	double control_action;
} control_io_t;

/* Static variables */
static control_io_t current_control_io = {
	.setpoint = 1.5f,
	.measured_value = 0.0f,
	.control_action = 0.0f};

static control_io_t voltage_control_io = {
	.setpoint = 2.5f,
	.measured_value = 0.0f,
	.control_action = 0.0f};

static pid_controller_t current_pid_controller;
static pid_controller_t voltage_pid_controller;
static control_mode_t current_mode = CONTROL_MODE_CURRENT;

/* Static function prototypes */
static void pid_init(pid_controller_t *pid, double ku, double tu, double control_frequency, double filter_coeff);
static double pid_update(pid_controller_t *pid, double setpoint, double measured_value);

/**
 * @brief initialize the control module
 *
 * @return esp_err_t error code
 */
HAL_StatusTypeDef control_init(void)
{
	/* Initialize current PID controller */
	pid_init(
		&current_pid_controller,
		5.0,
		0.41,
		CONTROL_FREQUENCY,
		5.0);

	/* Initialize voltage PID controller */
	pid_init(
		&voltage_pid_controller,
		5.0,
		0.41,
		CONTROL_FREQUENCY,
		5.0);

	return HAL_OK;
}

/**
 * @brief PID controller initialization function
 * 
 * @param pid PID controller instance
 * @param ku Controller ultimate gain
 * @param tu Controller ultimate period
 * @param control_frequency Controller frequency
 * @param filter_coeff Filter coefficient
 * @return * Function returns void
 */
void pid_init(pid_controller_t *pid, double ku, double tu, double control_frequency, double filter_coeff)
{
	pid->kp = 0.2;
	pid->ki = 74.0;
	pid->kd = 0.00062;
	pid->control_frequency = control_frequency;
	pid->integral = 0.0;
	for (int i = 0; i < 3; ++i)
	{
		pid->error_history[i] = 0.0;
		pid->output_history[i] = 0.0;
	}
	pid->history_index = 0;
}

/**
 * @brief PID controller update function
 * @ref https://www.scilab.org/discrete-time-pid-controller-implementation
 * @ref https://scilabdotninja.wordpress.com/scilab-control-engineering-basics/module-4-pid-control/
 * @param pid PID controller instance
 * @param setpoint controller setpoint
 * @param measured_value controller measured value
 * @return double controller output
 */
double pid_update(pid_controller_t *pid, double setpoint, double measured_value)
{
	const double error = setpoint - measured_value;
	const double proportional = pid->kp * error;

	pid->integral += pid->ki * ((pid->error_history[0] + error) / 2.0) * (1.0 / pid->control_frequency);
	if (pid->integral > MAX_INTEGRAL)
		pid->integral = MAX_INTEGRAL;
	else if (pid->integral < MIN_INTEGRAL)
		pid->integral = MIN_INTEGRAL;

	const double derivative = pid->kd * (2.0 * error - pid->error_history[0] - pid->error_history[1]);

	pid->error_history[2] = pid->error_history[1];
	pid->error_history[1] = pid->error_history[0];
	pid->error_history[0] = error;

	double output = proportional + pid->integral + derivative;
	if (output > MAX_DAC_VOLTAGE)
		output = MAX_DAC_VOLTAGE;
	else if (output < MIN_DAC_VOLTAGE)
		output = MIN_DAC_VOLTAGE;

	return output;
}

/**
 * @brief Update the control action based on current mode
 */
void control_update(void)
{
	if (current_mode == CONTROL_MODE_CURRENT)
	{
		current_control_io.measured_value = adc_get_value(ADC_INPUT_CURRENT);
		current_control_io.control_action = pid_update(&current_pid_controller, current_control_io.setpoint, current_control_io.measured_value);
		mcp4725_set_voltage(dac, 3.3, current_control_io.control_action, false);
	}
	else if (current_mode == CONTROL_MODE_VOLTAGE)
	{
		voltage_control_io.measured_value = adc_get_value(ADC_INPUT_VOLTAGE);
		voltage_control_io.control_action = pid_update(&voltage_pid_controller, voltage_control_io.setpoint, voltage_control_io.measured_value);
		mcp4725_set_voltage(dac, 3.3, voltage_control_io.control_action, false);
	}
}

/**
 * @brief Setter for the current mode
 *
 * @param mode CONTROL_MODE_CURRENT or CONTROL_MODE_VOLTAGE
 */
void control_set_mode(control_mode_t mode)
{
	current_mode = mode;
}

/**
 * @brief Setter for the voltage setpoint
 *
 * @param setpoint Voltage setpoint to set
 */
void control_set_voltage_setpoint(float setpoint)
{
	voltage_control_io.setpoint = setpoint;
	if (setpoint > MAX_VOLTAGE)
		voltage_control_io.setpoint = MAX_VOLTAGE;
	else if (setpoint < MIN_VOLTAGE)
		voltage_control_io.setpoint = MIN_VOLTAGE;
}

float control_get_voltage_setpoint(void)
{
	return voltage_control_io.setpoint;
}

float control_get_voltage_measured_value(void)
{
	return voltage_control_io.measured_value;
}
