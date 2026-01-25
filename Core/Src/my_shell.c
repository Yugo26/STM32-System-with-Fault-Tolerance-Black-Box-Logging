/*
 * my_shell.c
 *
 *  Created on: Jan 23, 2026
 *      Author: User
 */
#include "my_shell.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "my_flash.h"
#include "my_sensor.h"

extern UART_HandleTypeDef huart2;
extern ADC_HandleTypeDef hadc1;

#define  RX_BUFFER_SIZE 32
uint8_t  rx_data;
uint8_t  rx_buffer[RX_BUFFER_SIZE];
uint32_t rx_index = 0;
uint8_t  cmd_flag = 0;

UART_HandleTypeDef *shell_huart;

void Shell_Init(UART_HandleTypeDef *huart_handle)
{
	shell_huart = huart_handle;
	HAL_UART_Receive_IT(shell_huart, &rx_data, 1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance==USART2){
		if(rx_data =='\r' || rx_data =='\n'){
			rx_buffer[rx_index]='\0';
			rx_index = 0;
			cmd_flag = 1;
		}
		else {
			if(rx_index < RX_BUFFER_SIZE-1){
				rx_buffer[rx_index] = rx_data;
				rx_index++;
			}

		}
		HAL_UART_Receive_IT(shell_huart, &rx_data, 1);
	}
}

void Shell_Process()
{
	if(cmd_flag==1){
		HAL_UART_Transmit(shell_huart, rx_buffer, strlen((char*)rx_buffer), 100);
		uint8_t newline[] = "\r\n";
		HAL_UART_Transmit(shell_huart, newline, 2, 100);
		if(strcmp((char*)rx_buffer, "led on")==0){
			HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);

		}
		cmd_flag = 0;
	}
}
