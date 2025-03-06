/*
 * flash.h
 *
 *  Created on: Feb 27, 2025
 *      Author: thanh
 */

#ifndef FLASH_FLASH_H_
#define FLASH_FLASH_H_

typedef enum
{
	FLASH_OK       = 0,
	FLASH_ERROR,
	FLASH_BUSY,
	FLASH_TIMEOUT,
} FLASH_StatusTypeDef;

void FLASH_Unlock();
void FLASH_Lock();
FLASH_StatusTypeDef FLASH_Write_Byte(uint32_t address, uint8_t data);
FLASH_StatusTypeDef FLASH_Write_HalfWord(uint32_t address, uint16_t data);
FLASH_StatusTypeDef FLASH_Write_Word(uint32_t address, uint32_t data);
FLASH_StatusTypeDef FLASH_Erase_Sectors(uint8_t sector);
FLASH_StatusTypeDef FLASH_Erase_All(void);

#endif /* FLASH_FLASH_H_ */
