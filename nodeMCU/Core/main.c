/**
 * @file main.c
 * @author haihbv
 * @date 5/9/2025
 * @brief Triển khai bootloader: PC (Host) giao tiếp với STM32F1 qua UART.
 */

/******************************************************************************
Includes
******************************************************************************/
#include "main.h"
#include <stdio.h>

/******************************************************************************
Function Prototypes
******************************************************************************/
static void TIM2_HW_Init(void); // Cấu hình TIM2 để blink LED (PWM)

/**
 * @brief  Hàm Setup: khởi tạo hệ thống, delay, TIM2, UART
 */
static void Setup()
{
	SystemInit();		// Init clock mặc định STM32
	delay.Init();		// Khởi tạo delay
	TIM2_HW_Init();		// Cấu hình TIM2 -> blink LED báo trạng thái
	UART1.Init(115200); // Khởi tạo UART1 với baudrate 115200
	printf("Bootloader started... waiting for message from Host\r\n");
}

/**
 * @brief  Vòng lặp chính: đọc dữ liệu từ UART và xử lý command
 */
static void Loop()
{
	static uint8_t rx_buff[256]; // Buffer lưu dữ liệu nhận
	static uint16_t rx_len = 0;	 // Độ dài dữ liệu hiện tại trong buffer

	// Đọc các byte từ UART nếu có
	while (UART1.Available())
	{
		char c = UART1.GetChar();
		rx_buff[rx_len++] = (uint8_t)c;

		// Nếu đã có CMD + LEN thì kiểm tra đủ gói chưa
		if (rx_len >= 2)
		{
			uint8_t expected_len = 2 + rx_buff[1] + 1; // CMD + LEN + PAYLOAD + CHECKSUM
			if (rx_len >= expected_len)
			{
				BL_ProcessCommand(rx_buff, expected_len); // Gọi xử lý command
				rx_len = 0;								  // Reset buffer sau khi xử lý xong
			}
		}

		// Nếu buffer tràn thì reset tránh lỗi
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
	Setup(); // Khởi tạo hệ thống
	while (1)
	{
		Loop(); // Vòng lặp chính
	}
}

/******************************************************************************
TIM2 - PWM (BLINK LED)
******************************************************************************/
/**
 * @brief  Cấu hình TIM2 để tạo PWM blink LED trên chân PA0
 */
static void TIM2_HW_Init(void)
{
	// Bật clock GPIOA và TIM2
	RCC->APB2ENR |= (1 << 2);
	RCC->APB1ENR |= (1 << 0);

	// PA0: cấu hình Alternate Function Push Pull, 50MHz
	GPIOA->CRL &= ~(uint32_t)(0xF << (0 * 4));
	GPIOA->CRL |= (0xB << (0 * 4));

	// Cấu hình TIM2
	TIM2->PSC = 7199;  // Prescaler -> tick = 0.1 ms
	TIM2->ARR = 3999;  // Chu kỳ PWM = 400 ms
	TIM2->CCR1 = 2000; // Duty cycle 50% -> LED sáng 200ms, tắt 200ms

	// Cấu hình PWM mode
	TIM2->CCMR1 &= ~(0xFF);
	TIM2->CCMR1 |= (0x6 << 4);
	TIM2->CCER |= (1 << 0); // Enable kênh 1

	TIM2->EGR |= 1;		   // Update event, reset counter
	TIM2->CR1 |= (1 << 0); // Bật counter
}
