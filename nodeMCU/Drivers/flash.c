#include "main.h"

/*---------------------------------------------------------------------------*/
/* Định nghĩa hằng số cho vùng Flash STM32F103C8                             */
/*---------------------------------------------------------------------------*/

/**
 * @brief   Kích thước 1 trang flash (Page).
 * @note    STM32F103C8: 1KB/page; high-density STM32F1: 2KB/page.
 */
#define FLASH_PAGE_SIZE 1024

/**
 * @brief   Địa chỉ bắt đầu Flash (Base address).
 */
#define FLASH_BASE_ADDR 0x08000000

/**
 * @brief   Tổng dung lượng Flash (64KB).
 */
#define FLASH_TOTAL_SIZE (64 * 1024)

/*---------------------------------------------------------------------------*/
/* Hàm API thao tác với Flash                                                */
/*---------------------------------------------------------------------------*/

/**
 * @brief   Xóa một vùng Flash từ địa chỉ bắt đầu với độ dài chỉ định.
 *
 * @param   startAddr   Địa chỉ bắt đầu xóa (phải thuộc vùng Flash).
 * @param   size        Số byte cần xóa.
 * @retval  FlashStatus
 *          - FLASH_OP_OK            : thành công.
 *          - FLASH_OP_ERROR         : lỗi khi gọi hàm HAL xóa trang.
 *          - FLASH_OP_ADDR_INVALID  : địa chỉ không hợp lệ (ngoài Flash).
 *
 * @note    Hàm sẽ tự động khóa lại Flash sau khi hoàn tất.
 * @warning Địa chỉ @p startAddr phải căn theo page (FLASH_PAGE_SIZE) để xóa chính xác.
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
 * @brief   Ghi dữ liệu vào Flash.
 *
 * @param   addr    Địa chỉ Flash cần ghi (phải là số chẵn, 2-byte aligned).
 * @param   data    Con trỏ buffer dữ liệu cần ghi.
 * @param   len     Số byte cần ghi.
 * @retval  FlashStatus
 *          - FLASH_OP_OK            : thành công.
 *          - FLASH_OP_ERROR         : lỗi khi gọi hàm HAL ghi half-word hoặc verify.
 *          - FLASH_OP_ADDR_INVALID  : địa chỉ không hợp lệ hoặc không đúng alignment.
 *
 * @details
 *  - Ghi theo đơn vị half-word (16 bit).
 *  - Nếu độ dài @p len lẻ, byte cuối cùng sẽ được pad = 0xFF.
 *  - Sau mỗi lần ghi, dữ liệu sẽ được đọc lại và verify.
 *
 * @note    Hàm sẽ tự động khóa lại Flash sau khi hoàn tất.
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

		/* Verify dữ liệu sau khi ghi */
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
 * @brief   Đọc dữ liệu từ Flash vào buffer.
 *
 * @param   addr    Địa chỉ Flash bắt đầu đọc.
 * @param   buf     Con trỏ buffer đích.
 * @param   len     Số byte cần đọc.
 * @retval  None
 *
 * @note    Thao tác đọc từ Flash tương tự như đọc từ bộ nhớ thường.
 */
void Flash_ReadBuffer(uint32_t addr, uint8_t *buf, uint16_t len)
{
	for (uint16_t i = 0; i < len; i++)
	{
		buf[i] = *(volatile uint8_t *)(addr + i);
	}
}
