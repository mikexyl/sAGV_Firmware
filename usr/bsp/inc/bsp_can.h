#ifndef _BSP_CAN_H_
#define _BSP_CAN_H_

#include "stm32f4xx.h"
#include "bsp.h"
#include "string.h"

int bsp_InitCan(CAN_TypeDef* _CANx, u16 _uscbaudrata);
int bsp_CanReceive(CAN_TypeDef* _CANx, CanRxMsg* RxMessage);
int bsp_CanSend(CAN_TypeDef* _CANx, CanTxMsg* _pRxMessag);

#endif

