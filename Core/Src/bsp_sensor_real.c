/*
 * bsp_sensor_real.c
 *
 *  Created on: Feb 24, 2026
 *      Author: User
 */

#include "bsp_sensor.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_i2c.h"
#include "main.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"


extern I2C_HandleTypeDef hi2c1;
#define SHT30_I2C_ADDR (0x44<<1)

static int8_t Real_Init(void)
{
	return 0;
}
static int8_t Real_ReadTemp(float *val)
{
	uint8_t cmd[2] = {0x24, 0x00};
	uint8_t rx_data[6];

	HAL_StatusTypeDef ret;
	ret = HAL_I2C_Master_Transmit(&hi2c1, SHT30_I2C_ADDR, cmd, 2, 100);
	if(ret!=HAL_OK) return -1;

	osDelay(20);

	ret = HAL_I2C_Master_Receive(&hi2c1, SHT30_I2C_ADDR, rx_data, 6, 100);
	if(ret != HAL_OK) return -2;

	uint16_t ST = (rx_data[0] << 8) | rx_data[1];
	*val = -45.0f + 175.0f * ((float)ST / 65535.0f);
	return 0;
}

const SensorDriver_t RealSensorDriver = {
		.Name = "SHT30 Hardware",
		.Init = Real_Init,
		.ReadTemp = Real_ReadTemp,
		.ReadHumi = NULL
};
