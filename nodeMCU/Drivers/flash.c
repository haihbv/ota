#include "main.h"

#define FLASH_PAGE_SIZE 1024		 ///< STM32F103C8: 1KB/page; high-density STM32F1: 2KB/page
#define FLASH_BASE_ADDR 0x08000000	 ///< Dia chi bat dau Flash
#define FLASH_TOTAL_SIZE (64 * 1024) ///< 64KB flash

/**
 * @brief   Xoa toan bo vung Flash tu startAddr voi do dai size
 * @param   startAddr   Dia chi bat dau xoa (phai thuoc vung Flash)
 * @param   size        So byte can xoa
 * @retval  FlashStatus FLASH_OP_OK / FLASH_OP_ERROR / FLASH_OP_ADDR_INVALID
 */
FlashStatus Flash_Erase(uint32_t startAddr, uint32_t size)
{
	if (startAddr < FLASH_BASE_ADDR || startAddr >= (FLASH_BASE_ADDR + FLASH_TOTAL_SIZE))
	{
		return FLASH_OP_ADDR_INVALID;
	}

	uint32_t endAddr = startAddr + size;
	if (endAddr > (FLASH_BASE_ADDR + FLASH_TOTAL_SIZE))
	{
		endAddr = FLASH_BASE_ADDR + FLASH_TOTAL_SIZE;
	}

	FLASH_Unlock();

	for (uint32_t addr = startAddr; addr < endAddr; addr += FLASH_PAGE_SIZE)
	{
		FLASH_Status st = FLASH_ErasePage(addr);
		if (st != FLASH_COMPLETE)
		{
			FLASH_Lock();
			return FLASH_OP_ERROR;
		}
	}

	FLASH_Lock();
	return FLASH_OP_OK;
}

/**
 * @brief   Ghi du lieu vao Flash
 * @param   addr    Dia chi Flash can ghi (phai chan)
 * @param   data    Buffer du lieu can ghi
 * @param   len     So byte can ghi
 * @retval  FlashStatus FLASH_OP_OK / FLASH_OP_ERROR / FLASH_OP_ADDR_INVALID
 */
FlashStatus Flash_Write(uint32_t addr, uint8_t *data, uint16_t len)
{
	if (addr % 2 != 0)
	{
		return FLASH_OP_ADDR_INVALID;
	}
	if (addr < FLASH_BASE_ADDR || addr >= (FLASH_BASE_ADDR + FLASH_TOTAL_SIZE))
	{
		return FLASH_OP_ADDR_INVALID;
	}

	FLASH_Unlock();

	for (uint16_t i = 0; i < len; i += 2)
	{
		uint8_t b1 = data[i];
		uint8_t b2 = (i + 1 < len) ? data[i + 1] : 0xFF;
		uint16_t halfword = (uint16_t)(b1 | (b2 << 8));

		FLASH_Status st = FLASH_ProgramHalfWord(addr + i, halfword);
		if (st != FLASH_COMPLETE)
		{
			FLASH_Lock();
			return FLASH_OP_ERROR;
		}

		// Verify lại dữ liệu sau khi ghi
		if (*(volatile uint16_t *)(addr + i) != halfword)
		{
			FLASH_Lock();
			return FLASH_OP_ERROR;
		}
	}

	FLASH_Lock();
	return FLASH_OP_OK;
}

/**
 * @brief   Doc du lieu tu Flash vao buffer
 * @param   addr    Dia chi Flash bat dau doc
 * @param   buf     Buffer dich
 * @param   len     So byte can doc
 */
void Flash_ReadBuffer(uint32_t addr, uint8_t *buf, uint16_t len)
{
	for (uint16_t i = 0; i < len; i++)
	{
		buf[i] = *(volatile uint8_t *)(addr + i);
	}
}
