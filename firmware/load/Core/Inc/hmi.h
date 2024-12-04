#ifndef HMI_H
#define HMI_H

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_i2c.h"

#include <mcp4725.h>

void hmi_init(I2C_HandleTypeDef *hi2c);


#endif /* HMI_H */