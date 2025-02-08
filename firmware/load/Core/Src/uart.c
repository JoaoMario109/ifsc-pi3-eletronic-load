#include "uart.h"
#include "adc.h"
#include "control.h"
#include <utils.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <parser.h>
#include <string.h>

#define STR_2_UINT32(STR) *(const uint32_t *)STR
#define UART_BUFFER_SIZE (500)

uint8_t data_uart[UART_BUFFER_SIZE];

uint32_t trigger_test = 0;

static UART_HandleTypeDef *huart;
static mcp4725_t *dac;
static control_t *control_handler;
static uint32_t echo_command = 0;
static uint32_t print_measurements = 0;

/**
 * @brief Convert string buffer to int value
 * 
 * @param buffer buffer string in format XXXX
 * @return int value
 */
int get_command_int(char *buffer)
{
	int value = 0;
	for (int i = 0; i < 4; i++)
	{
		value += (int)((buffer[i] - '0') * (powf(10, (3 - i))));
	}

	return value;
}

/**
 * @brief Get the float value of a command XXXX.XXXX
 *
 * @param buffer string buffer to parse in format XXXX.XXXX
 * @return float value
 */
float get_command_float(char *buffer)
{
	float value = 0;
	value += get_command_int(buffer);

	if (buffer[4] == '.')
	{
		buffer += 5;
		value += get_command_int(buffer) / 10000.0f;
	}

	return value;
}

/* Buffer consumer function to advance buffer pointer */
parser_consumer_data_t consumer(struct buffer_t *buffer)
{
	if (buffer->actual == NULL || buffer->actual >= buffer->end)
	{
		return PARSER_CONSUMER_END_OF_BUFFER;
	}
	buffer->actual += sizeof(char) * 5; // Each command segment is 5 chars
	return PARSER_CONSUMER_OK;
}

/* Match function for comparing buffer data with node value */
parser_match_t match_func(void *data, const void *match_value)
{
	if (!memcmp((char *)data, (const char *)match_value, strlen((char *)match_value)))
	{
		return PARSER_MATCH_EQUAL;
	}
	return PARSER_MATCH_NOT_EQUAL;
}

/* Callback functions for command handling */
void parse_write_enbl(buffer_t *buffer, const void *value)
{
	int command_value = get_command_int((char *)buffer->actual);
	LOG_INFO("Setting ENABLE_LOAD to: %d\n", command_value);
	HAL_GPIO_WritePin(ENABLE_LOAD_GPIO_Port, ENABLE_LOAD_Pin, command_value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void parse_write_dac1(buffer_t *buffer, const void *value)
{

	float command_value = get_command_float((char *)buffer->actual);
	LOG_INFO("Setting DAC1 to %f\n", get_command_float((char *)buffer->actual));
	mcp4725_set_voltage(dac, 3.3, command_value, false);
}

void parse_read_meas_curr(buffer_t *buffer, const void *value)
{
	LOG_INFO("Current: %f\n", adc_get_value(ADC_INPUT_CURRENT));
}

void parse_read_meas_voln(buffer_t *buffer, const void *value)
{
	LOG_INFO("Voltage (Normal): %f\n", adc_get_value(ADC_INPUT_VOLTAGE));
}

void parse_read_meas_volk(buffer_t *buffer, const void *value)
{
	// LOG_INFO("Voltage (Kelvin): %f\n", adc_get_value(ADC_INPUT_VOLTAGE_KELVIN));
}

void parse_read_meas_allm(buffer_t *buffer, const void *value)
{
	print_measurements = !print_measurements;
	LOG_INFO("Toggle print measurements: %d\n", print_measurements);
}

void parse_read_syst_ping(buffer_t *buffer, const void *value)
{
	LOG_INFO("PONG\n");
}

void parse_write_ctrl_md__(buffer_t *buffer, const void *value)
{
	int command_value = get_command_int((char *)buffer->actual);

	control_mode_t mode;
	switch (command_value)
	{
	default:
	case CONTROL_MODE_CC:
		mode = CONTROL_MODE_CC;
		break;
	case CONTROL_MODE_CV:
		mode = CONTROL_MODE_CV;
		break;
	case CONTROL_MODE_CP:
		mode = CONTROL_MODE_CP;
		break;
	case CONTROL_MODE_CR:
		mode = CONTROL_MODE_CR;
		break;
	}

	LOG_INFO("Setting md__ to %d\n", mode);

	control_set_mode(control_handler, mode);
}

void parse_write_ctrl_cc__(buffer_t *buffer, const void *value)
{
	float command_value = get_command_float((char *)buffer->actual);
	LOG_INFO("Setting cc__ to %f\n", command_value);
	control_set_setpoint(control_handler, CONTROL_MODE_CC, command_value);
}

void parse_write_ctrl_cv__(buffer_t *buffer, const void *value)
{
	float command_value = get_command_float((char *)buffer->actual);
	LOG_INFO("Setting cv__ to %f\n", command_value);
	control_set_setpoint(control_handler, CONTROL_MODE_CV, command_value);
}

void parse_write_ctrl_cp__(buffer_t *buffer, const void *value)
{
	float command_value = get_command_float((char *)buffer->actual);
	LOG_INFO("Setting cp__ to %f\n", command_value);
	control_set_setpoint(control_handler, CONTROL_MODE_CP, command_value);
}

void parse_write_ctrl_cr__(buffer_t *buffer, const void *value)
{
	float command_value = get_command_float((char *)buffer->actual);
	LOG_INFO("Setting cr__ to %f\n", command_value);
	control_set_setpoint(control_handler, CONTROL_MODE_CR, command_value);
}

void parse_write_ctrl_kp__(buffer_t *buffer, const void *value)
{
	float command_value = get_command_float((char *)buffer->actual);
	LOG_INFO("Setting kp__ to %f\n", command_value);
	if (control_handler->mode == CONTROL_MODE_CV)
	{
		command_value = -command_value / 1000.0f;
	}
	
	control_set_kp(control_handler, control_handler->mode, command_value);
}

void parse_write_ctrl_ki__(buffer_t *buffer, const void *value)
{
	float command_value = get_command_float((char *)buffer->actual);
	LOG_INFO("Setting ki__ to %f\n", command_value);
	if (control_handler->mode == CONTROL_MODE_CV)
	{
		command_value = -command_value / 1000.0f;
	}
	control_set_ki(control_handler, control_handler->mode, command_value);
}

void parse_write_ctrl_kd__(buffer_t *buffer, const void *value)
{
	float command_value = get_command_float((char *)buffer->actual);
	LOG_INFO("Setting kd__ to %f\n", command_value);
	if (control_handler->mode == CONTROL_MODE_CV)
	{
		command_value = -command_value / 1000.0f;
	}
	if (control_handler->mode == CONTROL_MODE_CR)
	{
		command_value = -command_value;
	}
	control_set_kd(control_handler, control_handler->mode, command_value);
}

void parse_write_ctrl_trig(buffer_t *buffer, const void *value)
{
	trigger_test = 1;
}


/* UART parsing function */
void uart_parse(uint32_t last_index, uint32_t index_diff)
{
	/* Create a buffer object for parsing */
	buffer_t buffer = {
		.begin = &data_uart[last_index],
		.actual = &data_uart[last_index],
		.end = &data_uart[(last_index + index_diff) % UART_BUFFER_SIZE],
		.consumer = &consumer,
	};

	/**
	 * Sample command: XXXX:XXXX:XXXX:0000.0000
	 * Command should have 4 chars + separator
	 * Separators:
	 * '.': Float value
	 * '%': Int value
	 * '?': Response
	 * ':': Command
	 *
	 * Commands:
	 * WRTE:
	 *     DAC1:0000.0000 Write Voltage in DAC (channel and voltage)
	 *     ENBL:0000%           Enable the main relay (0: disable, 1: enable)
	 *     CTRL:MD__:000X%      Set control mode (0: CC, 1: CV, 2: CP, 3: CR)
	 *     CTRL:CC__:0000.0000  Set CC control setpoint
	 *     CTRL:CV__:0000.0000  Set CV control setpoint
	 *     CTRL:CP__:0000.0000  Set CP control setpoint
	 *     CTRL:CR__:0000.0000  Set CR control setpoint
	 *     CTRL:kp__:0000.0000  Set KP control constant
	 *     CTRL:ki__:0000.0000  Set KI control constant
	 *     CTRL:kd__:0000.0000  Set KD control constant
	 * 
	 * 
	 * READ:
	 *      CURR?          Get input current
	 *      VOLN?          Get normal voltage
	 *      VOLK?          Get Kelvin voltage
	 *      ALLM?          Get all measurements
	 * PING?          Reply with a PING
	 */

	/* Define parser nodes */
	MAKE_NODES(
		write_dac1,
		&match_func,
		{
			MAKE_WILDCARD_NODE(parse_write_dac1, NULL),
		}
	);

	MAKE_NODES(
		write_enbl,
		&match_func,
		{
			MAKE_WILDCARD_NODE(parse_write_enbl, NULL),
		}
	);

	MAKE_NODES(
		write_ctrl_md__,
		&match_func,
		{
			MAKE_WILDCARD_NODE(parse_write_ctrl_md__, NULL),
		}
	);

	MAKE_NODES(
		write_ctrl_cc__,
		&match_func,
		{
			MAKE_WILDCARD_NODE(parse_write_ctrl_cc__, NULL),
		}
	);

	MAKE_NODES(
		write_ctrl_cv__,
		&match_func,
		{
			MAKE_WILDCARD_NODE(parse_write_ctrl_cv__, NULL),
		}
	);

	MAKE_NODES(
		write_ctrl_cp__,
		&match_func,
		{
			MAKE_WILDCARD_NODE(parse_write_ctrl_cp__, NULL),
		}
	);

	MAKE_NODES(
		write_ctrl_cr__,
		&match_func,
		{
			MAKE_WILDCARD_NODE(parse_write_ctrl_cr__, NULL),
		}
	);

		MAKE_NODES(
		write_ctrl_kp__,
		&match_func,
		{
			MAKE_WILDCARD_NODE(parse_write_ctrl_kp__, NULL),
		}
	);


	MAKE_NODES(
		write_ctrl_ki__,
		&match_func,
		{
			MAKE_WILDCARD_NODE(parse_write_ctrl_ki__, NULL),
		}
	);

	MAKE_NODES(
		write_ctrl_kd__,
		&match_func,
		{
			MAKE_WILDCARD_NODE(parse_write_ctrl_kd__, NULL),
		}
	);

	MAKE_NODES(
		write_crtl,
		&match_func,
		{
			MAKE_NODE("MD__:", NULL, &write_ctrl_md__),
			MAKE_NODE("CC__:", NULL, &write_ctrl_cc__),
			MAKE_NODE("CV__:", NULL, &write_ctrl_cv__),
			MAKE_NODE("CP__:", NULL, &write_ctrl_cp__),
			MAKE_NODE("CR__:", NULL, &write_ctrl_cr__),
			MAKE_NODE("KP__:", NULL, &write_ctrl_kp__),
			MAKE_NODE("KI__:", NULL, &write_ctrl_ki__),
			MAKE_NODE("KD__:", NULL, &write_ctrl_kd__),
			MAKE_NODE("TRIG?", parse_write_ctrl_trig, NULL),
		}
	);


	MAKE_NODES(
		write,
		&match_func,
		{
			MAKE_NODE("DAC1:", NULL, &write_dac1),
			MAKE_NODE("ENBL:", NULL, &write_enbl),
			MAKE_NODE("CTRL:", NULL, &write_crtl),
		}
	);

	MAKE_NODES(
		read_meas,
		&match_func,
		{
			MAKE_NODE("CURR?", parse_read_meas_curr, NULL),
			MAKE_NODE("VOLN?", parse_read_meas_voln, NULL),
			MAKE_NODE("VOLK?", parse_read_meas_volk, NULL),
			MAKE_NODE("ALLM?", parse_read_meas_allm, NULL),
		}
	);

	MAKE_NODES(
		root,
		&match_func,
		{
			MAKE_NODE("WRTE:", NULL, &write),
			MAKE_NODE("READ:", NULL, &read_meas),
			MAKE_NODE("PING?", parse_read_syst_ping, NULL),
		}
	);

	/* Start parsing */
	parser(&buffer, &root);
}

/* UART run logic */
void uart_run(void)
{
	static uint32_t last_index_read = 0;
	const uint32_t last_index_dma = UART_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart->hdmarx);
	const uint32_t index_diff = (last_index_dma - last_index_read) % UART_BUFFER_SIZE;

	if (index_diff)
	{
		uart_parse(last_index_read, index_diff);
		last_index_read = last_index_dma;
	}

	if (print_measurements)
	{
		// printf("II: %f, VV: %f, VK: %f\n", adc_get_value(ADC_INPUT_CURRENT), adc_get_value(ADC_INPUT_VOLTAGE), adc_get_value(ADC_INPUT_VOLTAGE_KELVIN));
	}
}

/* UART initialization */
void uart_init(UART_HandleTypeDef *huart_rx, mcp4725_t *dac_handler, control_t *control)
{
	LOG_INFO("Initializing UART\n");

	/* Start UART reception in interrupt mode */
	while (HAL_UART_Receive_DMA(huart_rx, data_uart, UART_BUFFER_SIZE) != HAL_OK)
	{
		HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
		HAL_Delay(100);
	}

	huart = huart_rx;
	dac = dac_handler;
	control_handler = control;
}

/* UART error callback */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	LOG_ERROR("UART Error detected\n");
}
