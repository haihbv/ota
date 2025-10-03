#include "stm32f10x.h"
#include "delay.h"

/*
How to calc "Size" when app starts at 0x08004000:

1. Flash of STM32 starts at 0x08000000.

2. Total Flash size (example: F103C8 = 64KB = 0x10000).
   => EndOfFlash = 0x08000000 + 0x10000 = 0x08010000

3. Bootloader uses 0x0000 ? 0x3FFF (16KB).
   => AppStart = 0x08004000

4. Formula for app size:
   Size = EndOfFlash - AppStart

   Example with 64KB:
   Size = 0x08010000 - 0x08004000
        = 0xC000 (48KB)

5. General rule:
   Size = FlashTotalHex - 0x4000
   (0x4000 = 16KB bootloader)

=> If chip 128KB: Size = 0x20000 - 0x4000 = 0x1C000
   If chip 256KB: Size = 0x40000 - 0x4000 = 0x3C000
   If chip 512KB: Size = 0x80000 - 0x4000 = 0x7C000
*/


static _Bool led_blink = 0;
static uint32_t start = 0;

static void Blink_Led(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStruct);
}

int main(void)
{
	SystemInit();
	delay.Init();
	Blink_Led();
	while (1)
	{
		if (millis() - start >= 1000)
		{
			start = millis();
			led_blink = !led_blink;
			GPIO_WriteBit(GPIOC, GPIO_Pin_13, led_blink ? Bit_SET : Bit_RESET);
		}
	}
}
