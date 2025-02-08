#include "adc.h"
#include "fan.h"

static TIM_HandleTypeDef *htim1;

void fan_init(TIM_HandleTypeDef *htim)
{
	htim1 = htim;
	HAL_TIM_PWM_Start(htim, TIM_CHANNEL_1);
}

void fan_update()
{
	uint32_t timer_top = __HAL_TIM_GET_AUTORELOAD(htim1);
	float temperature = adc_get_value(ADC_TEMPERATURE);
	float power = adc_get_value(ADC_INPUT_VOLTAGE) * adc_get_value(ADC_INPUT_CURRENT);

	/* If temperature is above 80C force fan to 100% */
	if (temperature > FAN_MAX_TEMPERATURE)
	{
		__HAL_TIM_SET_COMPARE(htim1, TIM_CHANNEL_1, timer_top);
		return;
	}

	float fan_speed_duty = power / FAN_POWER_MAX_SPEED;

	if (fan_speed_duty > 1.0f)
	{
		fan_speed_duty = 1.0f;
	}

	__HAL_TIM_SET_COMPARE(htim1, TIM_CHANNEL_1, timer_top * fan_speed_duty);
}