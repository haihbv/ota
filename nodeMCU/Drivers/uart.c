#include "main.h"
#include <stdio.h>

static USART_TypeDef *pUSARTx = USART1; // default USART1

#define RX_BUF_SIZE 2048
static volatile char rx_buffer[RX_BUF_SIZE];
static volatile uint16_t rx_head = 0; // vi tri ghi tiep theo
static volatile uint16_t rx_tail = 0; // vi ti doc tiep theo

/**
 * @brief   Chon USART muon su dung (USART1/USART2)
 * @param   USARTx   Con tro USART
 */
void USART_Select(USART_TypeDef *USARTx)
{
    pUSARTx = USARTx;
}

/**
 * @brief   Cau hinh UART voi baudrate nhap vao
 * @param   baudrate    Toc do baud
 */
void USART_Setup(uint32_t baudrate)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    USART_InitTypeDef USART_InitStruct = {0};
    NVIC_InitTypeDef NVIC_InitStruct = {0};

    if (pUSARTx == USART1)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

        // PA9 - TX
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOA, &GPIO_InitStruct);

        // PA10 - RX
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
        GPIO_Init(GPIOA, &GPIO_InitStruct);

        // NVIC
        NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
    }
    else if (pUSARTx == USART2)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

        // PA2 - TX
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOA, &GPIO_InitStruct);

        // PA3 - RX
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
        GPIO_Init(GPIOA, &GPIO_InitStruct);

        // NVIC
        NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
    }
		else if (pUSARTx == USART3)
		{
				RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

        // PB10 - TX
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOB, &GPIO_InitStruct);

        // PB11 - RX
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
        GPIO_Init(GPIOB, &GPIO_InitStruct);

        // NVIC
        NVIC_InitStruct.NVIC_IRQChannel = USART3_IRQn;
		}

    USART_InitStruct.USART_BaudRate = baudrate;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(pUSARTx, &USART_InitStruct);
    USART_Cmd(pUSARTx, ENABLE);

    // Enable RX interrupt
    USART_ITConfig(pUSARTx, USART_IT_RXNE, ENABLE);

    // Enable NVIC
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/**
 * @brief   Gui 1 ky tu qua UART
 * @param   c   Ky tu
 */
void USART_SendChar(char c)
{
    while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET)
        ;
    USART_SendData(pUSARTx, c);
}

/**
 * @brief   Gui chuoi ky tu qua UART
 * @param   str Chuoi ky tu
 */
void USART_SendString(const char *str)
{
    while (*str)
    {
        USART_SendChar(*str++);
    }
}

/**
 * @brief   Kiem tra co byte san trong buffer RX
 * @retval  1 neu co byte, 0 neu khong
 */
int USART_Available(void)
{
    return (rx_head != rx_tail);
}

/**
 * @brief   Lay 1 byte tu buffer RX
 * @retval  Ky tu tu buffer RX
 */
char USART_GetChar(void)
{
    if (rx_head == rx_tail)
        return 0;

    char c = rx_buffer[rx_tail];
    rx_tail = (rx_tail + 1) % RX_BUF_SIZE;
    return c;
}

/**
 * @brief   Doc 1 byte voi timeout
 * @param   ch      Con tro nhan byte
 * @param   timeout Thoi gian timeout (ms)
 * @retval  1 neu co byte, 0 neu timeout
 */
uint8_t USART_ReadByteTimeout(uint8_t *ch, uint32_t timeout)
{
    uint32_t start = millis();
    while (!USART_Available())
    {
        if ((millis() - start) > timeout)
        {
            return 0; // timeout
        }
    }

    *ch = USART_GetChar();
    return 1;
}

/**
 * @brief   USART1 interrupt: doc byte vao buffer RX
 */
void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        char c = (char)USART_ReceiveData(USART1);
        uint16_t next_head = (rx_head + 1) % RX_BUF_SIZE;
        if (next_head != rx_tail)
        {
            rx_buffer[rx_head] = c;
            rx_head = next_head;
        }
    }
}

/**
 * @brief   USART2 interrupt: doc byte vao buffer RX
 */
void USART2_IRQHandler(void)
{
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        char c = (char)USART_ReceiveData(USART2);
        uint16_t next_head = (rx_head + 1) % RX_BUF_SIZE;
        if (next_head != rx_tail)
        {
            rx_buffer[rx_head] = c;
            rx_head = next_head;
        }
    }
}

/**
 * @brief   USART3 interrupt: doc byte vao buffer RX
 */
void USART3_IRQHandler(void)
{
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        char c = (char)USART_ReceiveData(USART3);
        uint16_t next_head = (rx_head + 1) % RX_BUF_SIZE;
        if (next_head != rx_tail)
        {
            rx_buffer[rx_head] = c;
            rx_head = next_head;
        }
    }
}

/**
 * @brief   Stub cho system exit (khong su dung tren STM32)
 * @param   x   Ma thoat (khong su dung)
 */
__attribute__((noreturn, unused)) static void _sys_exit(int x)
{
    (void)x;
    while (1)
        ;
}

#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)

FILE __stdout;

/**
 * @brief   Redirect printf -> UART
 */
PUTCHAR_PROTOTYPE
{
    (void)f;

    if (ch == '\n')
    {
        USART_SendData(pUSARTx, '\r');
        while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET)
            ;
    }
    USART_SendData(pUSARTx, (uint8_t)ch);
    while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET)
        ;

    return ch;
}

/**
 * @brief   Redirect scanf -> UART
 */
#define GETCHAR_PROTOTYPE int fgetc(FILE *f)

GETCHAR_PROTOTYPE
{
    (void)f;

    while (!USART_Available())
        ;

    return (int)USART_GetChar();
}
