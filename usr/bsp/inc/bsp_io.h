#ifndef _BSP_IO_H_
#define	_BSP_IO_H_
#include <ucos_ii.h>

#include "stm32f4xx.h"

// DO1 ~ DO8
#define GPIO_DO01					GPIOF
#define PIN_DO01					GPIO_Pin_6
#define GPIO_DO02					GPIOA
#define PIN_DO02					GPIO_Pin_3
#define GPIO_DO03					GPIOE
#define PIN_DO03					GPIO_Pin_11
#define GPIO_DO04					GPIOE
#define PIN_DO04					GPIO_Pin_12
#define GPIO_DO05					GPIOF
#define PIN_DO05					GPIO_Pin_5
#define GPIO_DO06					GPIOF
#define PIN_DO06					GPIO_Pin_4
#define GPIO_DO07					GPIOF
#define PIN_DO07					GPIO_Pin_3
#define GPIO_DO08					GPIOF
#define PIN_DO08					GPIO_Pin_2

// DI1 ~ DI8
#define GPIO_DI01					GPIOE
#define PIN_DI01					GPIO_Pin_5
#define GPIO_DI02					GPIOE
#define PIN_DI02					GPIO_Pin_4
#define GPIO_DI03					GPIOE
#define PIN_DI03					GPIO_Pin_3
#define GPIO_DI04					GPIOE
#define PIN_DI04					GPIO_Pin_2
#define GPIO_DI05					GPIOF
#define PIN_DI05					GPIO_Pin_7
#define GPIO_DI06					GPIOB
#define PIN_DI06					GPIO_Pin_1
#define GPIO_DI07					GPIOE
#define PIN_DI07					GPIO_Pin_13
#define GPIO_DI08					GPIOE
#define PIN_DI08					GPIO_Pin_14

int bsp_InitIO(void);
int bsp_DoSet(u8 ucdonum,u8 uclevel);
int bsp_DiGet(u8 ucdinum);

#endif
