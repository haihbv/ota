#ifndef __MAIN_H
#define __MAIN_H

/****************************************************************************** 
 * Library Includes
 ******************************************************************************/ 
#include "stm32f10x.h"
#include "delay.h"
#include "uart.h"
#include "jump.h"
#include "flash.h"
#include "bl_cmd.h"

/****************************************************************************** 
 * Application Address Definition
 ******************************************************************************/ 
#define APP_ADDRESS   (0x08004000)  // Dia chi bat dau cua Application trong Flash

/****************************************************************************** 
 * Boolean Type Definition
 ******************************************************************************/
typedef enum
{
	false = 0,
	true = !false
} bool;

#endif // __MAIN_H

/**
 * =========================================================================
 *  So do luong du lieu PC -> STM32 Bootloader
 * =========================================================================
 *
 *                  +------------------+
 *                  | application.bin  |
 *                  | (file firmware)  |
 *                  +------------------+
 *                          | doc file
 *                          v
 *         +------------------------------+
 *         |    hostBootloader.py (PC)    |
 *         |                              |
 *         | 1. ERASE (xoa vung app)      |
 *         | 2. WRITE (ghi tung block)    |
 *         | 3. VERIFY (so sanh checksum) |
 *         | 4. JUMP (chay app)           |
 *         +------------------------------+
 *                 | qua UART (COMx, 115200)
 *                 v
 *         +------------------------------+
 *         |   STM32 Bootloader (F103)    |
 *         |                              |
 *         | - Nhan CMD qua UART          |
 *         | - Xu ly:                     |
 *         |   + ERASE -> Flash_Erase()   |
 *         |   + WRITE -> Flash_Write()   |
 *         |   + VERIFY -> tinh checksum  |
 *         |   + JUMP  -> Jump_To_App()   |
 *         |                              |
 *         | - Phan hoi: ACK (0x79) hoac  |
 *         |   NACK (0x1F)                |
 *         +------------------------------+
 *                     |
 *                     v
 *        +------------------------------+
 *        | Application (tu 0x08004000)  |
 *        | chay binh thuong             |
 *        +------------------------------+
 *
 * -------------------------------------------------------------------------
 * Quy trinh update:
 *   1. PC mo file application.bin
 *   2. PC gui ERASE -> STM32 goi Flash_Erase(0x08004000, size)
 *   3. PC gui block WRITE -> STM32 Flash_Write()
 *   4. PC gui VERIFY -> STM32 tinh checksum app so voi PC
 *   5. Neu dung -> PC gui JUMP -> STM32 goi Jump_To_Application()
 * -------------------------------------------------------------------------
 */
 
/**
 * =========================================================================
 *  So do luong xu ly ben trong STM32 Bootloader
 * =========================================================================
 *
 *  [main.c]
 *    -> Vong lap chinh:
 *          -> Nhan byte UART -> luu vao rx_buff
 *                -> Khi du goi [CMD][LEN][PAYLOAD][CHECKSUM]
 *                       -> Goi BL_ProcessCommand()
 *
 *  [bl_cmd.c]
 *    -> BL_ProcessCommand():
 *          -> Kiem tra checksum
 *          -> switch(cmd):
 *          |     • CMD_ERASE  -> Flash_Erase(APP_ADDRESS, size)
 *          |     • CMD_WRITE  -> Flash_Write(addr, data, len)
 *          |     • CMD_VERIFY -> Tinh checksum App, so sanh voi PC
 *          |     • CMD_JUMP   -> Jump_To_Application()
 *          -> Gui phan hoi: ACK (0x79) hoac NACK (0x1F)
 *
 *  [flash.c]
 *    -> Flash_Erase(): xoa tung page flash (1KB)
 *    -> Flash_Write(): ghi tung halfword (16-bit)
 *    -> Flash_ReadBuffer(): doc du lieu tu flash
 *
 *  [jump.c]
 *    -> Jump_To_Application():
 *          • Tat SysTick, reset clock, reset peripheral
 *          • Chuyen Vector Table sang APP_ADDRESS (0x08004000)
 *          • Set MSP moi tu vector table app
 *          • Goi Reset_Handler cua Application
 *
 * -------------------------------------------------------------------------
 * Tom tat:
 *   - main.c chi lam nhiem vu gom packet UART.
 *   - bl_cmd.c phan tich & xu ly lenh.
 *   - flash.c thao tac voi flash.
 *   - jump.c nhay sang Application.
 * -------------------------------------------------------------------------
 */
