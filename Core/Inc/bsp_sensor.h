/*
 * bsp_sensor.h
 *
 *  Created on: Feb 17, 2026
 *      Author: User
 */

#ifndef INC_BSP_SENSOR_H_
#define INC_BSP_SENSOR_H_
#include <stdint.h>

typedef struct{
	const char *Name;
	int8_t (*Init)(void);
	int8_t (*ReadTemp)(float *val);
	int8_t (*ReadHumi)(float *val);
} SensorDriver_t;


#endif /* INC_BSP_SENSOR_H_ */
