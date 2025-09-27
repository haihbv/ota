#ifndef __UART_H
#define __UART_H

#include "stm32f10x.h"

/**
 *	link: https://blog.csdn.net/2401_84378523/article/details/145441410?utm_medium=distribute.pc_relevant.none-task-blog-2~default~baidujs_baidulandingword~default-0-145441410-blog-141330110.235^v43^pc_blog_bottom_relevance_base4&spm=1001.2101.3001.4242.1&utm_relevant_index=3
 */

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
