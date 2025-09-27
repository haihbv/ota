#include "delay.h"

static volatile uint32_t tick_millis = 0;

void Delay_Init(void)
{
    SysTick->LOAD = 72000 - 1;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_ENABLE_Msk |
                    SysTick_CTRL_TICKINT_Msk;
}

uint32_t millis(void)
{
    return tick_millis;
}

void Delay_Ms(uint32_t ms)
{
    uint32_t start = millis();
    while (millis() - start < ms)
        ;
}

void Delay_Us(uint32_t us)
{
    volatile uint32_t count;
    while (us--)
    {
        count = 72;
        while (count--)
            ;
    }
}

void SysTick_Handler(void)
{
    tick_millis++;
}
