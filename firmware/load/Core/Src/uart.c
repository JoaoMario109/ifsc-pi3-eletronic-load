#include "uart.h"
#include "adc.h"
#include <utils.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define STR_2_UINT32(STR) *(const uint32_t *)STR
#define VARIABLE 0
#define UART_BUFFER_SIZE (64)

/**
 * Helper macro for make a array of nodes
 */
#define MAKE_NODES(NAME, ...)                         \
	const node_t NAME##_NODE[] =                      \
		__VA_ARGS__;                                  \
	const nodes_t NAME = {                            \
		.nodes = NAME##_NODE,                         \
		.size = sizeof(NAME##_NODE) / sizeof(node_t), \
	}

/**
 * Helper macro for crate parsing nodes
 * COMMAND is the command
 * SEPARATOR is the separating char
 * CALLBACK is the callback function
 * NEXT is the next node
 */
#define MAKE_NODE(COMMAND, SEPARATOR, CALLBACK, NEXT) \
	{                                                 \
		.value.string = STR_2_UINT32(COMMAND),        \
		.value.separator = SEPARATOR,                 \
		.callback = CALLBACK,                         \
		.next = NEXT,                                 \
	}

uint8_t data_uart[UART_BUFFER_SIZE];

typedef enum
{
	DOT,
	COLON,
	SPACE,
	QUESTION,
	PERCENT,
	UNDEFINED,
} separator_t;
typedef struct
{
	union
	{
		uint32_t string;
		uint8_t chars[4];
	};
	separator_t separator;
} command_t;

typedef struct parser_t
{
	/* Index of the start of the parse */
	const uint32_t initial_index;
	/* Index of the consumed data */
	uint32_t actual_index;
	/* Index of the last value that can be consumed */
	const uint32_t end_index;
} parser_t;

typedef struct node_t
{
	const command_t value;
	void const (*callback)(struct parser_t *parser, command_t *command);
	const struct nodes_t *next;
} node_t;

typedef struct nodes_t
{
	const int size;
	const node_t *nodes;
} nodes_t;

static UART_HandleTypeDef *huart;
static mcp4725_t *dac;
static uint32_t echo_command = 0;
static uint32_t print_measurements = 0;

command_t get_command(parser_t *parser);
int get_command_int(parser_t *parser, command_t *command);
float get_command_float(parser_t *parser, command_t *command);

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart2)
{
	// Restart UART reception
	HAL_UART_Receive_DMA(huart, data_uart, UART_BUFFER_SIZE);
}

void uart_init(UART_HandleTypeDef *huart_rx, mcp4725_t *dac_handler)
{
	LOG_INFO("Initializing UART\n");
	// Start UART reception in interrupt mode
	while (HAL_UART_Receive_DMA(huart_rx, data_uart, UART_BUFFER_SIZE) != HAL_OK)
	{
		HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
		HAL_Delay(100);
	}

	huart = huart_rx;
	dac = dac_handler;
}

/**
 * @brief Get the int value of a command XXXX%
 *
 * @param parser parser object that have buffer consumption info
 * @param command the first command (first 4 chars)
 * @return int value
 */
int get_command_int(parser_t *parser, command_t *command)
{
	int value = 0;
	command_t reading_command = *command;
	for (int i = 0; i < 4; i++)
	{
		value += (int)((reading_command.chars[i] - '0') * (powf(10, (3 - i))));
	}

	return value;
}

/**
 * @brief Get the float value of a command XXXX.XXXX
 *
 * @param parser parser object that have buffer consumption info
 * @param command the first command (first 4 chars)
 * @return float value
 */
float get_command_float(parser_t *parser, command_t *command)
{
	float value = 0;
	command_t reading_command = *command;
	value += get_command_int(parser, &reading_command);

	if (reading_command.separator == DOT)
	{
		reading_command = get_command(parser);
		value += get_command_int(parser, &reading_command) / 10000.0f;
	}

	return value;
}

/**
 * @brief Return the command string and separator of the DMA data
 *
 * @param parser parser object that have buffer consumption info
 * @return command_t the command
 */
command_t get_command(parser_t *parser)
{
	command_t command;
	/* Read command (command has 4 chars)*/
	for (int i = 0; i < 4; i++)
	{
		if (parser->actual_index == parser->end_index)
		{
			return command;
		}
		command.chars[i] = data_uart[(parser->actual_index++) % UART_BUFFER_SIZE];
	}
	/* Read separator */
	switch (data_uart[(parser->actual_index++) % UART_BUFFER_SIZE])
	{
	case ':':
		command.separator = COLON;
		break;
	case ' ':
		command.separator = SPACE;
		break;
	case '.':
		command.separator = DOT;
		break;
	case '?':
		command.separator = QUESTION;
		break;
	case '%':
		command.separator = PERCENT;
		break;
	default:
		command.separator = UNDEFINED;
		break;
	}
	return command;
}

void leaf_node_parser(parser_t *parser, command_t *command, const node_t *node)
{
	if (node->callback != NULL)
		node->callback(parser, command);
}

const nodes_t *node_parser(parser_t *parser, command_t *command, const nodes_t *nodes)
{
	const node_t *node = nodes->nodes;

	if (echo_command)
		printf("%s ", (uint8_t *)command->string);

	for (int i = 0; i < nodes->size; i++)
	{
		/* Commands with dot and percent separator are variable*/
		if (command->separator == DOT || command->separator == PERCENT)
		{
			if (node[i].callback != NULL)
				node[i].callback(parser, command);
			return NULL;
		}

		if (command->string == node[i].value.string)
		{
			if (node[i].callback != NULL)
				node[i].callback(parser, command);
			/* Commands with question separator doesn't have next command value */
			if (command->separator == QUESTION && command->separator == node[i].value.separator)
			{
				if (node[i].next != NULL)
				{
					leaf_node_parser(parser, command, node[i].next->nodes);
				}
				return NULL;
			}

			return node[i].next;
		}
	}
	printf("Unrecognized command\n");
	return NULL;
}

void parse_write_hard_enbl(parser_t *parser, command_t *command)
{
	int value = get_command_int(parser, command);
	LOG_INFO("Setting ENABLE_LOAD to %d\n", value);
	HAL_GPIO_WritePin(ENABLE_LOAD_GPIO_Port, ENABLE_LOAD_Pin, value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void parse_write_hard_dac1(parser_t *parser, command_t *command)
{
	float value = get_command_float(parser, command);
	LOG_INFO("Setting DAC1 to %f\n", value);
	mcp4725_set_voltage(dac, 3.3, value, false);
}

void parse_read_meas_curr(parser_t *parser, command_t *command)
{
	printf("%f\n", adc_get_value(ADC_INPUT_CURRENT));
}

void parse_read_meas_voln(parser_t *parser, command_t *command)
{
	printf("%f\n", adc_get_value(ADC_INPUT_VOLTAGE));
}

void parse_read_meas_volk(parser_t *parser, command_t *command)
{
	printf("%f\n", adc_get_value(ADC_INPUT_VOLTAGE_KELVIN));
}

void parse_read_meas_allm(parser_t *parser, command_t *command)
{	
	print_measurements = !print_measurements;
}

void parse_read_syst_ping(parser_t *parser, command_t *command)
{
	printf("PONG\n");
}

void uart_parse(uint32_t last_index, uint32_t index_diff)
{
	/* Make the parser */
	parser_t parser = {
		.initial_index = last_index,
		.actual_index = last_index,
		.end_index = last_index + index_diff,
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
	 *      HARD: HARDware
	 *          DAC1:0000.0000 Write Voltage in DAC (channel and voltage)
	 *          ENBL:0000%           Enable the main relay (0: disable, 1: enable)
	 * READ:
	 *      MEAS: MEASurements
	 *          CURR?          Get input current
	 *          VOLN?          Get normal voltage
	 *          VOLK?          Get Kelvin voltage
	 *          ALLM?          Get all measurements
	 * PING?          Reply with a PING
	 */

	/* WRITE COMMANDS */
	MAKE_NODES(
		dac1,
		{
			MAKE_NODE(VARIABLE, DOT, parse_write_hard_dac1, NULL),
		});

	MAKE_NODES(
		enbl,
		{
			MAKE_NODE(VARIABLE, PERCENT, parse_write_hard_enbl, NULL),
		});

	MAKE_NODES(
		write_hardware,
		{
			MAKE_NODE("DAC1", COLON, NULL, &dac1),
			MAKE_NODE("ENBL", COLON, NULL, &enbl),
		});

	MAKE_NODES(
		write,
		{
			MAKE_NODE("HARD", COLON, NULL, &write_hardware),
		});

	/* READ COMMANDS */
	MAKE_NODES(
		curr,
		{
			MAKE_NODE(VARIABLE, UNDEFINED, parse_read_meas_curr, NULL),
		});

	MAKE_NODES(
		voln,
		{
			MAKE_NODE(VARIABLE, UNDEFINED, parse_read_meas_voln, NULL),
		});

	MAKE_NODES(
		volk,
		{
			MAKE_NODE(VARIABLE, UNDEFINED, parse_read_meas_volk, NULL),
		});

	MAKE_NODES(
		allm,
		{
			MAKE_NODE(VARIABLE, UNDEFINED, parse_read_meas_allm, NULL),
		});

	MAKE_NODES(
		meas,
		{
			MAKE_NODE("CURR", QUESTION, NULL, &curr),
			MAKE_NODE("VOLN", QUESTION, NULL, &voln),
			MAKE_NODE("VOLK", QUESTION, NULL, &volk),
			MAKE_NODE("ALLM", QUESTION, NULL, &allm),
		});

	MAKE_NODES(
		read,
		{
			MAKE_NODE("MEAS", COLON, NULL, &meas),
		});

	/* ROOT COMMAND */
	MAKE_NODES(
		root,
		{
			MAKE_NODE("WRTE", COLON, NULL, &write),
			MAKE_NODE("READ", COLON, NULL, &read),
			MAKE_NODE("PING", QUESTION, parse_read_syst_ping, NULL),
		});

	const nodes_t *nodes = &root;

	/* Consume message */
	for (command_t command = get_command(&parser); parser.actual_index <= parser.end_index; command = get_command(&parser))
	{
		if (nodes == NULL)
		{
			break;
		}

		nodes = node_parser(&parser, &command, nodes);
	}
}

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

/**
 * Uart message recived interrupt callback
 */

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
}