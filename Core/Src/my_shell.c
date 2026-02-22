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
#include "bsp_sensor.h"

extern UART_HandleTypeDef huart2;
extern ADC_HandleTypeDef hadc1;
extern const SensorDriver_t *pSensor;

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
			if(rx_index>0){
				rx_buffer[rx_index]='\0';
				cmd_flag = 1;
				rx_index = 0;
			}
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

CommandID Parse_Command(char *input_str)
{
	if (strcmp(input_str, "led on")  == 0)	return CMD_LED_ON;
	if (strcmp(input_str, "led off") == 0)  return CMD_LED_OFF;
	if (strcmp(input_str, "temp")    == 0)  return CMD_TEMP;
	if (strcmp(input_str, "time")    == 0)  return CMD_TIME;
	if (strcmp(input_str, "status")  == 0)  return CMD_STATUS;
	if (strcmp(input_str, "help")    == 0)  return CMD_HELP;
	if (strcmp(input_str, "bug")     == 0)  return CMD_BUG;

	return CMD_UNKNOWN;
}

void Shell_Process()
{
	if(cmd_flag){

		if(strlen(rx_buffer)==0){
			cmd_flag = 0;
			memset(rx_buffer, 0, sizeof(rx_buffer));
			rx_index = 0;
			return;
		}
		CommandID cmd_id = Parse_Command(rx_buffer);
		char msg[200];

		switch (cmd_id)
		{
			case CMD_LED_ON:
				HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
				sprintf(msg, "LED turned on\r\n");
				HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
				break;

			case CMD_LED_OFF:
				HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
				sprintf(msg, "LED turned off\r\n");
				HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
				break;

			case CMD_TEMP:
				float temp_val = 0.0f;
				if(pSensor!=NULL && pSensor->ReadTemp !=NULL){
					if(pSensor->ReadTemp(&temp_val)==0){
						int t_int = (int)temp_val;
						int t_dec = (int)((temp_val - t_int) * 10);
						sprintf(msg, "Temp: %d.%d C\r\n", t_int, t_dec);
					}
					else sprintf(msg, "Error: Sensor Read Failed\r\n");
				}
				else sprintf(msg, "Error: Sensor Read Failed\r\n");
				HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
				break;

			case CMD_TIME:
				uint32_t time = Get_RTC_Current_Time();
				uint8_t h = (time>>16)& 0XFF;
				uint8_t m = (time>> 8)& 0XFF;
				uint8_t s = (time>> 0)& 0XFF;

				sprintf(msg, "Time: %02d:%02d:%02d (Raw: 0x%08lX)\r\n", h, m, s, time);
				HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
				break;

			case CMD_STATUS:
				uint32_t uptime = HAL_GetTick();
				int up_sec = (uptime / 1000) % 60;
				int up_min = (uptime / 1000 / 60) % 60;
				int up_hr  = (uptime / 1000 / 3600);

				uint32_t rtc_raw = Get_RTC_Current_Time();
				uint8_t rtc_h = (rtc_raw>>16)& 0XFF;
				uint8_t rtc_m = (rtc_raw>> 8)& 0XFF;
				uint8_t rtc_s = (rtc_raw>> 0)& 0XFF;

				const char *drv_name = "Unknown";
				if(pSensor != NULL && pSensor->Name!=NULL){
					drv_name = pSensor->Name;
				}
			    sprintf(msg,
			        "\r\n=== [SYSTEM DASHBOARD] ===\r\n"
			        " Driver : %s\r\n"
			        " RTC    : %02d:%02d:%02d\r\n"
			        " Uptime : %02d:%02d:%02d\r\n"
			        "==========================\r\n",
			        drv_name,
			        rtc_h, rtc_m, rtc_s,
			        up_hr, up_min, up_sec
			    );
			    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);

				break;
			case CMD_HELP:
				sprintf(msg, "List: led on, led off, temp, time\r\n");
				HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
				break;

			case CMD_BUG:
				sprintf(msg, "System is going to freeze...\r\n");
				HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
				while(1){
					HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
					HAL_Delay(100);
				}
				break;

			default:
				sprintf(msg, "Unknown: [%s] (Len:%d, Hex:%02X)\r\n", rx_buffer, strlen(rx_buffer), rx_buffer[0]);
				HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
				break;
		}

		cmd_flag = 0;
		memset(rx_buffer, 0, sizeof(rx_buffer));
	}
}
