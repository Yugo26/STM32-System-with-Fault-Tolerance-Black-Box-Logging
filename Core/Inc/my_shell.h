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

#endif /* INC_MY_SHELL_H_ */
