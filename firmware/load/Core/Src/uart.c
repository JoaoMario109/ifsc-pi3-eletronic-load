#include "uart.h"
#include "adc.h"
#include <utils.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <parser.h>
#include <string.h>

#define STR_2_UINT32(STR) *(const uint32_t *)STR
#define UART_BUFFER_SIZE (64)

uint8_t data_uart[UART_BUFFER_SIZE];

static UART_HandleTypeDef *huart;
static mcp4725_t *dac;
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
	LOG_INFO("Voltage (Kelvin): %f\n", adc_get_value(ADC_INPUT_VOLTAGE_KELVIN));
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
		write,
		&match_func,
		{
			MAKE_NODE("DAC1:", NULL, &write_dac1),
			MAKE_NODE("ENBL:", NULL, &write_enbl),
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
		printf("II: %f, VV: %f, VK: %f\n", adc_get_value(ADC_INPUT_CURRENT), adc_get_value(ADC_INPUT_VOLTAGE), adc_get_value(ADC_INPUT_VOLTAGE_KELVIN));
	}
}

/* UART initialization */
void uart_init(UART_HandleTypeDef *huart_rx, mcp4725_t *dac_handler)
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
}

/* UART error callback */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	LOG_ERROR("UART Error detected\n");
}
