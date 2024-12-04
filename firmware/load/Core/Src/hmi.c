#include "hmi.h"
#include "utils.h"
#include <parser.h>

uint8_t RxData[6];
extern mcp4725_t *dac;

I2C_HandleTypeDef *hi2c_hmi;

float dac_voltage = 0.0f;
uint8_t enable = 0;

void hmi_init(I2C_HandleTypeDef *hi2c)
{
	LOG_INFO("I2C: HMI init\n");
	HAL_I2C_EnableListen_IT(hi2c);
	hi2c_hmi = hi2c;
	//HAL_I2C_Slave_Receive_IT(hi2c ,(uint8_t *)RxData, 6);
}

/* Buffer consumer function to advance buffer pointer */
static parser_consumer_data_t consumer(struct buffer_t *buffer)
{
	if (buffer->actual == NULL || buffer->actual >= buffer->end)
	{
		return PARSER_CONSUMER_END_OF_BUFFER;
	}
	buffer->actual += sizeof(uint8_t); // Advance buffer pointer
	return PARSER_CONSUMER_OK;
}

/* Match function for comparing buffer data with node value */
static parser_match_t match_func(void *data, const void *match_value)
{
	LOG_INFO("I2C: Match function comparing %d %d\n", (*(uint8_t *)data), (*(uint8_t *)match_value));
	if ((*(uint8_t *)data) == (*(uint8_t *)match_value))
	{
		return PARSER_MATCH_EQUAL;
	}
	return PARSER_MATCH_NOT_EQUAL;
}

typedef enum
{
	REGISTER_ID = 0x0,
	REGISTER_IN = 0x1,
	REGISTER_MD = 0x2,
	REGISTER_LV = 0x3,
	REGISTER_ST = 0x4,
} register_t;


typedef enum
{
	SUB_REGISTER_GENERAL = 0x0,
	SUB_REGISTER_CC = 0x1,
	SUB_REGISTER_CV = 0x2,
	SUB_REGISTER_CR = 0x3,
	SUB_REGISTER_CP = 0x4,
} sub_register_t;

typedef enum
{
	COMMAND_QUERY = 0x0,
	COMMAND_SET = 0x1,
} command_t;

void parse_i2c(uint8_t *data, uint32_t len)
{

	enable = data[0];
	uint16_t value = (uint16_t)data[1] << 8 | (uint16_t)data[2];

	float current = ((float)(value)) / 1000.0f;
	
	/* Convert desired current value to dac voltage*/
	dac_voltage = (current * 0.08906093f + 0.00743f);

	// /* Create a buffer object for parsing */
	// buffer_t buffer = {
	// 	.begin = data,
	// 	.actual = data,
	// 	.end = &data[len - 1],
	// 	.consumer = &consumer,
	// };

	// /**
	//  * Sample command: C:R:S:CVV
	//  * 
	//  * C: Command (0x0 for query, 0x1 for set)
	//  * R: Mode Regiter
	//  * S: Mode Sub Register
	//  * VV: Value (2 bytes)
	//  * 
	//  * 
	//  * 
	//  * Commands:
	//  * ID: Identifier
	//  * 0x0:0x0:0x0
	//  * IN: Disable/Enable Input
	//  * 0x1:0x1:0x0:V (0x0 for disable, 0x1 for enable)
	//  * MD:
	//  * 0x0:0x2 (Get Mode)
	//  * 0x1:0x2:0x1 (Set Mode to Constant Current)
	//  *    :0x2:0x2 (Set Mode to Constant Voltage)
	//  *    :0x2:0x3 (Set Mode to Constant Resistance)
	//  *    :0x2:0x4 (Set Mode to Constant Power)
	//  * LV:
	//  * 0x0:0x3 (Get Level)
	//  *    :0x3:0x1 (Query Current)
	//  *    :0x3:0x2 (Query Voltage)
	//  *    :0x3:0x3 (Query Resistance)
	//  *    :0x3:0x4 (Query Power)
	//  * 0x1
	//  *    :0x1:0x1:VV (Set Current)
	//  *    :0x2:0x1:VV (Set Voltage)
	//  *    :0x3:0x1:VV (Set Resistance)
	//  *    :0x4:0x1:VV (Set Power)
	//  * ST:
	//  * 0x4:0x0:0x0 (Query Status)
	//  */

	// /* MODE COMMAND SET */
	// MAKE_NODES(
	// 	mode_command_set,
	// 	&match_func,
	// 	{
			

	// /* MODE COMMAND */
	// MAKE_NODES(
	// 	mode_command,
	// 	&match_func,
	// 	{
	// 		MAKE_NODE(COMMAND_QUERY, NULL, NULL),
	// 		MAKE_NODE(COMMAND_SET, NULL, NULL),
	// 	}
	// );

	// /* MODE */
	// MAKE_NODES(
	// 	mode,
	// 	&match_func,
	// 	{
	// 		MAKE_NODE(SUB_REGISTER_GENERAL, NULL, NULL),
	// 		MAKE_NODE(SUB_REGISTER_CC, NULL, NULL),
	// 		MAKE_NODE(SUB_REGISTER_CV, NULL, NULL),
	// 		MAKE_NODE(SUB_REGISTER_CR, NULL, NULL),
	// 		MAKE_NODE(SUB_REGISTER_CP, NULL, NULL),
	// 	}
	// );

	// /* IN Value */
	// MAKE_NODES(
	// 	in_value,
	// 	&match_func,
	// 	{
	// 		MAKE_WILDCARD_NODE(SUB_REGISTER_GENERAL, NULL),
	// 	}
	// );

	// /* IN */
	// MAKE_NODES(
	// 	in,
	// 	&match_func,
	// 	{
	// 		MAKE_NODE(SUB_REGISTER_GENERAL, NULL, NULL),
	// 	}
	// );

	// /* ID */
	// MAKE_NODES(
	// 	id,
	// 	&match_func,
	// 	{
	// 		MAKE_NODE(SUB_REGISTER_GENERAL, NULL, NULL),
	// 	}
	// );

	// /* Define parser nodes */
	// MAKE_NODES(
	// 	root,
	// 	&match_func,
	// 	{
	// 		MAKE_NODE(REGISTER_ID, NULL, &id),
	// 		MAKE_NODE(REGISTER_IN, NULL, NULL),
	// 		MAKE_NODE(REGISTER_MD, NULL, NULL),
	// 		MAKE_NODE(REGISTER_LV, NULL, NULL),
	// 		MAKE_NODE(REGISTER_ST, NULL, NULL),
	// 	}
	// );

}

extern void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{
	LOG_INFO("I2C: Address callback begin\n");
	if (hi2c != hi2c_hmi)
	{
		LOG_ERROR("I2C: Invalid I2C handle\n");
		return;
	}
	if(TransferDirection == I2C_DIRECTION_TRANSMIT)
	{
		LOG_ERROR("I2C: Tran\n");
		HAL_I2C_Slave_Sequential_Receive_IT(hi2c, RxData, 6, I2C_FIRST_AND_LAST_FRAME);
		/* Parse I2C data */
		parse_i2c(RxData, 6);
	}
	else
	{
		//Error_Handler();
	}
}

extern void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
{
	LOG_INFO("I2C: Address match callback\n");
	HAL_I2C_EnableListen_IT(hi2c);
}

// void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
// {
// 	HAL_I2C_EnableListen_IT(hi2c);
// }

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
	uint32_t error = HAL_I2C_GetError(hi2c);
	LOG_ERROR("I2C: Error callback %ld\n", error);
	HAL_I2C_EnableListen_IT(hi2c);
}