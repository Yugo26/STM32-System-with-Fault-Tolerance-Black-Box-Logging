/*
 * my_sensor.c
 *
 *  Created on: Jan 23, 2026
 *      Author: User
 */
#include "my_sensor.h"

int32_t Convert_To_Temperature(uint32_t adc_raw_val)
{
	uint32_t voltage = (adc_raw_val * 3300) / 4095;
	int32_t temperature = ((int32_t)(voltage - 760) * 10 / 25) + 25;
	return temperature;
}

