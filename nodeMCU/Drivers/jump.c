#include "main.h"

/**
 * @def APP_ADDRESS
 * @brief Địa chỉ base của Vector Table Application (Stack Pointer tại offset 0, Reset_Handler tại offset 4).
 *
 * @note Cần được định nghĩa ở nơi khác (ví dụ: linker script hoặc file cấu hình).
 *       Ví dụ: #define APP_ADDRESS (0x08004000U)
 */

/**
 * @typedef pFunction
 * @brief Kiểu con trỏ hàm trỏ tới entry của ứng dụng (thường là Reset_Handler).
 */
typedef void (*pFunction)(void);

/**
 * @brief Nhảy (handoff) từ Bootloader sang Application.
 *
 * Hàm thực hiện các bước:
 *  - Đọc MSP (Main Stack Pointer) và địa chỉ Reset_Handler từ Vector Table của Application tại @ref APP_ADDRESS.
 *  - Xác thực MSP phải trỏ vào vùng SRAM hợp lệ (0x2000_0000..).
 *  - Vô hiệu hóa ngắt, dừng SysTick, trả clock về mặc định, reset các peripheral.
 *  - Xóa các cờ lỗi đang treo trong SCB (BusFault, UsageFault, MemFault).
 *  - Chuyển Vector Table (SCB->VTOR) sang @ref APP_ADDRESS, đồng bộ lệnh/bộ nhớ.
 *  - Thiết lập MSP cho Application rồi gọi Reset_Handler của Application.
 *
 * @pre  - @ref APP_ADDRESS phải trỏ tới một image Application hợp lệ (Vector Table đầy đủ).
 *       - Bộ nhớ flash tại @ref APP_ADDRESS chứa MSP (offset 0) và Reset_Handler (offset 4) đúng định dạng.
 * @post - Control flow không quay lại Bootloader sau khi gọi entry của Application.
 *
 * @warning
 *  - Nếu MSP không hợp lệ (không nằm trong vùng SRAM), hàm sẽ `return` và KHÔNG nhảy sang App.
 *  - Đảm bảo đã dừng tất cả tác vụ/IRQ tùy chỉnh khác trước khi gọi hàm này, tránh side-effects.
 *
 * @note
 *  - Mặt nạ kiểm tra SRAM (0x2FFE0000U) được dùng theo chuẩn họ STM32F1 để xác minh bit vùng 0x2000_0000.
 *  - Việc reset toàn bộ peripheral thông qua thanh ghi RCC_*RSTR giúp tránh rò cấu hình từ Bootloader sang App.
 *
 * @retval None
 */
void Jump_To_Application(void)
{
  uint32_t app_msp;    /**< Giá trị MSP của Application (đọc từ APP_ADDRESS). */
  uint32_t app_reset;  /**< Địa chỉ Reset_Handler của Application (đọc từ APP_ADDRESS + 4). */
  pFunction app_entry; /**< Con trỏ entry function trỏ tới Reset_Handler của Application. */

  /* Đọc MSP và Reset_Handler từ Vector Table của Application */
  app_msp = *(__IO uint32_t *)(APP_ADDRESS);
  app_reset = *(__IO uint32_t *)(APP_ADDRESS + 4U);

  /* Kiểm tra MSP có hợp lệ (trỏ vào SRAM) hay không.
     Với STM32F1, các bit [31:17] của địa chỉ SRAM thỏa điều kiện 0x2000_0000 khi mask 0x2FFE0000. */
  if ((app_msp & 0x2FFE0000U) != 0x20000000U)
  {
    /* MSP không hợp lệ -> không nhảy sang App */
    return;
  }

  /* Vô hiệu hóa ngắt toàn cục, dừng SysTick, trả clock về mặc định */
  __disable_irq();
  SysTick->CTRL = 0;
  SysTick->LOAD = 0;
  SysTick->VAL = 0;
  RCC_DeInit();

  /* Reset tất cả peripheral để tránh mang trạng thái từ Bootloader sang App */
  RCC->APB1RSTR = 0xFFFFFFFFU;
  RCC->APB1RSTR = 0;
  RCC->APB2RSTR = 0xFFFFFFFFU;
  RCC->APB2RSTR = 0;

  /* Xóa các cờ báo lỗi đang hoạt động trong SCB (nếu có) */
  SCB->SHCSR &= ~(SCB_SHCSR_BUSFAULTACT_Msk |
                  SCB_SHCSR_USGFAULTACT_Msk |
                  SCB_SHCSR_MEMFAULTACT_Msk);

  /* Chuyển Vector Table sang địa chỉ của Application */
  SCB->VTOR = APP_ADDRESS;
  __DSB(); /* Data Synchronization Barrier: đảm bảo mọi ghi bộ nhớ đã hoàn tất */
  __ISB(); /* Instruction Synchronization Barrier: làm mới pipeline/ống lệnh */

  /* Thiết lập MSP cho Application */
  __set_MSP(app_msp);

  /* Gọi Reset_Handler của Application */
  app_entry = (pFunction)app_reset;

  __enable_irq(); /* Bật lại ngắt để Application hoạt động bình thường */
  app_entry();    /* Thực thi Application; không quay về */
}
