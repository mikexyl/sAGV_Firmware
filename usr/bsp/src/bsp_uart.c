#include    <stdio.h>
#include    <ucos_ii.h>
#include    "bsp_uart.h"

#define UART1_TX_BUF_SIZE	1*512
#define UART1_RX_BUF_SIZE	1*512

#define UART2_TX_BUF_SIZE	1*512
#define UART2_RX_BUF_SIZE	1*512


UART_T g_tUart1;
uint8_t g_TxBuf1[UART1_TX_BUF_SIZE]; 	/* 发送缓冲区 */
uint8_t g_RxBuf1[UART1_RX_BUF_SIZE]; 	/* 接收缓冲区 */

UART_T g_tUart2;
uint8_t g_TxBuf2[UART2_TX_BUF_SIZE]; 	/* 发送缓冲区 */
uint8_t g_RxBuf2[UART2_RX_BUF_SIZE]; 	/* 接收缓冲区 */

#define	UART1_TX_GPIO		GPIOB
#define	UART1_TX_PIN		GPIO_Pin_6
#define	UART1_TX_PINSOURCE	GPIO_PinSource6

#define	UART1_RX_GPIO		GPIOB
#define	UART1_RX_PIN		GPIO_Pin_7
#define	UART1_RX_PINSOURCE	GPIO_PinSource7

#define	UART1_RCC			RCC_APB2Periph_USART1
#define	UART1_PORT_RCC		RCC_AHB1Periph_GPIOB


#define	UART2_TX_GPIO		GPIOD
#define	UART2_TX_PIN		GPIO_Pin_5
#define	UART2_TX_PINSOURCE	GPIO_PinSource5

#define	UART2_RX_GPIO		GPIOD
#define	UART2_RX_PIN		GPIO_Pin_6
#define	UART2_RX_PINSOURCE	GPIO_PinSource6

#define	UART2_RCC			RCC_APB1Periph_USART2
#define	UART2_PORT_RCC		RCC_AHB1Periph_GPIOD


//static int UartSend(UART_T *_pUart, const u8 *_ucaBuf, u16 _usLen);
static void UartIRQ(UART_T *_pUart);
static void UartVarInit(void);
UART_T *ComToUart(USART_TypeDef* _USARTx);



/****************************************************************************
u16 _stopbit:  USART_StopBits_1
input:
_USARTx: USART1/ USART2
baudrate: u32;
stopbit: USART_StopBits_1/ USART_StopBits_0_5/ USART_StopBits_2/ USART_StopBits_1_5
parity: USART_Parity_No/ USART_Parity_Even/ USART_Parity_Odd
******************************************************************************/

int bsp_InitUart(USART_TypeDef* _USARTx, u32 _baudrata, u16 _stopbit, u16 _parity)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	NVIC_InitTypeDef NVIC_InitStructure;

	USART_StructInit(&USART_InitStructure);
	GPIO_StructInit(&GPIO_InitStructure);
	UartVarInit();
	/***************************************************************
	PB6: USART1_TX; PB7:USART1_RX
	PD6: WIFI_USART2_RX; PD6: WIFI_USART2_TX
	***************************************************************/	
	if(USART1 == _USARTx)
	{
		RCC_AHB1PeriphClockCmd(UART1_PORT_RCC, ENABLE);
		RCC_APB2PeriphClockCmd(UART1_RCC, ENABLE);
		GPIO_PinAFConfig(UART1_TX_GPIO, UART1_TX_PINSOURCE, GPIO_AF_USART1);
		GPIO_PinAFConfig(UART1_RX_GPIO, UART1_RX_PINSOURCE, GPIO_AF_USART1);
		/* 配置 USART Tx 为复用功能 */
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	/* 输出类型为推挽 */
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	/* 内部上拉电阻使能 */
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;	/* 复用模式 */

		GPIO_InitStructure.GPIO_Pin = UART1_TX_PIN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(UART1_TX_GPIO, &GPIO_InitStructure);

		/* 配置 USART Rx 为复用功能 */
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Pin = UART1_RX_PIN;
		GPIO_Init(UART1_RX_GPIO, &GPIO_InitStructure);

		
			/* Enable the USARTx Interrupt */
		NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  //子优先级为0(数值越小,优先级越高,0是最高优先级)
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	}
	else if(USART2 == _USARTx)
	{
		RCC_AHB1PeriphClockCmd(UART2_PORT_RCC, ENABLE);
		RCC_APB1PeriphClockCmd(UART2_RCC, ENABLE);
		GPIO_PinAFConfig(UART2_TX_GPIO, UART2_TX_PINSOURCE, GPIO_AF_USART2);
		GPIO_PinAFConfig(UART2_RX_GPIO, UART2_RX_PINSOURCE, GPIO_AF_USART2);
		/* 配置 USART Tx 为复用功能 */
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	/* 输出类型为推挽 */
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	/* 内部上拉电阻使能 */
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;	/* 复用模式 */

		GPIO_InitStructure.GPIO_Pin = UART2_TX_PIN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(UART2_TX_GPIO, &GPIO_InitStructure);

		/* 配置 USART Rx 为复用功能 */
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Pin = UART2_RX_PIN;
		GPIO_Init(UART2_RX_GPIO, &GPIO_InitStructure);

		
			/* Enable the USARTx Interrupt */
		NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		  //子优先级为1(数值越小,优先级越高,0是最高优先级)
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	}
	else
	{
		return -1;
	}
	USART_InitStructure.USART_BaudRate = _baudrata;	/* 波特率 */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = _stopbit;
	USART_InitStructure.USART_Parity = _parity ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(_USARTx, &USART_InitStructure);

	USART_ITConfig(_USARTx,USART_IT_RXNE,ENABLE); 

	USART_Cmd(_USARTx, ENABLE);		/* 使能串口 */
	return 0;
}


/*
*********************************************************************************************************
*	函 数 名: UartIRQ
*	功能说明: 供中断服务程序调用，通用串口中断处理函数
*	形    参: _pUart : 串口设备
*	返 回 值: 无
*********************************************************************************************************
*/
static void UartIRQ(UART_T *_pUart)
{
	/* 处理接收中断  */
	if (USART_GetITStatus(_pUart->uart, USART_IT_RXNE) != RESET)
	{
		/* 从串口接收数据寄存器读取数据存放到接收FIFO */
		uint8_t ch;

		ch = USART_ReceiveData(_pUart->uart);
		_pUart->pRxBuf[_pUart->usRxWrite] = ch;
		if (++_pUart->usRxWrite >= _pUart->usRxBufSize)
		{
			_pUart->usRxWrite = 0;
	    }
		
		if (_pUart->usRxCount < _pUart->usRxBufSize)
		{
			_pUart->usRxCount++;
		}		
		else
		{//丢弃旧数据
			_pUart->usRxRead++;
		}
		
		if (_pUart->usRxRead >= _pUart->usRxBufSize)
		{
			_pUart->usRxRead = 0;
		}

		/* 回调函数,通知应用程序收到新数据,一般是发送1个消息或者设置一个标记 */
		//if (_pUart->usRxWrite == _pUart->usRxRead)
		//if (_pUart->usRxCount == 1)
		//{
		//	if (_pUart->ReciveNew)
		//	{
		//		_pUart->ReciveNew(ch);
		//	}
		//}
	}

	/* 处理发送缓冲区空中断 */
	if (USART_GetITStatus(_pUart->uart, USART_IT_TXE) != RESET)
	{
		//if (_pUart->usTxRead == _pUart->usTxWrite)
		if (_pUart->usTxCount == 0)
		{
			/* 发送缓冲区的数据已取完时， 禁止发送缓冲区空中断 （注意：此时最后1个数据还未真正发送完毕）*/
			USART_ITConfig(_pUart->uart, USART_IT_TXE, DISABLE);

			/* 使能数据发送完毕中断 */
			USART_ITConfig(_pUart->uart, USART_IT_TC, ENABLE);
			/* 置状态位*/
			_pUart->ucTxState = 1;
		}
		else
		{
			/* 从发送FIFO取1个字节写入串口发送数据寄存器 */
			USART_SendData(_pUart->uart, _pUart->pTxBuf[_pUart->usTxRead]);
			if (++_pUart->usTxRead >= _pUart->usTxBufSize)
			{
				_pUart->usTxRead = 0;
			}
			_pUart->usTxCount--;
		}

	}
	/* 数据bit位全部发送完毕的中断 */
	else if (USART_GetITStatus(_pUart->uart, USART_IT_TC) != RESET)
	{
		//if (_pUart->usTxRead == _pUart->usTxWrite)
		if (_pUart->usTxCount == 0)
		{
			/* 如果发送FIFO的数据全部发送完毕，禁止数据发送完毕中断 */
			USART_ITConfig(_pUart->uart, USART_IT_TC, DISABLE);

			/* 回调函数, 一般用来处理RS485通信，将RS485芯片设置为接收模式，避免抢占总线 */
			if (_pUart->SendOver)
			{
				_pUart->SendOver();
			}
		}
		else
		{
			/* 正常情况下，不会进入此分支 */

			/* 如果发送FIFO的数据还未完毕，则从发送FIFO取1个数据写入发送数据寄存器 */
			USART_SendData(_pUart->uart, _pUart->pTxBuf[_pUart->usTxRead]);
			if (++_pUart->usTxRead >= _pUart->usTxBufSize)
			{
				_pUart->usTxRead = 0;
			}
			_pUart->usTxCount--;
		}
	}
}


void USART1_IRQHandler(void)
{
	OSIntEnter(); 	
	UartIRQ(&g_tUart1);
	OSIntExit(); 
}

void USART2_IRQHandler(void)
{
	OSIntEnter(); 	
	UartIRQ(&g_tUart2);
	OSIntExit(); 
}



/*
*********************************************************************************************************
*	函 数 名: UartVarInit
*	功能说明: 初始化串口相关的变量
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void UartVarInit(void)
{
	g_tUart1.uart = USART1;						/* STM32 串口设备 */
	g_tUart1.pTxBuf = g_TxBuf1;					/* 发送缓冲区指针 */
	g_tUart1.pRxBuf = g_RxBuf1;					/* 接收缓冲区指针 */
	g_tUart1.usTxBufSize = UART1_TX_BUF_SIZE;	/* 发送缓冲区大小 */
	g_tUart1.usRxBufSize = UART1_RX_BUF_SIZE;	/* 接收缓冲区大小 */
	g_tUart1.usTxWrite = 0;						/* 发送FIFO写索引 */
	g_tUart1.usTxRead = 0;						/* 发送FIFO读索引 */
	g_tUart1.usRxWrite = 0;						/* 接收FIFO写索引 */
	g_tUart1.usRxRead = 0;						/* 接收FIFO读索引 */
	g_tUart1.usRxCount = 0;						/* 接收到的新数据个数 */
	g_tUart1.usTxCount = 0;						/* 待发送的数据个数 */
	g_tUart1.SendBefor = 0;						/* 发送数据前的回调函数 */
	g_tUart1.SendOver = 0;						/* 发送完毕后的回调函数 */
	g_tUart1.ReciveNew = 0;						/* 接收到新数据后的回调函数 */

	g_tUart2.uart = USART2;						/* STM32 串口设备 */
	g_tUart2.pTxBuf = g_TxBuf2;					/* 发送缓冲区指针 */
	g_tUart2.pRxBuf = g_RxBuf2;					/* 接收缓冲区指针 */
	g_tUart2.usTxBufSize = UART2_TX_BUF_SIZE;	/* 发送缓冲区大小 */
	g_tUart2.usRxBufSize = UART2_RX_BUF_SIZE;	/* 接收缓冲区大小 */
	g_tUart2.usTxWrite = 0;						/* 发送FIFO写索引 */
	g_tUart2.usTxRead = 0;						/* 发送FIFO读索引 */
	g_tUart2.usRxWrite = 0;						/* 接收FIFO写索引 */
	g_tUart2.usRxRead = 0;						/* 接收FIFO读索引 */
	g_tUart2.usRxCount = 0;						/* 接收到的新数据个数 */
	g_tUart2.usTxCount = 0;						/* 待发送的数据个数 */
	g_tUart2.SendBefor = 0;						/* 发送数据前的回调函数 */
	g_tUart2.SendOver = 0;						/* 发送完毕后的回调函数 */
	g_tUart2.ReciveNew = 0;						/* 接收到新数据后的回调函数 */

}



/*
*********************************************************************************************************
*	函 数 名: ComToUart
*	功能说明: 将COM端口号转换为UART指针
*	形    参: _ucPort: 端口号(COM1 - COM6)
*	返 回 值: uart指针
*********************************************************************************************************
*/
UART_T *ComToUart(USART_TypeDef* _USARTx)
{
	if (USART1 == _USARTx)
	{
 		return &g_tUart1;
	}
	else if (USART2 == _USARTx)
	{
		return &g_tUart2;
	}
	else
	{
		/* 不做任何处理 */
		return 0;
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_UartSend
*	功能说明: 向串口发送一组数据。数据放到发送缓冲区后立即返回，由中断服务程序在后台完成发送
*	形    参: _ucPort: 端口号(COM1 - COM6)
*			  _ucaBuf: 待发送的数据缓冲区
*			  _usLen : 数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
int bsp_UartSend(USART_TypeDef* _USARTx, const u8 *_ucaBuf, u16 _usLen)
{
    //采用轮询发送
    int i;
    for(i=0; i<_usLen; i++)
    {
      USART_SendData(_USARTx, (uint8_t)_ucaBuf[i]);
      while (USART_GetFlagStatus(_USARTx, USART_FLAG_TC) == RESET)
      {;}
    }

    return i;
}

#if 0
/*
*********************************************************************************************************
*	函 数 名: UartSend
*	功能说明: 填写数据到UART发送缓冲区,并启动发送中断。中断处理函数发送完毕后，自动关闭发送中断
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static int UartSend(UART_T *_pUart, const u8 *_ucaBuf, u16 _usLen)
{
	u32 tmvalue = 0;//延时累加参数
	uint16_t i;
	if (_usLen > _pUart->usTxBufSize)
	{
		return -1;
	}

	for (i = 0; i < _usLen; i++)
	{
 		/* 将新数据填入发送缓冲区 */
		_pUart->pTxBuf[_pUart->usTxWrite] = _ucaBuf[i];

		if (++_pUart->usTxWrite >= _pUart->usTxBufSize)
		{
			_pUart->usTxWrite = 0;
		}
		_pUart->usTxCount++;
	}
	/* 初始化状态位*/
	_pUart->ucTxState = 0; 
	USART_ITConfig(_pUart->uart, USART_IT_TXE, ENABLE);
	while(!_pUart->ucTxState)
	{
		tmvalue++;
		if(tmvalue > 2000 * _usLen)
			return -1;
	}
	return 0;
}
#endif


/*
*********************************************************************************************************
*	函 数 名: bsp_UartReceive
*	功能说明: 从串口缓冲区读取1字节，非阻塞。无论有无数据均立即返回
*	形    参: _ucPort: 端口号(COM1 - COM6)
*			  _pByte: 接收到的数据存放在这个地址
*	返 回 值: 0 表示无数据, 1 表示读取到有效字节
*********************************************************************************************************
*/
int bsp_UartReceive(USART_TypeDef* _USARTx, u8 *_prbuff, u16 _usrlen)
{
	UART_T *pUart;
	u16 i = 0;
	int   ret;

	pUart = ComToUart(_USARTx);
	if (pUart == 0)
	{//uart端口号错误，返回-1
		return -1;
	}

	DISABLE_INT();

	if(0 == pUart->usRxCount)
	{//没有有效数据，返回错误
		ret = 0;
		goto EXIT;
	}

	if(_usrlen > pUart->usRxCount)//需求的数据大于有效数据个数, 返回有效数据个数
	{
		for(i = 0; i < pUart->usRxCount; i++)
		{
			*_prbuff = pUart->pRxBuf[pUart->usRxRead];
			if (++pUart->usRxRead >= pUart->usRxBufSize)
			{
				pUart->usRxRead = 0;
			}			
			_prbuff++;
		}
		pUart->usRxCount -= i;

		ret = i;

	}
	else 
	{
		for(i = 0; i < _usrlen; i++)
		{
			*_prbuff = pUart->pRxBuf[pUart->usRxRead];
			if (++pUart->usRxRead >= pUart->usRxBufSize)
			{
				pUart->usRxRead = 0;
			}
			pUart->usRxCount--;
			_prbuff++;
		}

		ret = i;
	}

EXIT:
	ENABLE_INT();
	return ret;
}



/*
*********************************************************************************************************
*	函 数 名: comSendChar
*	功能说明: 向串口发送1个字节。数据放到发送缓冲区后立即返回，由中断服务程序在后台完成发送
*	形    参: _ucPort: 端口号(COM1 - COM6)
*			  _ucByte: 待发送的数据
*	返 回 值: 无
*********************************************************************************************************
*/
void comSendChar(uint8_t _ucByte)
{
	bsp_UartSend(USART1, &_ucByte, 1);
}

int bsp_UartRxNum(USART_TypeDef* _USARTx)
{
    UART_T *puart;

    puart = ComToUart(_USARTx);

    return puart->usRxCount;
}

void bsp_UartClearRx(USART_TypeDef* _USARTx)
{
    UART_T *puart;

    puart = ComToUart(_USARTx);

		DISABLE_INT();
    puart->usRxCount = 0;
    puart->usRxRead = puart->usRxWrite = 0;
    ENABLE_INT();
}

/*
*********************************************************************************************************
*	函 数 名: fputc
*	功能说明: 重定义putc函数，这样可以使用printf函数从串口1打印输出
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
int fputc(int ch, FILE *f)
{
#if 1	/* 将需要printf的字符通过串口中断FIFO发送出去，printf函数会立即返回 */
	comSendChar(ch);

	return ch;
#else	/* 采用阻塞方式发送每个字符,等待数据发送完毕 */
	/* 写一个字节到USART1 */
	USART_SendData(USART1, (uint8_t) ch);

	/* 等待发送结束 */
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
	{}

	return ch;
#endif
}


