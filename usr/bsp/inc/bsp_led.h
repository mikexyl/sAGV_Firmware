#ifndef _BSP_LED_H_
#define _BSP_LED_H_

#include "stm32f4xx.h"
#include "bsp.h"

void bsp_InitLed(void);
void bsp_LedOn(void);
void bsp_LedOff(void);
void bsp_LedToggle(void);
#endif

