/*
 * my_shell.h
 *
 *  Created on: Jan 23, 2026
 *      Author: User
 */

#ifndef INC_MY_SHELL_H_
#define INC_MY_SHELL_H_

#include "stm32f4xx_hal.h"

void Shell_Init(UART_HandleTypeDef *huart_handle);
void Shell_Process();

typedef enum
{
	CMD_UNKNOWN = 0,
	CMD_LED_ON,
	CMD_LED_OFF,
	CMD_TEMP,
	CMD_TIME,
	CMD_STATUS,
	CMD_HELP,
	CMD_BUG
} CommandID;

#endif /* INC_MY_SHELL_H_ */
