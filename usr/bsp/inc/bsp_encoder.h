#ifndef _BSP_ENCODER_H_
#define	_BSP_ENCODER_H_

#include "stm32f4xx.h"

/*****************************************************************
encoder1: postive: TIM3_CH1-PA6, negative: TIM3_CH2-PA7
encoder2: postive: TIM4_CH1-PD12, negative: TIM4_CH2-PD13
*******************************************************************/
typedef struct
{
	void (*fun)(uint8_t num, uint16_t  cnt, uint8_t dir);
}StructEncoder;

extern StructEncoder gsEncoder[2];


int bsp_InitEncoder(GPIO_TypeDef* PositiveGPIOx, uint16_t PositiveGPIO_PinX, 
						GPIO_TypeDef* NegativeGPIOx, uint16_t NegativeGPIO_PinX, 		
						TIM_TypeDef* TIMx, uint8_t PositiveChannel, uint8_t NegativeChannel);

int bsp_EncoderGetPulseNum(u8 _encodernum, u16* _pulnum, u8* _dir);
u16 bsp_EncoderSetZero(u8 _encodernum);

#define	BSP_INITENCODER(_encodernum,  overflow_callback)	do{\
	if(1 == _encodernum)	{\
			bsp_InitEncoder(GPIOA, GPIO_Pin_6, GPIOA, GPIO_Pin_7, TIM3, 1, 2);\
			gsEncoder[0].fun = overflow_callback;\
		}\
	else if(2 == _encodernum)	{\
			bsp_InitEncoder(GPIOD, GPIO_Pin_12, GPIOD, GPIO_Pin_13, TIM4, 1, 2);\
			gsEncoder[1].fun = overflow_callback;\
		}\
}while(0)

#if 0
	else if(3 == _encodernum)	{\
			bsp_InitEncoder(GPIOD, GPIO_Pin_12, GPIOD, GPIO_Pin_13, TIM4, 1, 2);\
			gsEncoder[2].fun = overflow_callback;\
		}\
	else		{\
			bsp_InitEncoder(GPIOA, GPIO_Pin_0, GPIOA, GPIO_Pin_1, TIM5, 1, 2);\
			gsEncoder[3].fun = overflow_callback;\
		}
#endif

#endif
