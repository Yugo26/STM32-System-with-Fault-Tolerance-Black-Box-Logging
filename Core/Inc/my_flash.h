/*
 * my_flash.h
 *
 *  Created on: Jan 23, 2026
 *      Author: User
 */

#ifndef INC_MY_FLASH_H_
#define INC_MY_FLASH_H_

#include <stdint.h>
#define FLASH_START_ADDR 0x08060000
#define FLASH_END_ADDR   0x0807FFFF

void Flash_Init_Cursor(void);
void Erase_Log_Sector(void);
void Save_To_Flash(uint32_t *pData, uint16_t word_count);
void Log_Dump(void);

#endif /* INC_MY_FLASH_H_ */
