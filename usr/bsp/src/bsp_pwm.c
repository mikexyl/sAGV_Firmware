#include "bsp_pwm.h"

/*
	���������GPIO��TIMͨ��:

	TIM1_CH1, PA8,	PE9,
	TIM1_CH2, PA9,	PE11
	TIM1_CH3, PA10,	PE13
	TIM1_CH4, PA11,	PE14

	TIM2_CH1, PA15 (����429��439) 407û�д˽�
	TIM2_CH2, PA1,	PB3
	TIM2_CH3, PA2,	PB10
	TIM2_CH4, PA3,	PB11

	TIM3_CH1, PA6,  PB4, PC6
	TIM3_CH2, PA7,	PB5, PC7
	TIM3_CH3, PB0,	PC8
	TIM3_CH4, PB1,	PC9

	TIM4_CH1, PB6,  PD12
	TIM4_CH2, PB7,	PD13
	TIM4_CH3, PB8,	PD14
	TIM4_CH4, PB9,	PD15

	TIM5_CH1, PA0,  PH10
	TIM5_CH2, PA1,	PH11
	TIM5_CH3, PA2,	PH12
	TIM5_CH4, PA3,	PI10

	TIM8_CH1, PC6,  PI5
	TIM8_CH2, PC7,	PI6
	TIM8_CH3, PC8,	PI7
	TIM8_CH4, PC9,	PI2

	TIM9_CH1, PA2,  PE5
	TIM9_CH2, PA3,	PE6

	TIM10_CH1, PB8,  PF6

	TIM11_CH1, PB9,  PF7

	TIM12_CH1, PB14,  PH6
	TIM12_CH2, PB15,  PH9

	TIM13_CH1, PA6,  PF8
	TIM14_CH1, PA7,  PF9

	APB1 ��ʱ���� TIM2, TIM3 ,TIM4, TIM5, TIM6, TIM7, TIM12, TIM13, TIM14
	APB2 ��ʱ���� TIM1, TIM8 ,TIM9, TIM10, TIM11
*/
typedef struct
{
	TIM_TypeDef * 	sTimx;
	u8				ucchannel;
	GPIO_TypeDef * 	sGpiox;
	u16 			usGpioPinNum;
  int       running;
}StructTimx;

StructTimx gTimx[15] = {0};

/*
*********************************************************************************************************
*	�� �� ��: bsp_GetRCCofGPIO
*	����˵��: ����GPIO �õ�RCC�Ĵ���
*	��    �Σ���
*	�� �� ֵ: GPIO����ʱ����
*********************************************************************************************************
*/
uint32_t bsp_GetRCCofGPIO(GPIO_TypeDef* GPIOx)
{
	uint32_t rcc = 0;

	if (GPIOx == GPIOA)
	{
		rcc = RCC_AHB1Periph_GPIOA;
	}
	else if (GPIOx == GPIOB)
	{
		rcc = RCC_AHB1Periph_GPIOB;
	}
	else if (GPIOx == GPIOC)
	{
		rcc = RCC_AHB1Periph_GPIOC;
	}
	else if (GPIOx == GPIOD)
	{
		rcc = RCC_AHB1Periph_GPIOD;
	}
	else if (GPIOx == GPIOE)
	{
		rcc = RCC_AHB1Periph_GPIOE;
	}
	else if (GPIOx == GPIOF)
	{
		rcc = RCC_AHB1Periph_GPIOF;
	}
	else if (GPIOx == GPIOG)
	{
		rcc = RCC_AHB1Periph_GPIOG;
	}
	else if (GPIOx == GPIOH)
	{
		rcc = RCC_AHB1Periph_GPIOH;
	}
	else if (GPIOx == GPIOI)
	{
		rcc = RCC_AHB1Periph_GPIOI;
	}

	return rcc;
}


/*
*********************************************************************************************************
*	�� �� ��: bsp_GetPinSource
*	����˵��: ���� GPIO_Pin_X �õ� GPIO_PinSource
*	��    �Σ�gpio_pin
*	�� �� ֵ: GPIO_PinSource
*********************************************************************************************************
*/
uint16_t bsp_GetPinSource(uint16_t gpio_pin)
{
	uint16_t ret = 0;

	if (gpio_pin == GPIO_Pin_0)
	{
		ret = GPIO_PinSource0;
	}
	else if (gpio_pin == GPIO_Pin_1)
	{
		ret = GPIO_PinSource1;
	}
	else if (gpio_pin == GPIO_Pin_2)
	{
		ret = GPIO_PinSource2;
	}
	else if (gpio_pin == GPIO_Pin_3)
	{
		ret = GPIO_PinSource3;
	}
	else if (gpio_pin == GPIO_Pin_4)
	{
		ret = GPIO_PinSource4;
	}
	else if (gpio_pin == GPIO_Pin_5)
	{
		ret = GPIO_PinSource5;
	}
	else if (gpio_pin == GPIO_Pin_6)
	{
		ret = GPIO_PinSource6;
	}
	else if (gpio_pin == GPIO_Pin_7)
	{
		ret = GPIO_PinSource7;
	}
	else if (gpio_pin == GPIO_Pin_8)
	{
		ret = GPIO_PinSource8;
	}
	else if (gpio_pin == GPIO_Pin_9)
	{
		ret = GPIO_PinSource9;
	}
	else if (gpio_pin == GPIO_Pin_10)
	{
		ret = GPIO_PinSource10;
	}
	else if (gpio_pin == GPIO_Pin_11)
	{
		ret = GPIO_PinSource11;
	}
	else if (gpio_pin == GPIO_Pin_12)
	{
		ret = GPIO_PinSource12;
	}
	else if (gpio_pin == GPIO_Pin_13)
	{
		ret = GPIO_PinSource13;
	}
	else if (gpio_pin == GPIO_Pin_14)
	{
		ret = GPIO_PinSource14;
	}
	else if (gpio_pin == GPIO_Pin_15)
	{
		ret = GPIO_PinSource15;
	}

	return ret;
}


/*
*********************************************************************************************************
*	�� �� ��: bsp_GetRCCofTIM
*	����˵��: ����TIM �õ�RCC�Ĵ���
*	��    �Σ���
*	�� �� ֵ: TIM����ʱ����
*********************************************************************************************************
*/
uint32_t bsp_GetRCCofTIM(TIM_TypeDef* TIMx)
{
	uint32_t rcc = 0;

	/*
		APB1 ��ʱ���� TIM2, TIM3 ,TIM4, TIM5, TIM6, TIM7, TIM12, TIM13, TIM14
		APB2 ��ʱ���� TIM1, TIM8 ,TIM9, TIM10, TIM11
	*/
	if (TIMx == TIM1)
	{
		rcc = RCC_APB2Periph_TIM1;
	}
	else if (TIMx == TIM8)
	{
		rcc = RCC_APB2Periph_TIM8;
	}
	else if (TIMx == TIM9)
	{
		rcc = RCC_APB2Periph_TIM9;
	}
	else if (TIMx == TIM10)
	{
		rcc = RCC_APB2Periph_TIM10;
	}
	else if (TIMx == TIM11)
	{
		rcc = RCC_APB2Periph_TIM11;
	}
	/* ������ APB1ʱ�� */
	else if (TIMx == TIM2)
	{
		rcc = RCC_APB1Periph_TIM2;
	}
	else if (TIMx == TIM3)
	{
		rcc = RCC_APB1Periph_TIM3;
	}
	else if (TIMx == TIM4)
	{
		rcc = RCC_APB1Periph_TIM4;
	}
	else if (TIMx == TIM5)
	{
		rcc = RCC_APB1Periph_TIM5;
	}
	else if (TIMx == TIM6)
	{
		rcc = RCC_APB1Periph_TIM6;
	}
	else if (TIMx == TIM7)
	{
		rcc = RCC_APB1Periph_TIM7;
	}
	else if (TIMx == TIM12)
	{
		rcc = RCC_APB1Periph_TIM12;
	}
	else if (TIMx == TIM13)
	{
		rcc = RCC_APB1Periph_TIM13;
	}
	else if (TIMx == TIM14)
	{
		rcc = RCC_APB1Periph_TIM14;
	}

	return rcc;
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_GetAFofTIM
*	����˵��: ����TIM �õ�AF�Ĵ�������
*	��    ��: ��
*	�� �� ֵ: AF�Ĵ�������
*********************************************************************************************************
*/
uint8_t bsp_GetAFofTIM(TIM_TypeDef* TIMx)
{
	uint8_t ret = 0;

	/*
		APB1 ��ʱ���� TIM2, TIM3 ,TIM4, TIM5, TIM6, TIM7, TIM12, TIM13, TIM14
		APB2 ��ʱ���� TIM1, TIM8 ,TIM9, TIM10, TIM11
	*/
	if (TIMx == TIM1)
	{
		ret = GPIO_AF_TIM1;
	}
	else if (TIMx == TIM8)
	{
		ret = GPIO_AF_TIM8;
	}
	else if (TIMx == TIM9)
	{
		ret = GPIO_AF_TIM9;
	}
	else if (TIMx == TIM10)
	{
		ret = GPIO_AF_TIM10;
	}
	else if (TIMx == TIM11)
	{
		ret = GPIO_AF_TIM11;
	}
	/* ������ APB1ʱ�� */
	else if (TIMx == TIM2)
	{
		ret = GPIO_AF_TIM2;
	}
	else if (TIMx == TIM3)
	{
		ret = GPIO_AF_TIM3;
	}
	else if (TIMx == TIM4)
	{
		ret = GPIO_AF_TIM4;
	}
	else if (TIMx == TIM5)
	{
		ret = GPIO_AF_TIM5;
	}
	/* û�� TIM6 TIM7 */
	else if (TIMx == TIM8)
	{
		ret = GPIO_AF_TIM8;
	}
	else if (TIMx == TIM9)
	{
		ret = GPIO_AF_TIM9;
	}
	else if (TIMx == TIM12)
	{
		ret = GPIO_AF_TIM12;
	}
	else if (TIMx == TIM13)
	{
		ret = GPIO_AF_TIM13;
	}
	else if (TIMx == TIM14)
	{
		ret = GPIO_AF_TIM14;
	}

	return ret;
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_ConfigGpioOut
*	����˵��: ����GPIOΪ�����������Ҫ����PWM�����ռ�ձ�Ϊ0��100�������
*	��    ��: GPIOx
*			  GPIO_PinX
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_ConfigGpioOut(GPIO_TypeDef* GPIOx, uint16_t GPIO_PinX)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	/* ʹ��GPIOʱ�� */
	RCC_AHB1PeriphClockCmd(bsp_GetRCCofGPIO(GPIOx), ENABLE);

	/* ����GPIO */
	GPIO_InitStructure.GPIO_Pin = GPIO_PinX;		/* ������β� */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	/* ��� */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	/* ���� */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;	/* ������ */
	GPIO_Init(GPIOx, &GPIO_InitStructure);
}



/*
*********************************************************************************************************
*	�� �� ��: bsp_ConfigTimGpio
*	����˵��: ����GPIO��TIMʱ�ӣ� GPIO���ӵ�TIM���ͨ��
*	��    ��: GPIOx
*			 GPIO_PinX
*			 TIMx
*			 _ucChannel
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_ConfigTimGpio(GPIO_TypeDef* GPIOx, uint16_t GPIO_PinX, TIM_TypeDef* TIMx, uint8_t _ucChannel)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	/* ʹ��GPIOʱ�� */
	RCC_AHB1PeriphClockCmd(bsp_GetRCCofGPIO(GPIOx), ENABLE);

  	/* ʹ��TIMʱ�� */
	if ((TIMx == TIM1) || (TIMx == TIM8) || (TIMx == TIM9) || (TIMx == TIM10) || (TIMx == TIM11))
	{
		RCC_APB2PeriphClockCmd(bsp_GetRCCofTIM(TIMx), ENABLE);
	}
	else
	{
		RCC_APB1PeriphClockCmd(bsp_GetRCCofTIM(TIMx), ENABLE);
	}

	/* ����GPIO */
	GPIO_InitStructure.GPIO_Pin = GPIO_PinX;	/* ������β� */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
	GPIO_Init(GPIOx, &GPIO_InitStructure);

	/* ���ӵ�AF���� */
	GPIO_PinAFConfig(GPIOx, bsp_GetPinSource(GPIO_PinX), bsp_GetAFofTIM(TIMx));
}



/*
*********************************************************************************************************
*	�� �� ��: bsp_SetTIMOutPWM
*	����˵��: �������������PWM�źŵ�Ƶ�ʺ�ռ�ձ�.  ��Ƶ��Ϊ0������ռ��Ϊ0ʱ���رն�ʱ����GPIO���0��
*			  ��Ƶ��Ϊ0��ռ�ձ�Ϊ100%ʱ��GPIO���1.
*	��    ��: _ulFreq : PWM�ź�Ƶ�ʣ���λHz  (ʵ�ʲ��ԣ�������Ƶ��Ϊ 168M / 4 = 42M��. 0 ��ʾ��ֹ���
*			  _ulDutyCycle : PWM�ź�ռ�ձȣ���λ�����֮һ����5000����ʾ50.00%��ռ�ձ�
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitPWM(GPIO_TypeDef* GPIOx, u16 GPIO_Pin, u8 _tim_num, u8 _ucChannel)
{
	gTimx[_tim_num].sGpiox = GPIOx;
	if(1 == _tim_num) 
	{
		gTimx[_tim_num].sTimx = TIM1;
		TIM_DeInit(TIM1);
	}
	else if(2 == _tim_num)
	{
		gTimx[_tim_num].sTimx = TIM2;
		TIM_DeInit(TIM2);
	}
	else if(3 == _tim_num)
	{
		gTimx[_tim_num].sTimx = TIM3;
		TIM_DeInit(TIM3);
	}
	else if(4 == _tim_num)
	{
		gTimx[_tim_num].sTimx = TIM4;
		TIM_DeInit(TIM4);
	}
	else if(5 == _tim_num)
	{
		gTimx[_tim_num].sTimx = TIM5;
		TIM_DeInit(TIM5);
	}
	else if(6 == _tim_num)
	{
		gTimx[_tim_num].sTimx = TIM6;
		TIM_DeInit(TIM6);
	}
	else if(7 == _tim_num)
	{
		gTimx[_tim_num].sTimx = TIM7;
		TIM_DeInit(TIM7);
	}
	else if(8 == _tim_num)
	{
		gTimx[_tim_num].sTimx = TIM8;
		TIM_DeInit(TIM8);
	}
	else if(9 == _tim_num)
	{
		gTimx[_tim_num].sTimx = TIM9;
		TIM_DeInit(TIM9);
	}
	else if(10 == _tim_num)
	{
		gTimx[_tim_num].sTimx = TIM10;
		TIM_DeInit(TIM10);
	}
	else if(11 == _tim_num)
	{
		gTimx[_tim_num].sTimx = TIM11;
		TIM_DeInit(TIM11);
	}
	else if(12 == _tim_num)
	{
		gTimx[_tim_num].sTimx = TIM12;
		TIM_DeInit(TIM12);
	}
	else if(13 == _tim_num)
	{
		gTimx[_tim_num].sTimx = TIM13;
		TIM_DeInit(TIM13);
	}
	else
	{
		gTimx[_tim_num].sTimx = TIM14;
		TIM_DeInit(TIM14);
	}
	gTimx[_tim_num].ucchannel = _ucChannel;
	gTimx[_tim_num].usGpioPinNum = GPIO_Pin;

  gTimx[_tim_num].running = 0;

	bsp_ConfigTimGpio(GPIOx, GPIO_Pin, gTimx[_tim_num].sTimx, _ucChannel);	/* ʹ��GPIO��TIMʱ�ӣ�������TIMͨ����GPIO */

}

int bsp_PwmFreDutySet(u8 _tim_num, u32 _ulFreq, u32 _ulDutyCycle)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	u16 usPeriod;
	u16 usPrescaler;
	u32 uiTIMxCLK;

/******************************************************************
	//����֮ǰ����Ҫ��ֹ�����¼���������ֹ���ù��̲��������¼����󴥷�DMA��
	//������DMA����PWM����ʱ����һ������;
**************************************************************/
//	TIM_UpdateDisableConfig(gTimx[_tim_num].sTimx,ENABLE);
	if (_ulDutyCycle == 0)
	{		
		TIM_Cmd(gTimx[_tim_num].sTimx, DISABLE); 	/* �ر�PWM��� */
		return -1;
	}
	else if (_ulDutyCycle == 10000)
	{
		TIM_Cmd(gTimx[_tim_num].sTimx, DISABLE); 	/* �ر�PWM��� */
		return -1;
	}

	
	if ((gTimx[_tim_num].sTimx == TIM1) || (gTimx[_tim_num].sTimx == TIM8) || \
		(gTimx[_tim_num].sTimx == TIM9) || (gTimx[_tim_num].sTimx == TIM10) || \
		(gTimx[_tim_num].sTimx == TIM11))
	{
		/* APB2 ��ʱ�� */
		uiTIMxCLK = SystemCoreClock;
	}
	else	/* APB1 ��ʱ�� */
	{
		uiTIMxCLK = SystemCoreClock / 2;
	}

	if (_ulFreq < 100)
	{
		usPrescaler = 10000 - 1;					/* ��Ƶ�� = 10000 */
		usPeriod =	(uiTIMxCLK / 10000) / _ulFreq  - 1; 	/* �Զ���װ��ֵ */
	}
	else if (_ulFreq < 3000)
	{
		usPrescaler = 100 - 1;					/* ��Ƶ�� = 100 */
		usPeriod =	(uiTIMxCLK / 100) / _ulFreq  - 1;		/* �Զ���װ��ֵ */
	}
	else	/* ����4K��Ƶ�ʣ������Ƶ */
	{
		usPrescaler = 0;					/* ��Ƶ�� = 1 */
		usPeriod = uiTIMxCLK / _ulFreq - 1; /* �Զ���װ��ֵ */
	}
	TIM_TimeBaseStructure.TIM_Period = usPeriod;
	TIM_TimeBaseStructure.TIM_Prescaler = usPrescaler;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

	TIM_TimeBaseInit(gTimx[_tim_num].sTimx, &TIM_TimeBaseStructure);

	/* PWM1 Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
	TIM_OCInitStructure.TIM_Pulse = (_ulDutyCycle * usPeriod) / 10000;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
 
	if (gTimx[_tim_num].ucchannel == 1)
	{
		TIM_OC1Init(gTimx[_tim_num].sTimx, &TIM_OCInitStructure);
		TIM_OC1PreloadConfig(gTimx[_tim_num].sTimx, TIM_OCPreload_Enable);
	}
	else if (gTimx[_tim_num].ucchannel == 2)
	{
		TIM_OC2Init(gTimx[_tim_num].sTimx, &TIM_OCInitStructure);
		TIM_OC2PreloadConfig(gTimx[_tim_num].sTimx, TIM_OCPreload_Enable);
	}
	else if (gTimx[_tim_num].ucchannel == 3)
	{
		TIM_OC3Init(gTimx[_tim_num].sTimx, &TIM_OCInitStructure);
		TIM_OC3PreloadConfig(gTimx[_tim_num].sTimx, TIM_OCPreload_Enable);
	}
	else if (gTimx[_tim_num].ucchannel == 4)
	{
		TIM_OC4Init(gTimx[_tim_num].sTimx, &TIM_OCInitStructure);
		TIM_OC4PreloadConfig(gTimx[_tim_num].sTimx, TIM_OCPreload_Enable);
	}
	TIM_ARRPreloadConfig(gTimx[_tim_num].sTimx, ENABLE);
	/********************�ָ����������¼�***********************/
//	TIM_UpdateDisableConfig(gTimx[_tim_num].sTimx,DISABLE);
	return 0;
}

//ָ����ַ����ֹ���䵽DMA�޷����ʵ��ĵ�ַ���������ߴ���
u32 pwm_tstbuf[1] __attribute__((at(0x2001fff0)));
int bsp_PwmNumCtrl(u8 _pwm_channel, u16 _pwm_num)
{
//PWM2��������
  	DMA_InitTypeDef  DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_Stream_TypeDef* dma_stream;
	TIM_TypeDef * tim_num;
	u32 Rcc_Source, dma_chanel, dma_pend_bit;
	u8 dma_stream_irq, irq_pre_prio, irq_sub_prio;

  	/* ���� DMA2�� */
	if(_pwm_channel == 1)
	{
		Rcc_Source = RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_DMA2D;
		tim_num = TIM1;
		dma_stream = DMA2_Stream5;
		dma_chanel = DMA_Channel_6;
		dma_stream_irq = DMA2_Stream5_IRQn;
		irq_pre_prio = 0;
		irq_sub_prio = 0;
		dma_pend_bit = DMA_IT_TCIF5;
		
	}
	else if(_pwm_channel == 2)
	{
		Rcc_Source = RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_DMA2D;
		tim_num = TIM8;
		dma_stream = DMA2_Stream1;
		dma_pend_bit = DMA_IT_TCIF1;
		dma_chanel = DMA_Channel_7;
		dma_stream_irq = DMA2_Stream1_IRQn;
		irq_pre_prio = 0;
		irq_sub_prio = 1;
	}
	else
	{
		Rcc_Source = RCC_AHB1Periph_DMA1;
		tim_num = TIM2;
		dma_stream = DMA1_Stream1;
		dma_pend_bit = DMA_IT_TCIF1;
		dma_chanel = DMA_Channel_3;
		dma_stream_irq = DMA1_Stream1_IRQn;
		irq_pre_prio = 0;
		irq_sub_prio = 2;
	}

	RCC_AHB1PeriphClockCmd(Rcc_Source, ENABLE);

	TIM_DMACmd(tim_num, TIM_DMA_Update, ENABLE);

	
  	DMA_DeInit(dma_stream);
  	DMA_InitStructure.DMA_Channel = dma_chanel;
  	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&tim_num->ARR);		
  	DMA_InitStructure.DMA_Memory0BaseAddr = (u32)(&pwm_tstbuf[0]);			
  	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
//  	DMA_InitStructure.DMA_BufferSize = 1;
	DMA_InitStructure.DMA_BufferSize = _pwm_num + 1;
  	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
  	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  	//DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  	//DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  	DMA_Init(dma_stream, &DMA_InitStructure);

	/* Enable DMA Stream Transfer Complete interrupt */
	DMA_ClearITPendingBit(dma_stream, dma_pend_bit);
	DMA_ITConfig(dma_stream, DMA_IT_TC, ENABLE);
 
  	DMA_Cmd(dma_stream, ENABLE);

	/* Enable the DMA Stream IRQ Channel */
	NVIC_InitStructure.NVIC_IRQChannel = dma_stream_irq;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = irq_pre_prio;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = irq_sub_prio;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

  	DCMI_Cmd(ENABLE);
  	DCMI_CaptureCmd(ENABLE);

	return 0;
	
}

int bsp_PwmStart(u8 _tim_num)
{
	gTimx[_tim_num].sTimx->CNT = 0;
//	gTimx[_tim_num].sTimx->ARR = 0;
  gTimx[_tim_num].running = 1;
	TIM_Cmd(gTimx[_tim_num].sTimx, ENABLE);	

	if ((gTimx[_tim_num].sTimx == TIM1) || (gTimx[_tim_num].sTimx == TIM8))
	{
		TIM_CtrlPWMOutputs(gTimx[_tim_num].sTimx, ENABLE);
	}
	return 0;
}

int bsp_PwmStop(u8 _tim_num)
{
	TIM_Cmd(gTimx[_tim_num].sTimx, DISABLE);
  gTimx[_tim_num].running = 0;
	gTimx[_tim_num].sTimx->CNT = 0;
	gTimx[_tim_num].sTimx->ARR = 0;
	return 0;
}
volatile uint32_t  ulPwmInterNum[5] = {0};

int bsp_PwmSet(u8 _tim_num, u32 _ulFreq, u32 _pwm_num)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
		/* Enable the USARTx Interrupt */
	 
//	TIM11->DIER |= 1;


	NVIC_InitStructure.NVIC_IRQChannel = TIM1_TRG_COM_TIM11_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 6;		  //�����ȼ�Ϊ0(��ֵԽС,���ȼ�Խ��,0��������ȼ�)
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	ulPwmInterNum[_tim_num] = _pwm_num;
	
	BSP_STATPWM(_tim_num);
	
	TIM_ClearFlag(TIM11,TIM_FLAG_Update);       //������±�־
	TIM_ITConfig(TIM11,TIM_IT_Update,ENABLE);
 	BSP_SETPWM(_tim_num, _ulFreq);
	
	return 0;
}

volatile uint32_t pwm1internum = 0;

void TIM1_TRG_COM_TIM11_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM11, TIM_FLAG_Update) == SET)
	{
		TIM_ClearFlag(TIM11, TIM_FLAG_Update);
		
		if(pwm1internum++ >= ulPwmInterNum[1])
		{
			BSP_STOPPWM(1);
			ulPwmInterNum[1] = 0;
			pwm1internum = 0;
			
		}
	}
}

int bsp_PwmIsRunning(u8 _tim_num)
{
    return gTimx[_tim_num].running;
}