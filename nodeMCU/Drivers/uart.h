#ifndef __UART_H
#define __UART_H

#include "stm32f10x.h"

void USART_Select(USART_TypeDef *USARTx);
void USART_Setup(uint32_t baudrate);

void USART_SendChar(char c);
void USART_SendString(const char *str);

int USART_Available(void);
char USART_GetChar(void);
uint8_t USART_ReadByteTimeout(uint8_t *ch, uint32_t timeout);

void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
void USART3_IRQHandler(void);

#endif
