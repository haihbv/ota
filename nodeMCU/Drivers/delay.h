#ifndef __DELAY_H
#define __DELAY_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "stm32f10x.h"

void Delay_Init(void);
void Delay_Ms(uint32_t ms);
void Delay_Us(uint32_t us);
uint32_t millis(void);
void SysTick_Handler(void);


#ifdef __cplusplus
}
#endif

#endif // __DELAY_H
