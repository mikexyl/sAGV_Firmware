#ifndef	_BSP_UART_H_
#define	_BSP_UART_H_

#include "stm32f4xx.h"
#include "bsp.h"



int bsp_InitUart(USART_TypeDef* _USARTx, u32 _baudrata, u16 _stopbit, u16 _parity);
int bsp_UartSend(USART_TypeDef* _USARTx, const u8 *_ucaBuf, u16 _usLen);
int bsp_UartReceive(USART_TypeDef* _USARTx, u8 *_prbuff, u16 _usrlen);
int bsp_UartRxNum(USART_TypeDef* _USARTx);
void bsp_UartClearRx(USART_TypeDef* _USARTx);

#if 0
void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
#endif


typedef struct
{
	USART_TypeDef *uart;		/* STM32内部串口设备指针 */
	uint8_t *pTxBuf;			/* 发送缓冲区 */
	uint8_t *pRxBuf;			/* 接收缓冲区 */
	uint16_t usTxBufSize;		/* 发送缓冲区大小 */
	uint16_t usRxBufSize;		/* 接收缓冲区大小 */

	volatile uint16_t usTxWrite;			/* 发送缓冲区写指针 */
	volatile uint16_t usTxRead;			/* 发送缓冲区读指针 */
	volatile uint16_t usTxCount;			/* 等待发送的数据个数 */

	uint8_t	ucTxState;			/* 发送状态标志，发送完为1*/

	volatile uint16_t usRxWrite;			/* 接收缓冲区写指针 */
	volatile uint16_t usRxRead;			/* 接收缓冲区读指针 */
	volatile uint16_t usRxCount;			/* 还未读取的新数据个数 */

	void (*SendBefor)(void); 	/* 开始发送之前的回调函数指针（主要用于RS485切换到发送模式） */
	void (*SendOver)(void); 	/* 发送完毕的回调函数指针（主要用于RS485将发送模式切换为接收模式） */
	void (*ReciveNew)(uint8_t _byte);	/* 串口收到数据的回调函数指针 */
}UART_T;

#endif

