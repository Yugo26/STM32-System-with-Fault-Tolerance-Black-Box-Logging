/*
 * my_logger.c
 *
 *  Created on: Mar 4, 2026
 *      Author: User
 */
#include "my_logger.h"
#include "main.h"
#include "stm32f4xx_hal.h"
#include "my_flash.h"
#include <stdio.h>
#include <string.h>

extern UART_HandleTypeDef huart2;
extern ADC_HandleTypeDef hadc1;

LogRecord_t log_buffer[LOG_MAX_COUNT];
uint8_t log_index = 0;

void Add_Log(int8_t current_temp, uint8_t is_mock)
{
	LogRecord_t *pLogRecorder = &log_buffer[log_index];
	pLogRecorder->time = Get_RTC_Current_Time();
	pLogRecorder->temp = current_temp;
	pLogRecorder->driver_status = is_mock;

	log_index++;

	if(log_index>=LOG_MAX_COUNT){
		char msg[50];
		sprintf(msg, "Buffer Full Ready to write Flash...\r\n");
		HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);

		uint16_t total_words = LOG_MAX_COUNT *(sizeof(LogRecord_t)/4);
		Save_To_Flash((uint32_t *)log_buffer, total_words);
		log_index = 0;
	}
}


