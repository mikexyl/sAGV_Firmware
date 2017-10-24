/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    22-March-2012
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"

/** @addtogroup Fractal_Project
  * @{
  */

/** @addtogroup Fractal_Project_IT
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {}
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {}
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {}
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {}
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
//void SVC_Handler(void)    //屏蔽20141118
//{}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
/*
void PendSV_Handler(void)
{}
*/
/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
/*
void SysTick_Handler(void)
{
}
*/
/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/
/*******************************************************************************
* 描    述	: TIM4中断请求
* 输入参数  : 无
* 返回参数  : 无
* 100ms中断1次
*******************************************************************************/
void TIM4_IRQHandler_M6(void)
{
}
/*******************************************************************************
* 描    述	: TIM3中断请求
* 输入参数  : 无
* 返回参数  : 无
* 10ms中断1次
*******************************************************************************/
void TIM3_IRQHandler_M3(void)
{
}
#if 0
/*******************************************************************************
* 描    述	: TIM3中断请求
* 输入参数  : 无
* 返回参数  : 无
* 10ms中断1次
*******************************************************************************/
void TIM3_IRQHandler(void)
{
}
/*******************************************************************************
* 描    述	: TIM4中断请求
* 输入参数  : 无
* 返回参数  : 无
* 100ms中断1次
*editor-20160413-z
*******************************************************************************/
void TIM4_IRQHandler(void)
{
}
//USART irq handle will be defined in the uart driver

/*******************************************************************************
* 描    述	: USART1中断请求
* 输入参数  : 无
* 返回参数  : 无
*******************************************************************************/
void USART1_IRQHandler(void)
{
}
/*******************************************************************************
* 描    述	: USART2中断请求
* 输入参数  : 无
* 返回参数  : 无
*******************************************************************************/
void USART2_IRQHandler(void)
{
}
/*******************************************************************************
* 描    述	: USART3中断请求
* 输入参数  : 无
* 返回参数  : 无
*******************************************************************************/
void USART3_IRQHandler(void)
{
}
/*******************************************************************************
* 描    述	: UART4中断请求
* 输入参数  : 无
* 返回参数  : 无
*******************************************************************************/
void UART4_IRQHandler(void)
{
}
/*******************************************************************************
* 描    述	: UART5中断请求
* 输入参数  : 无
* 返回参数  : 无
*******************************************************************************/
void UART5_IRQHandler(void)
{
 
}
/*******************************************************************************
* 描    述	: UART6中断请求
* 输入参数  : 无
* 返回参数  : 无
*******************************************************************************/
void USART6_IRQHandler(void)
{
  
}
#endif
/**
  * @brief  This function handles External IT 0 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI0_IRQHandler(void)
{

}
/*******************************************************************************
* 描    述	: EXTI1中断请求
* 输入参数  : 无
* 返回参数  : 无
*******************************************************************************/
void EXTI1_IRQHandler (void)
{

}
/*******************************************************************************
* 描    述	: EXTI2中断请求
* 输入参数  : 无
* 返回参数  : 无
*******************************************************************************/
void EXTI2_IRQHandler(void)
{

}
/*******************************************************************************
* 描    述	: EXTI3中断请求
* 输入参数  : 无
* 返回参数  : 无
*******************************************************************************/
void EXTI3_IRQHandler(void)
{

}
/*******************************************************************************
* 描    述	: EXTI4中断请求
* 输入参数  : 无
* 返回参数  : 无
*******************************************************************************/
void EXTI4_IRQHandler(void)
{

}
//EXTI9_5_IRQHandler
/*******************************************************************************
* 描    述	: EXTI6/EXTI5中断请求
* 输入参数  : 无
* 返回参数  : 无
*******************************************************************************/
#if 0
void EXTI9_5_IRQHandler(void)
{

}
#endif
/*******************************************************************************
* 描    述	: EXTI15中断请求
* 输入参数  : 无
* 返回参数  : 无
*******************************************************************************/
void EXTI15_10_IRQHandler(void)
{
 

}
#if 0
/*******************************************************************************
* 描    述	: CAN1_RX0中断请求
* 输入参数  : 无
* 返回参数  : 无
*******************************************************************************/
void CAN1_RX0_IRQHandler(void)
{

}
/*******************************************************************************
* 描    述	: CAN1_TX中断请求
* 输入参数  : 无
* 返回参数  : 无
*******************************************************************************/
void CAN1_TX_IRQHandler(void)
{
 
}
#endif
/******************* (C) COPYRIGHT 2014 Hollysys *****END OF FILE*************/
