/**
 * @file    main.h
 * @brief   File header chính cho Bootloader STM32F103.
 *
 * @details
 *  - Khai báo thư viện cần thiết.
 *  - Định nghĩa địa chỉ Application.
 *  - Kiểu boolean cơ bản.
 *  - Sơ đồ tổng quan về quy trình update firmware qua Bootloader.
 */

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
/**
 * @brief Địa chỉ bắt đầu của Application trong Flash.
 * @note  Bootloader sẽ nhảy sang Application từ địa chỉ này sau khi update thành công.
 */
#define APP_ADDRESS (0x08004000)

/******************************************************************************
 * Boolean Type Definition
 ******************************************************************************/
/**
 * @brief Kiểu boolean cơ bản.
 */
typedef enum
{
	false = 0,	  /**< Giá trị false */
	true = !false /**< Giá trị true */
} bool;

#endif // __MAIN_H

/**
 * =========================================================================
 * @section Flow1 Luồng dữ liệu PC -> STM32 Bootloader
 * =========================================================================
 *
 *                  +------------------+
 *                  | application.bin  |
 *                  | (file firmware)  |
 *                  +------------------+
 *                          | đọc file
 *                          v
 *         +------------------------------+
 *         |    hostBootloader.py (PC)    |
 *         |                              |
 *         | 1. ERASE (xóa vùng app)      |
 *         | 2. WRITE (ghi từng block)    |
 *         | 3. VERIFY (so sánh checksum) |
 *         | 4. JUMP (chạy app)           |
 *         +------------------------------+
 *                 | qua UART (COMx, 115200)
 *                 v
 *         +------------------------------+
 *         |   STM32 Bootloader (F103)    |
 *         |                              |
 *         | - Nhận CMD qua UART          |
 *         | - Xử lý:                     |
 *         |   + ERASE -> Flash_Erase()   |
 *         |   + WRITE -> Flash_Write()   |
 *         |   + VERIFY -> tính checksum  |
 *         |   + JUMP  -> Jump_To_App()   |
 *         |                              |
 *         | - Phản hồi: ACK (0x79) hoặc  |
 *         |   NACK (0x1F)                |
 *         +------------------------------+
 *                     |
 *                     v
 *        +------------------------------+
 *        | Application (0x08004000)     |
 *        | chạy bình thường             |
 *        +------------------------------+
 *
 * @subsection Update Quy trình update:
 *   1. PC mở file application.bin
 *   2. PC gửi ERASE -> STM32 gọi Flash_Erase(0x08004000, size)
 *   3. PC gửi block WRITE -> STM32 Flash_Write()
 *   4. PC gửi VERIFY -> STM32 tính checksum app so với PC
 *   5. Nếu đúng -> PC gửi JUMP -> STM32 gọi Jump_To_Application()
 */

/**
 * =========================================================================
 * @section Flow2 Luồng xử lý bên trong STM32 Bootloader
 * =========================================================================
 *
 *  [main.c]
 *    -> Vòng lặp chính:
 *          -> Nhận byte UART -> lưu vào rx_buff
 *          -> Khi đủ gói [CMD][LEN][PAYLOAD][CHECKSUM]
 *                 -> Gọi BL_ProcessCommand()
 *
 *  [bl_cmd.c]
 *    -> BL_ProcessCommand():
 *          -> Kiểm tra checksum
 *          -> switch(cmd):
 *          |     • CMD_ERASE  -> Flash_Erase(APP_ADDRESS, size)
 *          |     • CMD_WRITE  -> Flash_Write(addr, data, len)
 *          |     • CMD_VERIFY -> Tính checksum App, so sánh với PC
 *          |     • CMD_JUMP   -> Jump_To_Application()
 *          -> Gửi phản hồi: ACK (0x79) hoặc NACK (0x1F)
 *
 *  [flash.c]
 *    -> Flash_Erase(): xóa từng page flash (1KB)
 *    -> Flash_Write(): ghi từng halfword (16-bit)
 *    -> Flash_ReadBuffer(): đọc dữ liệu từ flash
 *
 *  [jump.c]
 *    -> Jump_To_Application():
 *          • Tắt SysTick, reset clock, reset peripheral
 *          • Chuyển Vector Table sang APP_ADDRESS (0x08004000)
 *          • Set MSP mới từ vector table app
 *          • Gọi Reset_Handler của Application
 *
 * @subsection Summary Tóm tắt:
 *   - main.c: gom packet UART.
 *   - bl_cmd.c: phân tích & xử lý lệnh.
 *   - flash.c: thao tác flash.
 *   - jump.c: nhảy sang Application.
 */
