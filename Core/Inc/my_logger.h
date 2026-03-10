/*
 * my_logger.h
 *
 *  Created on: Mar 4, 2026
 *      Author: User
 */

#ifndef INC_MY_LOGGER_H_
#define INC_MY_LOGGER_H_

#include "stm32f4xx_hal.h"
#define LOG_MAX_COUNT 10

typedef enum
{
    DRIVER_REAL = 0,
	DRIVER_MOCK = 1
} DriverStatus_e;

typedef struct
{
	uint32_t 		time;
	int8_t   		temp;
	DriverStatus_e  driver_status;
	uint8_t			reserved[2];
} LogRecord_t;

void Add_Log(int8_t current_temp, uint8_t is_mock);

#endif /* INC_MY_LOGGER_H_ */
