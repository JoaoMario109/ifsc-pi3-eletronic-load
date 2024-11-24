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

volatile uint8_t data_uart[UART_BUFFER_SIZE];
volatile uint8_t data_received_flag = 0;


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
static uint32_t echo_command = 0;


command_t get_command(parser_t *parser);
int get_command_int(parser_t *parser, command_t *command);
float get_command_float(parser_t *parser, command_t *command);

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart2)
{
	data_received_flag = 1;

	// Restart UART reception
	HAL_UART_Receive_DMA(huart, data_uart, UART_BUFFER_SIZE);
	LOG_INFO("data");
}

void uart_init(UART_HandleTypeDef *huart_rx)
{
	LOG_INFO("Initializing UART\n");
	// Start UART reception in interrupt mode
	while(HAL_UART_Receive_DMA(huart_rx, data_uart, UART_BUFFER_SIZE) != HAL_OK)
	{
		HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
		HAL_Delay(100);

	}
	
	huart = huart_rx;
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

void parse_ping(parser_t *parser, command_t *command)
{
	printf("pong\n");
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
	 * '?': Respose
	 * ':': Command
	 * 
	 * Commands:
	 * READ:
	 *      MEAS: MEASurements
	 *            VBAT? Get Battery Voltage
	 *            IBAT? Get Battery Current
	 *            VPAN? Get Panel Voltage
	 *            IPAN? Get Panel Current
	 *            ALLM? Get All Measurements comma separatedRRRRR
	 *      CTRL: ConTRol
	 *            ALGO? Get the current algorithm
	 * 
	 *
	 * WRTE:
	 *      SYST: System
	 *          PING? Reply the sent message with a ping to the terminal
	 *          ECHO:XXXE% Reply the sent message with the same message (E = 1 enabled, E = 0 disabled)
	 * 			REST:XXXE% Reset peripherical (E = 1: ADC)
	 *      HARD: HARDware
	 *          DUTY:0000.0000 Write PWM DUTY cycle (Is not recommended write directly in the hardware
	 *                                                   because control algorithms change the duty cycle,
	 *                                                   is recommended to use the command: WRTE:CTRL:ALGO:FIXD:0000.0000 instead)
	 *          FREQ:0000.0000 Write FREQuency in kHz
	 *          DAC1:0000%0000 Write Voltage in DAC first int is the channel and second is voltage in mV
	 *      MACH: MACHine
	 *          STAT:(Not implemented)XXX0% Force machine state 
	 *      CTRL: ConTRoL
	 *          ALGO: Force Control Algorithm
	 *              FIXD:0000.0000  Change to fixed duty cycle algorithm with initial duty cycle 0000.0000
	 *              PEOF:0000.0000  Change to PeO fixed step algorithm with initial duty cycle 0000.0000
	 *              BRUT:0000.0000  Change to Brute Force algorithm with initial duty cycle 0000.0000
	 * 				CONF: CONFigure algorithms 
	 * 					PEOF:	Perturbe and Observe
	 * 						STEP:0000.0000 Perturbation steps
	 * 						FREQ:0000.0000 Perturbation frequency in khz
	 * 
	 *
	 */

	/* READ COMMANDS */


	/* ROOT COMMAND */

	MAKE_NODES(
		root,
		{
			MAKE_NODE("PING", QUESTION, parse_ping, NULL),
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
}

/**
 * Uart message recived interrupt callback
 */

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
}