#include "main.h"

/****************************************************************************************
 * @typedef pFunction
 * @brief   Kieu con tro ham tro toi Reset_Handler cua Application
 ****************************************************************************************/
typedef void (*pFunction)(void);

/****************************************************************************************
 * @brief  Nhay tu Bootloader sang Application
 * @param  None
 * @retval None
 ****************************************************************************************/
void Jump_To_Application(void)
{
  uint32_t app_msp;
  uint32_t app_reset;
  pFunction app_entry;

  /* Doc MSP va Reset_Handler tu vector table cua App */
  app_msp = *(__IO uint32_t *)(APP_ADDRESS);
  app_reset = *(__IO uint32_t *)(APP_ADDRESS + 4U);

  /* Kiem tra MSP co hop le khong (co tro vao SRAM) */
  if ((app_msp & 0x2FFE0000U) != 0x20000000U)
  {
    return;
  }

  /* Disable global interrupt, reset SysTick, reset clock */
  __disable_irq();
  SysTick->CTRL = 0;
  SysTick->LOAD = 0;
  SysTick->VAL = 0;
  RCC_DeInit();

  /* Reset tat ca peripheral */
  RCC->APB1RSTR = 0xFFFFFFFFU;
  RCC->APB1RSTR = 0;
  RCC->APB2RSTR = 0xFFFFFFFFU;
  RCC->APB2RSTR = 0;

  /* Clear cac co bao loi trong SCB */
  SCB->SHCSR &= ~(SCB_SHCSR_BUSFAULTACT_Msk |
                  SCB_SHCSR_USGFAULTACT_Msk |
                  SCB_SHCSR_MEMFAULTACT_Msk);

  /* Chuyen Vector Table sang dia chi cua App */
  SCB->VTOR = APP_ADDRESS;

  /* Set MSP cho App */
  __set_MSP(app_msp);

  /* Goi Reset_Handler cua App */
  app_entry = (pFunction)app_reset;
  __enable_irq(); /* Bat lai ngat cho App hoat dong */
  app_entry();    /* Thuc thi Application */
}
