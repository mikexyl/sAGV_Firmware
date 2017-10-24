#include "bsp_encoder.h"
#include "bsp_pwm.h"
#include "stm32f4xx.h"

#define ENCODER_ARR		0xffff
/*****************************************************************
encoder1: postive: TIM1_CH1-PE9, negative: TIM1_CH2-PE11
encoder2: postive: TIM3_CH1-PA6, negative: TIM2_CH2-PA7
encoder3: postive: TIM4_CH1-PD12, negative :TIM4_CH2-PD13
encoder4: postive: TIM5_CH1-PA0, negative: TIM5_CH2-PA1
*******************************************************************/

StructEncoder gsEncoder[2] = {0};

extern uint32_t bsp_GetRCCofTIM(TIM_TypeDef* TIMx);
extern uint16_t bsp_GetPinSource(uint16_t gpio_pin);
extern uint32_t bsp_GetRCCofGPIO(GPIO_TypeDef* GPIOx);
extern uint8_t bsp_GetAFofTIM(TIM_TypeDef* TIMx);

int bsp_EncoderGPIOTimConfig(GPIO_TypeDef* PositiveGPIOx, uint16_t PositiveGPIO_PinX, 
						GPIO_TypeDef* NegativeGPIOx, uint16_t NegativeGPIO_PinX, 		
						TIM_TypeDef* TIMx, uint8_t PositiveChannel, uint8_t NegativeChannel)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	/* 使能GPIO时钟 */
	RCC_AHB1PeriphClockCmd(bsp_GetRCCofGPIO(PositiveGPIOx), ENABLE);
	RCC_AHB1PeriphClockCmd(bsp_GetRCCofGPIO(NegativeGPIOx), ENABLE);

	/* 使能TIM时钟 */
	if ((TIMx == TIM1) || (TIMx == TIM8) || (TIMx == TIM9) || (TIMx == TIM10) || (TIMx == TIM11))
	{
		RCC_APB2PeriphClockCmd(bsp_GetRCCofTIM(TIMx), ENABLE);
	}
	else
	{
		RCC_APB1PeriphClockCmd(bsp_GetRCCofTIM(TIMx), ENABLE);
	}

	/* 配置GPIO */
	GPIO_InitStructure.GPIO_Pin = PositiveGPIO_PinX;	/* 带入的形参 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
	GPIO_Init(PositiveGPIOx, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = NegativeGPIO_PinX;	/* 带入的形参 */
	GPIO_Init(NegativeGPIOx, &GPIO_InitStructure);

	/* 连接到AF功能 */
	GPIO_PinAFConfig(PositiveGPIOx, bsp_GetPinSource(PositiveGPIO_PinX), bsp_GetAFofTIM(TIMx));
	GPIO_PinAFConfig(NegativeGPIOx, bsp_GetPinSource(NegativeGPIO_PinX), bsp_GetAFofTIM(TIMx));
	return 0;
}

int bsp_InitEncoder(GPIO_TypeDef* PositiveGPIOx, uint16_t PositiveGPIO_PinX, 
						GPIO_TypeDef* NegativeGPIOx, uint16_t NegativeGPIO_PinX, 		
						TIM_TypeDef* TIMx, uint8_t PositiveChannel, uint8_t NegativeChannel)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_ICInitTypeDef TIM_ICInitStructure;

	NVIC_InitTypeDef NVIC_InitStructure;
  
	bsp_EncoderGPIOTimConfig(PositiveGPIOx, PositiveGPIO_PinX, NegativeGPIOx, NegativeGPIO_PinX, 
		TIMx, PositiveChannel, NegativeChannel);	/* 使能GPIO和TIM时钟，并连接TIM通道到GPIO */

	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Prescaler = 0; //No prescal
	TIM_TimeBaseStructure.TIM_Period = ENCODER_ARR;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIMx, &TIM_TimeBaseStructure);

	TIM_EncoderInterfaceConfig(TIMx, TIM_EncoderMode_TI12, 
					TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
	TIM_ICStructInit(&TIM_ICInitStructure);
	TIM_ICInitStructure.TIM_ICFilter = 0;
	TIM_ICInit(TIMx, &TIM_ICInitStructure);
	if(TIM1 == TIMx)
	{
		NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_TIM10_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	}
	else if(TIM3 == TIMx)
	{
		NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
	}
	else if(TIM4 == TIMx)
	{
		NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
	}
	else//tim5
	{
		NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 6;
	}	
	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIMx->CNT = 0;

	if(TIM1 == TIMx)
	{
		TIM_ClearFlag(TIM1, TIM_FLAG_Update);
		TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
	}
	else if(TIM3 == TIMx)
	{
		TIM_ClearFlag(TIM3, TIM_FLAG_Update);
		TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	}
	else if(TIM4 == TIMx)
	{
		TIM_ClearFlag(TIM4, TIM_FLAG_Update);
		TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
	}
	else//tim5
	{
		TIM_ClearFlag(TIM5, TIM_FLAG_Update);
		TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
	}	
	

	TIM_Cmd(TIMx, ENABLE); 
	return 0;
}


int bsp_EncoderGetPulseNum(u8 _encodernum, u16* _pulnum, u8* _dir)
{
	TIM_TypeDef* TIMx;
	if(1 == _encodernum)
	{
		TIMx = TIM3;
	}
	else if(2 == _encodernum)
	{
		TIMx = TIM4;
	}
#if 0
	else if(3 == _encodernum)
	{
		TIMx = TIM4;
	}
	else //4号编码器
	{
		TIMx = TIM5;
	}
#endif
	*_pulnum = TIMx->CNT;
	*_dir = ((TIMx->CR1 >> 4) & 1);
	return 0;
}


u16 bsp_EncoderSetZero(u8 _encodernum)
{
	u16 pulscnt = 0;
	TIM_TypeDef* TIMx;
	if(1 == _encodernum)
	{
		TIMx = TIM3;
	}
	else if(2 == _encodernum)
	{
		TIMx = TIM4;
	}
#if 0
	else if(3 == _encodernum)
	{
		TIMx = TIM4;
	}
	else //4号编码器
	{
		TIMx = TIM5;
	}
#endif
	pulscnt = TIMx->CNT;
	TIMx->CNT = 0;
	return pulscnt;
}



#if 0
void TIM1_UP_TIM10_IRQHandler(void)
{
	u16 pulcnt = 0;
	u8 motordir = 0;
	
	if(TIM_GetITStatus(TIM1,TIM_FLAG_Update) == SET)
	{
		TIM_ClearFlag(TIM1,TIM_FLAG_Update);
		motordir = (TIM1->CR1>>4)&1;
		pulcnt = TIM1->CNT;	
		if(gsEncoder[0].fun != NULL)
			gsEncoder[0].fun(1, pulcnt, motordir);
	}
	
}
#endif

void TIM3_IRQHandler(void)
{
	u16 pulcnt = 0;
	u8 motordir = 0;

	if(TIM_GetITStatus(TIM3,TIM_FLAG_Update) == SET)
	{
		TIM_ClearFlag(TIM3,TIM_FLAG_Update);
		motordir = (TIM3->CR1>>4)&1;
		pulcnt = TIM3->CNT;
		if(gsEncoder[0].fun != NULL)
			gsEncoder[0].fun(1, pulcnt, motordir);
	}

}

void TIM4_IRQHandler(void)
{
	u16 pulcnt = 0;
	u8 motordir = 0;

	if(TIM_GetITStatus(TIM4,TIM_FLAG_Update) == SET)
	{
		TIM_ClearFlag(TIM4,TIM_FLAG_Update);
		motordir = (TIM4->CR1>>4)&1;
		pulcnt = TIM4->CNT;	
		if(gsEncoder[1].fun != NULL)
			gsEncoder[1].fun(2, pulcnt, motordir);
	}

}

#if 0
void TIM5_IRQHandler(void)
{
	u8 encodernum = 0;
	u16 pulcnt = 0;
	u8 motordir = 0;
	
	if(TIM_GetITStatus(TIM5,TIM_FLAG_Update) == SET)
	{
		TIM_ClearFlag(TIM5,TIM_FLAG_Update);
		motordir = (TIM5->CR1>>4)&1;
		pulcnt = TIM5->CNT;	
		encodernum = 4;
		if(gsEncoder[3].fun != NULL)
			gsEncoder[3].fun(encodernum, pulcnt, motordir);
	}
	
}
#endif
