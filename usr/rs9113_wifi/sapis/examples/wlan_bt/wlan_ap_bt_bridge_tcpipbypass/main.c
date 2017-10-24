/**
 * @file    main.c
 * @version 0.1
 * @date    15 Aug 2015
 *
 *  Copyright(C) Redpine Signals 2015
 *  All rights reserved by Redpine Signals.
 *
 *  @section License
 *  This program should be used on your own responsibility.
 *  Redpine Signals assumes no responsibility for any losses
 *  incurred by customers or third parties arising from the use of this file.
 *
 *  @brief : This file contains application to explain bridging between wlan and bT tasks
 *
 *  @section Description  This file contains initialization , wlan and bT tasks execution in loop
 *
 *
 */

#include "rsi_driver.h"

//! Error include files
#include "rsi_error.h"

#ifdef RSI_WITH_OS
//! OS include file to refer OS specific functionality
#include "rsi_os.h"
#endif

//! BLE include file to refer BLE APIs
#include<rsi_bt_apis.h>
#include<rsi_bt_config.h>
#include<rsi_bt_common_apis.h>

//! Common include file 
#include<rsi_common_apis.h>

//! Memory length for driver
#define GLOBAL_BUFF_LEN    10000

#ifdef RSI_WITH_OS
//!Wlan Bt task priority
#define RSI_WLAN_BT_TASK_PRIORITY 	1

//! Wlan task priority
#define RSI_WLAN_TASK_PRIORITY   2

//! BT task priority
#define RSI_BT_TASK_PRIORITY   3

//! Wireless driver task priority
#define RSI_DRIVER_TASK_PRIORITY   4

//! Wlan BT task stack size
#define RSI_WLAN_BT_TASK_STACK_SIZE 1000

//! Wlan task stack size
#define RSI_WLAN_TASK_STACK_SIZE  1000

//! BT task stack size
#define RSI_BT_TASK_STACK_SIZE 1000

//! Wireless driver task stack size
#define RSI_DRIVER_TASK_STACK_SIZE  3000

#endif

//! Wlan access point mode 
#define RSI_WLAN_AP_MODE    6

//! BTLE Coex mode 
#define RSI_BT_MODE           5

//! Parameter to run forever loop
#define RSI_FOREVER             1 

//! Memory to initialize driver
uint8_t global_buf[GLOBAL_BUFF_LEN];

//! Function prototypes
extern void rsi_wlan_ap_app_task(void);
extern void rsi_bt_app_init (void);
extern void rsi_bt_app_task (void);

#ifdef RSI_WITH_OS
rsi_semaphore_handle_t commonsem,wlan_thread_sem,bt_thread_sem;

int32_t rsi_wlan_bt_app_init(void)
{
	int32_t     status = RSI_SUCCESS;
	rsi_task_handle_t wlan_task_handle = NULL;
	rsi_task_handle_t bt_task_handle = NULL;

	//! WiSeConnect initialization
	status = rsi_wireless_init(RSI_WLAN_AP_MODE, RSI_OPERMODE_WLAN_BT_CLASSIC);
	if(status != RSI_SUCCESS)
	{
		return status;
	}
	//! BT initialization
	rsi_bt_app_init();

	//! Task created for WLAN task
	rsi_task_create( rsi_wlan_ap_app_task, "wlan_task", RSI_WLAN_TASK_STACK_SIZE, NULL, RSI_WLAN_TASK_PRIORITY, &wlan_task_handle);

	//! Task created for BT task
	rsi_task_create( rsi_bt_app_task, "bt_task",RSI_BT_TASK_STACK_SIZE, NULL, RSI_WLAN_BT_TASK_PRIORITY , &bt_task_handle);

	 rsi_semaphore_wait(&commonsem,0);
}
#endif

int32_t rsi_wlan_ap_bt_app(void)
{
	int32_t     status = RSI_SUCCESS;

	//! WiSeConnect initialization
	status = rsi_wireless_init(RSI_WLAN_AP_MODE, RSI_OPERMODE_WLAN_BT_CLASSIC);
	if(status != RSI_SUCCESS)
	{
		return status;
	}

	//! BT initialization
	rsi_bt_app_init();

	while(RSI_FOREVER)
	{
		//! WLAN application tasks
		rsi_wlan_ap_app_task();

		//! BT application tasks
		rsi_bt_app_task();
#ifndef RSI_WITH_OS

		//! wireless driver tasks
		rsi_wireless_driver_task();
#endif
	}
	return 0;
}

//! Forever in wireless driver task 
void main_loop()
{
	while (RSI_FOREVER)
	{
		rsi_wireless_driver_task ();
	}
}

//! main funtion definition
int main(void)
{
  int32_t status;

#ifdef RSI_WITH_OS
	rsi_task_handle_t wlan_bt_task_handle = NULL;
	rsi_task_handle_t driver_task_handle = NULL;
#endif

#ifndef RSI_SAMPLE_HAL
	//! Board Initialization
	Board_init();
#endif

	//! Driver initialization
	status = rsi_driver_init(global_buf, GLOBAL_BUFF_LEN);
	if((status < 0) || (status > GLOBAL_BUFF_LEN))
	{
		return status;
	}

	//! RS9113 intialisation
	status = rsi_device_init(RSI_LOAD_IMAGE_I_FW);
	if(status != RSI_SUCCESS)
	{
		return status;
	}

  //Start BT Stack
  intialize_bt_stack(STACK_BT_MODE);

#ifdef RSI_WITH_OS
	rsi_semaphore_create(&wlan_thread_sem,0);
    rsi_semaphore_create(&bt_thread_sem,0);
    rsi_semaphore_create(&commonsem,0);
	//! OS case
	//! Task created for COMMON task
	rsi_task_create(rsi_wlan_bt_app_init, "common_task", RSI_WLAN_BT_TASK_STACK_SIZE, NULL, RSI_WLAN_BT_TASK_PRIORITY, &wlan_bt_task_handle);

	//! Task created for Driver task
	rsi_task_create(rsi_wireless_driver_task, "driver_task",RSI_DRIVER_TASK_STACK_SIZE, NULL, RSI_DRIVER_TASK_PRIORITY, &driver_task_handle);

	//! OS TAsk Start the scheduler
	rsi_start_os_scheduler();

#else
	//! NON - OS case
	//! Call  WLAN BT  application
	status = rsi_wlan_ap_bt_app();

	//! Application main loop
	main_loop();
#endif
	return status;
}
