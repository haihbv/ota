#include "delay.h"

volatile uint32_t tickMs = 0;
Delay_Driver_t delay;
void Delay_Init(void)
{
    SysTick->CTRL = 0; // disable Systick
    SysTick->VAL = 0;
    SysTick->LOAD = 72000 - 1;

    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk;
}

void Delay_AutoInit(void) __attribute__((constructor));

void Delay_AutoInit(void)
{
    delay.Init = Delay_Init;
}

void DelayMs(uint32_t msDelay)
{
    uint32_t start = millis();
    while ((millis() - start) < msDelay)
        ;
}
void DelayUs(uint32_t usDelay)
{
    while (usDelay--)
    {
        volatile uint32_t count = 8;
        while (count--)
        {
            __NOP();
        }
    }
}
void SysTick_Handler(void)
{
    tickMs++;
}
