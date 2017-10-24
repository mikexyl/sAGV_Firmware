/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                          (c) Copyright 2003-2013; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/
#ifndef  __APP_CFG_H__
#define  __APP_CFG_H__

#include "common/log_print.h"

#define MS_TO_TICKS(ms) ((ms)*OS_TICKS_PER_SEC/1000)
#define TICKS_TO_MS(tk) ((tk)*1000/OS_TICKS_PER_SEC)

/*
*********************************************************************************************************
*                                            TASK PRIORITIES
*********************************************************************************************************
*/
/* Æô¶¯ÈÎÎñ */
#define  ROOTTASK_PRIO                               18


/*Application task priorities define*/
#define MOTIONCTRL_TASK_PRIOR   10
#define CAN_TASK_PRIOR          11
#define WIFIDRIVER_TASK_PRIOR   9
#define AGVCP_TASK_PRIOR        13
#define AGV_TASK_PRIOR          14
#define BMS_TASK_PRIOR          19

#define LOG_TASK_PRIOR              22
#define UART_CONSOLE_TASK_PRIOR     8
#define UDP_CONSOLE_TASK_PRIOR      23

/*
*********************************************************************************************************
*                                            TASK STACK SIZES
*                             Size of the task stacks (# of OS_STK entries)
*********************************************************************************************************
*/
#define  ROOTTASK_STK_SIZE                          512
//#define  OS_CPU_EXCEPT_STK_SIZE                   1024

/*
*********************************************************************************************************
*                                     TRACE / DEBUG CONFIGURATION
*********************************************************************************************************
*/
#define  APP_TRACE_LEVEL                			TRACE_LEVEL_INFO
#define  APP_TRACE                      			log_printf
//#define  APP_TRACE                      			printf

#define  APP_TRACE_INFO(x)            ((APP_TRACE_LEVEL >= TRACE_LEVEL_INFO)  ? (void)(APP_TRACE x) : (void)0)
#define  APP_TRACE_DEBUG(x)           ((APP_TRACE_LEVEL >= TRACE_LEVEL_DBG) ? (void)(APP_TRACE x) : (void)0)

//#define MAKE_IP(h,mh,ml,l)  ((((uint32_t)(h))<<24)|(((uint32_t)(mh))<<16)|(((uint32_t)(ml))<<8)|(l))
#define MAKE_IP(h,mh,ml,l)  ((((uint32_t)(l))<<24)|(((uint32_t)(ml))<<16)|(((uint32_t)(mh))<<8)|(h))

#define SW_VERSION          "0.6"

extern  int32_t agv_id;

extern  int32_t controller_ip;
extern  int32_t controller_port;

extern  char wifi_ssid[32];
extern  char wifi_password[32];
extern  int32_t local_ip;
extern  int32_t subnet_mask;
extern  int32_t gateway;
extern  int32_t local_port;

/*
 * AGV CONFIGURATION
 * */
#define AGV_ID                          0


#define DEFAULT_DUMPER_REVERSE_ANGLE    60
#define DEFAULT_DUMPER_W                50
#define DEFAULT_DUMPER_HOLDTIME_MS      800

//Wifi CONFIGURATION

#define WIFI_AP_NAME        "JD_XRobot"
#define WIFI_PASSWORD       "Auto_warehouse"
#define WIFI_LOCAL_IP       0

#define CONTROLLER_IP       MAKE_IP(192,168,51,180)


#define CONTROLLER_PORT         3000
#define AGVCP_LOCAL_UDP_PORT    3001

#endif
