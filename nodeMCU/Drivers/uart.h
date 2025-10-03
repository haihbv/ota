#ifndef __USART_H
#define __USART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f10x.h"

typedef struct
{
	void (*Init)(uint32_t BaudRate);
	void (*SendChar)(char c);
	void (*SendStr)(const char *str);
	int (*Available)(void);
	char (*GetChar)(void);
} USART_Driver_t;

extern USART_Driver_t UART1;
extern USART_Driver_t UART2;
extern USART_Driver_t UART3;

void USARTx_Init(USART_TypeDef *USARTx, uint32_t USART_BaudRate);

/*****************************************************************
* Transmit
*****************************************************************/
void USART_SendChar(USART_TypeDef *USARTx, char c);
void USART_SendStr(USART_TypeDef *USARTx, const char *str);

/*****************************************************************
* Receive
*****************************************************************/
void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
void USART3_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif // __USART_H
