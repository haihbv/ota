#ifndef DELAY_H
#define DELAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f10x.h"

typedef struct
{
	void (*Init)(void);
} Delay_Driver_t;

extern Delay_Driver_t delay;

void Delay_Init(void);
void DelayMs(uint32_t msDelay);
void DelayUs(uint32_t usDelay);

static inline uint32_t millis(void)
{
	extern volatile uint32_t tickMs;
	return tickMs;
}

void SysTick_Handler(void);

#ifdef __cplusplus
}
#endif /* cplusplus */

#endif /* DELAY_H */
