#include <stdint.h>

#include "parser.h"

/** Buffer */
uint8_t *msg_buffer;

/** Control Vars */
uint16_t buffer_length = 0U;
parser_state_t parser_state = PARSER_WAIT_START;

void parser_init(uint8_t *buffer, uint16_t length)
{
	msg_buffer = buffer;
	buffer_length = length;
}

void parse_byte(uint8_t byte)
{

}

parser_state_t get_parser_state()
{

}


