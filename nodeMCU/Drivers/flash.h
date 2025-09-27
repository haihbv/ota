#ifndef __FLASH_H
#define __FLASH_H

#include "stm32f10x.h"

#define FLASH_PAGE_SIZE 1024
#define FLASH_BASE_ADDR 0x08000000
#define FLASH_TOTAL_SIZE (64 * 1024) // 64KB flash

typedef enum
{
    FLASH_OP_OK = 0,
    FLASH_OP_ERROR,
    FLASH_OP_ADDR_INVALID
} FlashStatus;

FlashStatus Flash_Erase(uint32_t startAddr, uint32_t size);
FlashStatus Flash_Write(uint32_t addr, uint8_t *data, uint16_t len);
void Flash_ReadBuffer(uint32_t addr, uint8_t *buf, uint16_t len);

#endif
