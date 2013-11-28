#include "bcq.h"
#include "sp.h"

#define RBUF_SIZE   (1024)
static uint8_t rbuf[RBUF_SIZE];
static bcq_t rbq;

/*! Initialize the Serial Port !*/
void sp_init(void)
{
    GPIO_InitTypeDef gpio;
    NVIC_InitTypeDef nvic;
	USART_InitTypeDef uart;
	 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    gpio.GPIO_Pin = GPIO_Pin_9;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &gpio);
   
    gpio.GPIO_Pin = GPIO_Pin_10;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio);  

    nvic.NVIC_IRQChannel = USART1_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 3;
    nvic.NVIC_IRQChannelSubPriority = 3;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);

    uart.USART_BaudRate = 115200;
    uart.USART_WordLength = USART_WordLength_8b;
    uart.USART_StopBits = USART_StopBits_1;
    uart.USART_Parity = USART_Parity_No;
    uart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    uart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &uart);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART1, ENABLE);

    bcq_init(&rbq, rbuf, RBUF_SIZE);
}

/*! Check if the Serial Port empty !*/
int sp_empty(void)
{
    return bcq_empty(&rbq);
}

/*! Check if the Serial Port full !*/
int sp_full(void)
{
    return bcq_full(&rbq);
}

/*! Get the Serial Port data count !*/
int sp_count(void)
{
    return bcq_count(&rbq);
}

/*! Get a character from the Serial Port !*/
char sp_getc(void)
{
    if(bcq_empty(&rbq)) return 0;
    else return bcq_pop(&rbq);
}

/*! Put a character to the Serial Port !*/
void sp_putc(char ch)
{
    USART_SendData(USART1, ch);
    while(USART_GetFlagStatus(USART1,USART_FLAG_TC) != SET); 
}

/*! Put a string to the Serial Port !*/
void sp_puts(const char* str)
{
    while(*str)
    {
        USART_SendData(USART1, *str++); 
        while(USART_GetFlagStatus(USART1,USART_FLAG_TC) != SET);
    }
}

/*! Read datas from the Serial Port !*/
void sp_read(uint8_t* datas, int len)
{
    int i;
    if(bcq_count(&rbq) < len) return;
    for(i = 0; i < len; i++) datas[i] = bcq_pop(&rbq);
}

/*! Write datas to the Serial Port !*/
void sp_write(const uint8_t* datas, int len)
{
    int i;
    for(i = 0; i < len; i++)
    {
        USART_SendData(USART1, datas[i]); 
        while(USART_GetFlagStatus(USART1,USART_FLAG_TC) != SET);
    }
}

/*! USART1 interrupt service routine !*/
void USART1_IRQHandler(void)
{
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
        bcq_push(&rbq, USART_ReceiveData(USART1));
}
