/*
 * my_flash.c
 *
 *  Created on: Jan 23, 2026
 *      Author: User
 */

#include "my_flash.h"
#include "stm32f4xx_hal.h"
#include "my_logger.h"

extern UART_HandleTypeDef huart2;
static uint32_t next_addr = FLASH_START_ADDR;

void Flash_Init_Cursor(void)
{
	uint32_t addr = FLASH_START_ADDR;
	while(addr<FLASH_END_ADDR){
		uint32_t val = *(__IO uint32_t *)addr;

		if(val == 0xFFFFFFFF){
			break;
		}
		addr+=sizeof(LogRecord_t);
	}
	next_addr = addr;
}

void Save_To_Flash(uint32_t *pData, uint16_t word_count)
{
	HAL_FLASH_Unlock();

	if((next_addr+word_count*4)>FLASH_END_ADDR){
		HAL_FLASH_Lock();
		return;
	}
	for(int i = 0;i<word_count;i++){
		if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, next_addr, pData[i]) == HAL_OK){
			next_addr+=4;
		}
	}
	HAL_FLASH_Lock();
}

void Erase_Log_Sector(void)
{
	HAL_FLASH_Unlock();
	FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t SectorError;

	EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.Sector = FLASH_SECTOR_7;
	EraseInitStruct.NbSectors = 1;
	EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;

	if(HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) == HAL_OK){
		next_addr = FLASH_START_ADDR;
	}
	HAL_FLASH_Lock();
}

void Log_Dump(void)
{


    LogRecord_t *pFlashLog = (LogRecord_t *)FLASH_START_ADDR;
    uint32_t max_records = 131072/sizeof(LogRecord_t);
    uint32_t total_written = 0;

    for(uint32_t i = 0;i<max_records;i++){
    	if(pFlashLog[i].time==0xFFFFFFFF){
    		total_written = i;
    		break;
    	}
    }
    if(total_written ==0){
    	char msg[] = "\r\nHistory is empty.\r\n";
    	HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
    	return;
    }

    uint32_t start_index = 0;
    uint32_t SHOW_LIMIT  = 20;
    if(total_written>SHOW_LIMIT){
    	start_index = total_written-SHOW_LIMIT;
    }

	char msg[128];

    sprintf(msg, "\r\n--- History Log ---\r\n");
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
    sprintf(msg, "Time(ms) | Temp(C) | Driver\r\n");
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
    sprintf(msg, "------------------------------\r\n");
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
    for(int i = start_index;i<total_written;i++){
    	char *driver_str = (pFlashLog[i].driver_status==1)? "Mock":"Real";
    	uint32_t time = pFlashLog[i].time;
		uint8_t h = (time>>16)& 0XFF;
		uint8_t m = (time>> 8)& 0XFF;
		uint8_t s = (time>> 0)& 0XFF;

    	sprintf(msg, "%02d:%02d:%02d |   %d    | %s\r\n", h, m, s, pFlashLog[i].temp, driver_str);
    	HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
    }
    sprintf(msg, "------------------------------\r\n");
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
}
