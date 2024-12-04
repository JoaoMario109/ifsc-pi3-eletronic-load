#include "control.h"
#include "adc.h"
#include <utils.h>
#include <mcp4725.h>
#include <math.h>

#define MAX_CURRENT 10.0f
#define MIN_CURRENT 0.0f

/* Anti windup */
#define MAX_INTEGRAL 3.0f
#define MIN_INTEGRAL -1.0f

#define MAX_DAC_VOLTAGE 3.3f
#define MIN_DAC_VOLTAGE 0.0f

extern mcp4725_t *dac;

/**
 * @brief pid controller main structure
 * 
 */
typedef struct {
	float kp; // Proportional gain
	float ki; // Integral gain
	float kd; // Derivative gain

	double control_frequency; // Control frequency

	double filter_coeff; // Filter coefficient

	double error_history[3];  // Store last error values for derivative calculation
	double output_history[3]; // Store last output values for derivative calculation
	double integral;          // Integral term
	int history_index;        // Index for storing error history

} pid_controller_t;

/**
 * @brief Controller i/o
 * 
 */
typedef struct {
	double setpoint;
	double measured_value;
	double control_action;
} control_io_t;

/* Static variables */
static control_io_t control_io = {
	.setpoint = 1.5f,
	.measured_value = 0.0f,
	.control_action = 0.0f

};
static pid_controller_t pid_controller;

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
	
	/* Initialize pid controller */
	pid_init(
		/* PID controller instance */
		&pid_controller,
		/* Ultimate gain */
		5.0,
		/* Ultimate period */
		0.41,
		/* Control frequency */
		CONTROL_FREQUENCY,
		/* Filter coefficient */
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
void pid_init(pid_controller_t *pid, double ku, double tu, double control_frequency, double filter_coeff) {
	/* Initialize PID controller gains */
	pid->kp = 0.2;
	pid->ki = 74.0;
	pid->kd = 0.00062;

	/* Initialize control frequency */
	pid->control_frequency = control_frequency;

	/* Initialize error histories */
	for (int i = 0; i < (sizeof(pid->error_history) / sizeof(pid->error_history[0])); ++i) {
		pid->error_history[i] = 0.0;
	}

	/* Initialize output histories */
	for (int i = 0; i < (sizeof(pid->output_history) / sizeof(pid->output_history[0])); ++i) {
		pid->output_history[i] = 0.0;
	}

	/* Initialize history index */
	pid->history_index = 0;

	pid->integral = 0.0;

}

void pid_reset(pid_controller_t *pid) 
{
	/* Initialize error histories */
	for (int i = 0; i < (sizeof(pid->error_history) / sizeof(pid->error_history[0])); ++i) {
		pid->error_history[i] = 0.0;
	}

	/* Initialize output histories */
	for (int i = 0; i < (sizeof(pid->output_history) / sizeof(pid->output_history[0])); ++i) {
		pid->output_history[i] = 0.0;
	}

	/* Initialize history index */
	pid->history_index = 0;

	pid->integral = 0.0;
}

void pid_set_constants(double kp, double ki, double kd) 
{
	LOG_INFO("Setting PID constants: %f, %f, %f\n", kp, ki, kd);
	pid_controller.kp = kp;
	pid_controller.ki = ki;
	pid_controller.kd = kd;

	pid_reset(&pid_controller);
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
double pid_update(pid_controller_t *pid, double setpoint, double measured_value) {
	const float kp = pid->kp;
	const float ki = pid->ki;
	//const float Kd = 0;
	const float kd = pid->kd;

	const double error = setpoint - measured_value;

	const double proportional = kp * error;

	pid->integral += ki * ((pid->error_history[0] + error) / 2.0) * (1.0 / pid->control_frequency);

	const double derivative = kd * (2.0 * error - pid->error_history[0] - pid->error_history[1]);

	/* Anti windup */
	if (pid->integral > MAX_INTEGRAL)
		pid->integral = MAX_INTEGRAL;

	else if (pid->integral < MIN_INTEGRAL)
		pid->integral = MIN_INTEGRAL;


	double output = proportional + pid->integral + derivative;

	/* Store error history */
	pid->error_history[2] = pid->error_history[1];
	pid->error_history[1] = pid->error_history[0];
	pid->error_history[0] = error;

	pid->output_history[0] = proportional;

	/* Anti windup */
	if (output > MAX_DAC_VOLTAGE)
		output = MAX_DAC_VOLTAGE;
	else if (output < MIN_DAC_VOLTAGE)
		output = MIN_DAC_VOLTAGE;

	return output;
}

/**
 * @brief Update the control action
 * 
 */
void control_update(void)

{
	control_io.measured_value = adc_get_value(ADC_INPUT_CURRENT);
	control_io.control_action = pid_update(&pid_controller, control_io.setpoint, control_io.measured_value);

	mcp4725_set_voltage(dac, 3.3, control_io.control_action, false);

}

/**
 * @brief setter for the current setpoint
 * 
 * @param setpoint setpoint to set
 */
void control_set_current_setpoint(float setpoint)
{
	control_io.setpoint = setpoint;

	if (setpoint > MAX_CURRENT)
	{
		control_io.setpoint = MAX_CURRENT;
	}
	else if (setpoint < MIN_CURRENT)
	{
		control_io.setpoint = MIN_CURRENT;
	}
}

float control_get_current_setpoint(void)
{
	return control_io.setpoint;
}

float control_get_current_control_action(void)
{
	return control_io.control_action;
}

float control_get_current_measured_value(void)
{
	return control_io.measured_value;
}