#ifndef _BSP_PWM_H_
#define	_BSP_PWM_H_
#include "stdint.h"
#include "stm32f4xx.h"
#include "bsp.h"

void bsp_InitPWM(GPIO_TypeDef* GPIOx, u16 GPIO_Pin, u8 _tim_num, u8 _ucChannel);
int bsp_PwmStart(u8 _tim_num);

int bsp_PwmFreDutySet(u8 _tim_num, u32 _ulFreq, u32 _ulDutyCycle);
int bsp_PwmStop(u8 _tim_num);
int bsp_PwmIsRunning(u8 _tim_num);

#define PWM1_PORT   1
#define PWM2_PORT   8
#define PWM3_PORT   2

/*******************************************************************
PWM1: TIM11_CH1, PB9
PWM2: TIM13_CH1, PF8
PWM3: TIM9_CH1, PE5
PWM4: TIM14_CH1, PF9

*******************************************************************/

#define	BSP_INITPWM(_pwmnum)	do{\
			if(1 == _pwmnum)		bsp_InitPWM(GPIOE, GPIO_Pin_9, PWM1_PORT, 1);\
			else if(2 == _pwmnum)	bsp_InitPWM(GPIOC, GPIO_Pin_6, PWM2_PORT, 1);\
			else if(3 == _pwmnum)	bsp_InitPWM(GPIOB, GPIO_Pin_3, PWM3_PORT, 2);\
	}while(0)
			//else					bsp_InitPWM(GPIOF, GPIO_Pin_9, PWM4_PORT, 1);\

#define	BSP_STATPWM(_pwmnum)	do{\
			if(1 == _pwmnum)		bsp_PwmStart(PWM1_PORT);\
			else if(2 == _pwmnum)	bsp_PwmStart(PWM2_PORT);\
			else if(3 == _pwmnum)	bsp_PwmStart(PWM3_PORT);\
	}while(0)
			//else					bsp_PwmStart(PWM4_PORT);\

#define	BSP_STOPPWM(_pwmnum)	do{\
						if(1 == _pwmnum)		bsp_PwmStop(PWM1_PORT);\
						else if(2 == _pwmnum)	bsp_PwmStop(PWM2_PORT);\
						else if(3 == _pwmnum)	bsp_PwmStop(PWM3_PORT);\
				}while(0)
						//else					bsp_PwmStop(PWM4_PORT);\

#define	BSP_SETPWM(_pwmnum, _ulFreq)	do{\
						if(1 == _pwmnum)		bsp_PwmFreDutySet(PWM1_PORT ,_ulFreq, 5000);\
						else if(2 == _pwmnum)	bsp_PwmFreDutySet(PWM2_PORT ,_ulFreq, 5000);\
						else if(3 == _pwmnum)	bsp_PwmFreDutySet(PWM3_PORT ,_ulFreq, 5000);\
				}while(0)
						//else					bsp_PwmFreDutySet(PWM4_PORT ,_ulFreq, 5000);\

#define BSP_PWM_ISRUNNING(_pwmnum) do{\
						if(1 == _pwmnum)		bsp_PwmIsRunning(PWM1_PORT);\
						else if(2 == _pwmnum)	bsp_PwmIsRunning(PWM2_PORT);\
						else if(3 == _pwmnum)	bsp_PwmIsRunning(PWM3_PORT);\
				}while(0)
						//else					bsp_PwmIsRunning(PWM4_PORT);\

#endif
