#include "bsp_led.h"

#define LED1_PORT   GPIOB
#define LED1_PIN    GPIO_Pin_8
#define LED1_RCC    RCC_AHB1Periph_GPIOB

#define LED1_ON()   GPIO_SetBits(LED1_PORT, LED1_PIN);
#define LED1_OFF()  GPIO_ResetBits(LED1_PORT, LED1_PIN);


void bsp_InitLed(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  //RCC_AHB1PeriphClockCmd(LED1_RCC, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

	GPIO_InitStructure.GPIO_Pin = LED1_PIN;
	GPIO_Init(LED1_PORT, &GPIO_InitStructure);

#if 0
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_MCO);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, GPIO_AF_MCO);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	RCC_MCO1Config(RCC_MCO1Source_PLLCLK, RCC_MCO1Div_4);
	RCC_MCO2Config(RCC_MCO2Source_SYSCLK, RCC_MCO2Div_4);
#endif
}

void bsp_LedOn(void)
{
	LED1_ON();
}

void bsp_LedOff(void)
{
	LED1_OFF();
}

void bsp_LedToggle(void)
{
    GPIO_ToggleBits(LED1_PORT, LED1_PIN);
}

