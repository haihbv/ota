/**

@file main.c

@author haihbv

@date 5/9/2025

@brief Trien khai bootloader de PC(Host) giao tiep voi Node (STM32F1xx)

*/

/******************************************************************************

Includes
******************************************************************************/
 #include "main.h"
 #include <stdio.h>
 
/******************************************************************************

Function Prototypes
******************************************************************************/
static void TIM2_HW_Init(void);

static void Setup()
{
	SystemInit();
	Delay_Init();
	TIM2_HW_Init();
	USART_Select(USART1);
	USART_Setup(115200);
	printf("Booloader started... waiting for message from Host\r\n");
}

static void Loop()
{
	/* Nhan du lieu tu UART */
	static uint8_t rx_buff[256];
	static uint16_t rx_len = 0;
	
	while (USART_Available())
	{
		char c = USART_GetChar();
		rx_buff[rx_len++] = (uint8_t)c; // luu ky tu nhan duoc vao rx_buff
		
		/* Kiem tra xem co nhan duoc it nhat la CMD + LEN*/
		if (rx_len >= 2)
		{
			/* Tinh do dai packet: CMD + LEN + PAYLOAD + CHECKSUM */
			uint8_t expected_len = 2 + rx_buff[1] + 1;
			if (rx_len >= expected_len)
			{
				BL_ProcessCommand(rx_buff, expected_len);
				rx_len = 0;
			}
		}
		
		/* Ngan chan tran bo dem */
		if (rx_len >= sizeof(rx_buff))
		{
			rx_len = 0;
		}
	}
}
/******************************************************************************

Main 
******************************************************************************/
int main(void)
{
	Setup();
	while (1)
	{
		Loop();
	}
}

/******************************************************************************

TIM2 - PWM (BLINK LED)
******************************************************************************/
static void TIM2_HW_Init(void)
{
	/* Enable Clock GPIOA va TIM2 */
	RCC->APB2ENR |= (1 << 2);
	RCC->APB1ENR |= (1 << 0);
	
	/* PA0: Alternate Function Push Pull - 50Mhz */
	GPIOA->CRL &= ~(uint32_t)(0xF << (0 * 4));
	GPIOA->CRL |= (0xB << (0 * 4));
	
	/* Config TIM2 */
	TIM2->PSC = 7199; // tick = 0,1ms
	TIM2->ARR = 3999; // T_PWM = 400ms
	TIM2->CCR1 = 2000; // high = low = 200ms
	
	/* PWM */
	TIM2->CCMR1 &= ~(0xFF);
	TIM2->CCMR1 |= (0x6 << 4);
	TIM2->CCER |= (1 << 0); // Capture enabled
	
	TIM2->EGR |= 1; // update event, reset counter
	TIM2->CR1 |= (1 << 0); // bat counter
}
