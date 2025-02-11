/* Stm32 HAL */
#include "stm32f1xx_hal.h"
/* ADS111x Driver*/
#include "ads111x.h"
/* Utils */
#include "utils.h"
/* ADC header */
#include "adc.h"
#include "uart.h"

/**
 * @brief ADC correction coefficients
 *  Linearization equation:
 *  y = ADC_CORRECTION_COEFFICIENTS[0] + ADC_CORRECTION_COEFFICIENTS[1] * x
 *
 */
static const float ADC_CORRECTION_COEFFICIENTS[ADC_CHANNELS_SIZE][2] = {
	{0.20919486f, 31.65715446f},
	{-0.08353555f, 11.22826758f},
	{0.0f, 1.0f},
	{192.02738f, -72.69488f}};

static const uint8_t ADC_CHANNELS[] = {
	ADS111X_MUX_0_GND,
	ADS111X_MUX_1_GND,
	ADS111X_MUX_2_GND,
	ADS111X_MUX_3_GND,
};

volatile uint16_t dma_adc_buffer[6];

/**
 * @brief ADC structure
 *
 */
adc_t adc;

/**
 * @brief Initialize ADC
 *
 * @param hi2c I2C handle
 * @return HAL_StatusTypeDef HAL status
 */
HAL_StatusTypeDef adc_init(I2C_HandleTypeDef *hi2c, ADC_HandleTypeDef *hadc)
{
	LOG_INFO("Initializing ADC...");

	/* Check if device is ready */
	if (HAL_I2C_IsDeviceReady(hi2c, (uint16_t)(ADS111X_ADDR_GND << 1), 10, 100) != HAL_OK)
	{
		return HAL_ERROR;
	}

	if (ads111x_init_desc(hi2c, ADS111X_ADDR_GND) != HAL_OK)
	{
		LOG_ERROR("ads111x_init_desc");
		return HAL_ERROR;
	}

	/* Save I2C handler */
	adc.hi2c = hi2c;

	/* Set adc ready queue to 1 sample */
	if (ads111x_set_comp_queue(hi2c, ADS111X_COMP_QUEUE_1) != HAL_OK)
	{
		LOG_ERROR("ads111x_set_comp_queue");
	}

	/* Enable conversion ready */
	if (ads111x_enable_conv_ready(hi2c, 1) != HAL_OK)
	{
		LOG_ERROR("ads111x_enable_conv_ready");
	}

	/* Initialize in single mode */
	if (ads111x_set_mode(hi2c, ADS111X_MODE_SINGLE_SHOT) != HAL_OK)
	{
		LOG_ERROR("ads111x_set_mode");
	}

	/* Initialize data rate */
	if (ads111x_set_data_rate(hi2c, ADS111X_DATA_RATE_860) != HAL_OK)
	{
		LOG_ERROR("ads111x_set_data_rate");
	}

	/* Initialize on channel 0 */
	if (ads111x_set_input_mux(hi2c, ADS111X_MUX_0_GND) != HAL_OK)
	{
		LOG_ERROR("ads111x_set_input_mux");
	}
	/* Save last channel */
	adc.last_channel_index = 0;

	/* Initialize gain */
	if (ads111x_set_gain(hi2c, ADS111X_GAIN_4V096) != HAL_OK)
	{
		LOG_ERROR("ads111x_set_gain");
	}
	/* Start conversion */
	if (ads111x_start_conversion(hi2c) != HAL_OK)
	{
		LOG_ERROR("ads111x_start_conversion");
	}

	/* Calibrate temperature sensor */
	if (HAL_ADCEx_Calibration_Start(hadc) != HAL_OK)
	{
		LOG_ERROR("HAL_ADCEx_Calibration_Start");
	}

	/* Start temperature sensor conversion */
	if (HAL_ADC_Start_DMA(hadc, (uint32_t *)dma_adc_buffer, sizeof(dma_adc_buffer) / sizeof(dma_adc_buffer[0])) != HAL_OK)
	{
		LOG_ERROR("HAL_ADC_Start_DMA");
	}

	/* Initialize channels */
	for (uint8_t i = 0; i < ADC_CHANNELS_SIZE; i++)
	{
		adc.channels[i].gain = ADS111X_GAIN_4V096;
		adc.channels[i].value.samples = 0;
		adc.channels[i].value.sum = 0;
		adc.channels[i].value.avg = 0;
	}

	LOG_INFO(" OK.\n");
	/* Adc initialized correctly */
	return HAL_OK;
}

/**
 * @brief Set ADC channel gain
 *
 */
HAL_StatusTypeDef adc_set_gain(adc_channels_t channel, uint16_t value)
{
	const uint16_t LOW_TRESHOLD = (ADS111X_MAX_VALUE / 2);
	const uint16_t HIGH_TRESHOLD = ADS111X_MAX_VALUE;
	const uint16_t HYSTERESIS = ADS111X_MAX_VALUE / 50;

	/* Check if channel is valid */
	if (channel >= ADC_CHANNELS_SIZE)
	{
		LOG_ERROR("Invalid channel");
		return HAL_ERROR;
	}

	/* if value is near of top reduce gain */
	ads111x_gain_t gain = adc.channels[channel].gain;
	if (value > (ADS111X_MAX_VALUE - HYSTERESIS) && gain != ADS111X_GAIN_4V096)
	{
		gain--;
	}
	else if (value < (LOW_TRESHOLD - HYSTERESIS) && gain != ADS111X_GAIN_0V256)
	{
		gain++;
	}

	/* Set gain */
	adc.channels[channel].gain = gain;

	return HAL_OK;
}

/**
 * @brief Measure a channel
 *
 * @return HAL_StatusTypeDef HAL status
 */
HAL_StatusTypeDef adc_measure(void)
{
	/* Temperature sensor uses ADC DMA */
	if (adc.last_channel_index == ADC_TEMPERATURE)
	{
		for (uint8_t i = 0; i < sizeof(dma_adc_buffer) / sizeof(dma_adc_buffer[0]); i++)
		{
			float voltage = (float)dma_adc_buffer[i] * (3.778742f / 4095.0f);
			adc.channels[adc.last_channel_index].value.sum += voltage;
			adc.channels[adc.last_channel_index].value.samples++;
		}
		adc.last_channel_index = (adc.last_channel_index + 1) % (ADC_CHANNELS_SIZE - 1);

		/* If all channels are measured set flag */
		if (adc.last_channel_index == 0)
		{
			adc.all_channels_measured = 1;
		}
	}

	/* If conversion available */
	uint8_t busy;

	if (ads111x_is_busy(adc.hi2c, &busy) != HAL_OK)
	{
		LOG_ERROR("ads111x_failed reading busy flag");
		return HAL_ERROR;
	}

	if (busy)
	{
		LOG_WARN("Aborting adc measurement because adc is busy\n");
		return HAL_OK;
	}

	/* Read conversion result */
	int16_t value;
	if (ads111x_get_value(adc.hi2c, &value) != HAL_OK)
	{
		LOG_ERROR("ads111x_get_value");
		return HAL_ERROR;
	}

	/* Convert to voltage */
	float voltage = (float)value * (ads111x_gain_values[adc.channels[adc.last_channel_index].gain] / ADS111X_MAX_VALUE);

	/* Sum the sample */
	adc.channels[adc.last_channel_index].value.sum += voltage;
	adc.channels[adc.last_channel_index].value.samples++;

	adc_set_gain(adc.last_channel_index, value);

	/* Read next channel */
	adc.last_channel_index = (adc.last_channel_index + 1) % (ADC_CHANNELS_SIZE - 1);

	/* If all channels are measured set flag */
	if (adc.last_channel_index == 0)
	{
		adc.all_channels_measured = 1;
	}

	/* Select next channel */
	if (ads111x_set_input_mux(adc.hi2c, ADC_CHANNELS[adc.last_channel_index]) != HAL_OK)
	{
		LOG_ERROR("ads111x_set_input_mux");
		return HAL_ERROR;
	}

	/* Select gain of the channel */
	if (ads111x_set_gain(adc.hi2c, adc.channels[adc.last_channel_index].gain) != HAL_OK)
	{
		LOG_ERROR("ads111x_set_gain");
		return HAL_ERROR;
	}

	/* Start conversion */
	if (ads111x_start_conversion(adc.hi2c) != HAL_OK)
	{
		LOG_ERROR("ads111x_start_conversion");
		return HAL_ERROR;
	}

	return HAL_OK;
}

/**
 * @brief Calculate average of all channels
 */
void adc_calculate_average(void)
{
	/* Iterate over channels */
	for (uint8_t i = 0; i < ADC_CHANNELS_SIZE; i++)
	{
		/* Division by zero protection */
		if (adc.channels[i].value.samples == 0)
		{
			continue;
		}

		/* Calculate average */
		adc.channels[i].value.avg = adc.channels[i].value.sum / adc.channels[i].value.samples;

		/* Reset sum and samples */
		adc.channels[i].value.sum = 0;
		adc.channels[i].value.samples = 0;
	}
	h_load_state.measurement.cc_milli = (uint32_t)(adc_get_value(ADC_INPUT_CURRENT) * 1000);
	h_load_state.measurement.cv_milli = (uint32_t)(adc_get_value(ADC_INPUT_VOLTAGE) * 1000);
	h_load_state.measurement.cr_milli = (uint32_t)(adc_get_value(ADC_INPUT_VOLTAGE) / adc_get_value(ADC_INPUT_CURRENT) * 1000);
	h_load_state.measurement.cp_milli = (uint32_t)(adc_get_value(ADC_INPUT_VOLTAGE) * adc_get_value(ADC_INPUT_CURRENT) * 1000);
	h_load_state.measurement.temp_milli = (uint32_t)(adc_get_value(ADC_TEMPERATURE) * 1000);

}

/**
 * @brief Get channel value
 *
 * @param channel Channel
 * @return float Value
 */
float adc_get_value(adc_channels_t channel)
{
	/* Check if channel is valid */
	if (channel >= ADC_CHANNELS_SIZE)
	{
		LOG_ERROR("Invalid channel");
		return 0;
	}

	float value = adc.channels[channel].value.avg;

	/* Apply correction coefficients */
	value = ADC_CORRECTION_COEFFICIENTS[channel][1] * value + ADC_CORRECTION_COEFFICIENTS[channel][0];

	/* Get value */
	return value;
}

/**
 * @brief ADC all channels measured
 *
 */
uint8_t adc_all_channels_measured(void)
{
	uint8_t all_channels_measured = adc.all_channels_measured;
	adc.all_channels_measured = 0;
	return all_channels_measured;
}