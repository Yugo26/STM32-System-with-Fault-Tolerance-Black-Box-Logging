/*
 * my_flash.c
 *
 *  Created on: Jan 23, 2026
 *      Author: User
 */

#include "my_flash.h"
#include "stm32f4xx_hal.h"

#define FLASH_STORAGE_ADDR 0x08060000

void Save_To_Flash(uint32_t data)
{
	HAL_FLASH_Unlock();
	FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t SectorError;

	EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.Sector = FLASH_SECTOR_7;
	EraseInitStruct.NbSectors = 1;
	EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;

	if(HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError)!=HAL_OK){
	   HAL_FLASH_Lock();
	   return;
	}
	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_STORAGE_ADDR, data) == HAL_OK){

	}
	HAL_FLASH_Lock();
}

uint32_t Load_From_Flash(void)
{
  return *(__IO uint32_t *)FLASH_STORAGE_ADDR;
}
