/*
 * bsp_sensor.c
 *
 *  Created on: Feb 17, 2026
 *      Author: User
 */
#include "bsp_sensor.h"
#include <math.h>


static int8_t Mock_Init(void)
{
	return 0;
}

static int8_t Mock_ReadTemp(float *val)
{
	static float fake_temp = 25.0f;
	fake_temp += 0.1f;
	if(fake_temp > 30.0f) fake_temp = 25.0f;

	*val = fake_temp;
	return 0;
}

static int8_t Mock_ReadHumi(float *val)
{
	*val = 60.5f;
	return 0;
}

const SensorDriver_t MockSensorDriver = {
    .Name = "Mock Sensor",
    .Init = Mock_Init,
    .ReadTemp = Mock_ReadTemp,
    .ReadHumi = Mock_ReadHumi
};

