/**
 * @file    rsi_ble_central.c
 * @version 0.4
 * @date    19 May 2016
 *
 *  Copyright(C) Redpine Signals 2015
 *  All rights reserved by Redpine Signals.
 *
 *  @section License
 *  This program should be used on your own responsibility.
 *  Redpine Signals assumes no responsibility for any losses
 *  incurred by customers or third parties arising from the use of this file.
 *
 *  @brief : This file contains example application for BLE Central mode in power save mode.
 *
 *  @section Description  This application connects as a Central/Master in power save mode.
 *
 */

/**
 * Include files
 * */

//! BLE include file to refer BLE APIs
#include<rsi_ble_apis.h>
#include<rsi_ble_config.h>
#include<rsi_bt_common_apis.h>
#include<rsi_common_apis.h>

//! application defines 
//! OS include file to refer OS specific functionality
#include "rsi_os.h"
#ifdef RSI_SAMPLE_HAL
#define LOG_PRINT                     printf
#else 
#define LOG_PRINT
#endif

#define SCAN_MODE             1
#define CONNECTED_MODE        2

#define LE_PS_MODE         SCAN_MODE

//! Power Save Profile Mode
#define PSP_MODE              RSI_SLEEP_MODE_2

//! Power Save Profile type
#define PSP_TYPE              RSI_MAX_PSP

uint8_t remote_dev_addr[18] = {0};
//! Address type of the device to connect
#define RSI_BLE_DEV_ADDR_TYPE LE_PUBLIC_ADDRESS

//! Address of the device to connect
#define RSI_BLE_DEV_ADDR      "11:11:11:11:11:11"

//! Memory length for the driver
#define BT_GLOBAL_BUFF_LEN   10000

//! Memory to initialize the driver
uint8_t bt_global_buf[BT_GLOBAL_BUFF_LEN];

//! Maximum number of advertise reports to hold
#define NO_OF_ADV_REPORTS              10

//! Application supported events list
#define RSI_APP_EVENT_ADV_REPORT       0
#define RSI_APP_EVENT_CONNECTED        1
#define RSI_APP_EVENT_DISCONNECTED     2

#ifdef RSI_WITH_OS
//! BLE task stack size
#define RSI_BT_TASK_STACK_SIZE  1000

//! BT task priority
#define RSI_BT_TASK_PRIORITY   1

//! Number of packet to send or receive
#define NUMBER_OF_PACKETS 1000

//! Wireless driver task priority
#define RSI_DRIVER_TASK_PRIORITY   2

//! Wireless driver task stack size
#define RSI_DRIVER_TASK_STACK_SIZE  3000

void rsi_wireless_driver_task(void);

#endif
//! Application global parameters.
static uint16_t rsi_app_no_of_adv_reports_rcvd = 0;
static uint8_t rsi_app_async_event_map = 0;
static rsi_ble_event_adv_report_t      rsi_app_adv_reports_to_app[NO_OF_ADV_REPORTS];
static rsi_ble_event_conn_status_t     rsi_app_connected_device;
static rsi_ble_event_disconnect_t      rsi_app_disconnected_device;

/*==============================================*/
/**
 * @fn         rsi_ble_app_init_events
 * @brief      initializes the event parameter.
 * @param[in]  none.
 * @return     none.
 * @section description
 * This function is used during BLE initialization.
 */
static void rsi_ble_app_init_events()
{
	rsi_app_async_event_map = 0;
	return;
}

/*==============================================*/
/**
 * @fn         rsi_ble_app_set_event
 * @brief      set the specific event.
 * @param[in]  event_num, specific event number.
 * @return     none.
 * @section description
 * This function is used to set/raise the specific event.
 */
static void rsi_ble_app_set_event(uint32_t event_num)
{
	rsi_app_async_event_map |= BIT(event_num);
	return;
}

/*==============================================*/
/**
 * @fn         rsi_ble_app_clear_event
 * @brief      clear the specific event.
 * @param[in]  event_num, specific event number.
 * @return     none.
 * @section description
 * This function is used to clear the specific event.
 */
static void rsi_ble_app_clear_event(uint32_t event_num)
{
	rsi_app_async_event_map &= ~BIT(event_num);
	return;
}

/*==============================================*/
/**
 * @fn         rsi_ble_app_get_event
 * @brief      returns the first set event based on priority
 * @param[in]  none.
 * @return     int32_t
 *             > 0  = event number
 *             -1   = not received any event
 * @section description
 * This function returns the highest priority event among all the set events
 */
static int32_t rsi_ble_app_get_event(void)
{
	uint32_t  ix;

	for (ix = 0; ix < 32; ix++)
	{
		if (rsi_app_async_event_map & (1 << ix))
		{
		   return ix;
		}
	}

	return (-1);
}

/*==============================================*/
/**
 * @fn         rsi_ble_simple_central_on_adv_report_event
 * @brief      invoked when advertise report event is received
 * @param[in]  adv_report, pointer to the received advertising report
 * @return     none.
 * @section description
 * This callback function updates the scanned remote devices list
 */
void rsi_ble_simple_central_on_adv_report_event(rsi_ble_event_adv_report_t *adv_report)
{
	memcpy(&rsi_app_adv_reports_to_app[(rsi_app_no_of_adv_reports_rcvd)% (NO_OF_ADV_REPORTS)], adv_report, sizeof(rsi_ble_event_adv_report_t));
	rsi_app_no_of_adv_reports_rcvd++;
	rsi_6byte_dev_address_to_ascii ((int8_t *)remote_dev_addr, (uint8_t *)adv_report->dev_addr);	
  LOG_PRINT ("\n DEV_ADDR: %s\r\n", remote_dev_addr);
	rsi_ble_app_set_event (RSI_APP_EVENT_ADV_REPORT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_simple_central_on_conn_status_event
 * @brief      invoked when connection complete event is received
 * @param[out] resp_conn, connected remote device information
 * @return     none.
 * @section description
 * This callback function indicates the status of the connection
 */
void rsi_ble_simple_central_on_conn_status_event(rsi_ble_event_conn_status_t *resp_conn)
{
	memcpy(&rsi_app_connected_device, resp_conn, sizeof(rsi_ble_event_conn_status_t));

	rsi_ble_app_set_event (RSI_APP_EVENT_CONNECTED);
}

/*==============================================*/
/**
 * @fn         rsi_ble_simple_central_on_disconnect_event
 * @brief      invoked when disconnection event is received
 * @param[in]  resp_disconnect, disconnected remote device information
 * @param[in]  reason, reason for disconnection.
 * @return     none.
 * @section description
 * This Callback function indicates disconnected device information and status
 */
void rsi_ble_simple_central_on_disconnect_event(rsi_ble_event_disconnect_t *resp_disconnect, uint16_t reason)
{
	memcpy(&rsi_app_disconnected_device, resp_disconnect, sizeof(rsi_ble_event_disconnect_t ));

	rsi_ble_app_set_event (RSI_APP_EVENT_DISCONNECTED);
}

/*==============================================*/
/**
 * @fn         rsi_ble_central
 * @brief      Tests the BLE GAP central role.
 * @param[in]  none
  * @return    none.
 * @section description
 * This function is used to test the BLE central role and simple GAP API's.
 */
int32_t rsi_ble_central(void)
{
	int32_t status = 0;
	int32_t temp_event_map = 0;

#ifndef RSI_WITH_OS
	//! Driver initialization
	status = rsi_driver_init(bt_global_buf, BT_GLOBAL_BUFF_LEN);
  if((status < 0) || (status > BT_GLOBAL_BUFF_LEN))
	{
		return status;
	}

  //! RS9113 intialisation 
  status = rsi_device_init(RSI_LOAD_IMAGE_I_FW);
	if(status != RSI_SUCCESS)
	{
		return status;
	}
#endif

	//! WC initialization
	status = rsi_wireless_init(0, RSI_OPERMODE_WLAN_BLE);
	if(status != RSI_SUCCESS)
	{
		return status;
	}

	//! BLE register GAP callbacks
	rsi_ble_gap_register_callbacks(
			rsi_ble_simple_central_on_adv_report_event,
			rsi_ble_simple_central_on_conn_status_event,
			rsi_ble_simple_central_on_disconnect_event,
            NULL);

	//! initialize the event map
	rsi_ble_app_init_events ();

	//! start scanning
	status = rsi_ble_start_scanning();
	if(status != RSI_SUCCESS)
	{
    return status;
  }

#if(LE_PS_MODE == SCAN_MODE)
  status = rsi_bt_power_save_profile(PSP_MODE, PSP_TYPE);
  if(status != RSI_SUCCESS)
  {
    return status;
  }
#endif

	while(1)
	{
		//! Application main loop
#ifndef RSI_WITH_OS
		//! run the non os scheduler
		rsi_wireless_driver_task();
#endif
		//! checking for received events
		temp_event_map = rsi_ble_app_get_event ();
		if (temp_event_map == RSI_FAILURE) {
			//! if events are not received, loop will be continued
			continue;
		}

		switch(temp_event_map)
		{
			case RSI_APP_EVENT_ADV_REPORT:
			{
				//! advertise report event.
				if(!strcmp(remote_dev_addr,RSI_BLE_DEV_ADDR))
				{
                //! initiate stop scanning command.
								status = rsi_ble_stop_scanning();
								if (status != RSI_SUCCESS)
								{
												return status;
								}

                //! initiateing the connection with remote BLE device
								status = rsi_ble_connect(RSI_BLE_DEV_ADDR_TYPE, RSI_BLE_DEV_ADDR);
								if(status != RSI_SUCCESS)
								{
												return status;
								}

#if(LE_PS_MODE == CONNECTED_MODE)
                //! initiateing the connection with remote BLE device
                status = rsi_bt_power_save_profile(RSI_SLEEP_MODE_2, PSP_TYPE);
                if(status != RSI_SUCCESS)
                {
                  return status;
                }
#endif
        }
        //! clear the advertise report event.
        rsi_ble_app_clear_event (RSI_APP_EVENT_ADV_REPORT);
      }
      break;

			case RSI_APP_EVENT_CONNECTED:
			{
				//! remote device connected event
        LOG_PRINT("\n connected sucess .............\n");
				//! clear the connected event.
				rsi_ble_app_clear_event (RSI_APP_EVENT_CONNECTED);
			}
			break;

			case RSI_APP_EVENT_DISCONNECTED:
			{
				//! remote device disconnected event
				LOG_PRINT("\n disconnected event............ \n ");
				//! clear the disconnected event.
				rsi_ble_app_clear_event (RSI_APP_EVENT_DISCONNECTED);
			}
			break;
		}
	}
	return 0;
}

/*==============================================*/
/**
 * @fn         main_loop
 * @brief      Schedules the Driver task.
 * @param[in]  none.
 * @return     none.
 * @section description
 * This function schedules the Driver task.
 */
void main_loop (void)
{
	while (1) {
		rsi_wireless_driver_task ();
	}
}

/*==============================================*/
/**
 * @fn         main
 * @brief      main function
 * @param[in]  none.
 * @return     none.
 * @section description
 * This is the main function to call Application
 */
int main (void)
{
  int32_t status;
#ifdef RSI_WITH_OS
  rsi_task_handle_t bt_task_handle = NULL;
  rsi_task_handle_t driver_task_handle = NULL;
#endif

#ifndef RSI_SAMPLE_HAL
  //! Board Initialization
  Board_init();
#endif

#ifndef RSI_WITH_OS
  //Start BT Stack
  intialize_bt_stack(STACK_BTLE_MODE);
  
  //! Call BLE Central application
  status = rsi_ble_central();

  //! Application main loop
  main_loop();

  return status;
#endif
#ifdef RSI_WITH_OS
 //! Driver initialization
  status = rsi_driver_init(bt_global_buf, BT_GLOBAL_BUFF_LEN);
  if((status < 0) || (status > BT_GLOBAL_BUFF_LEN))
  {
    return status;
  }
  //! RS9113 initialization
  status = rsi_device_init(RSI_LOAD_IMAGE_I_FW);
  if(status != RSI_SUCCESS)
  {
    return status;
  }

  //Start BT Stack
  intialize_bt_stack(STACK_BTLE_MODE);

  //! OS case
  //! Task created for BLE task
  rsi_task_create(rsi_ble_central, "ble_task", RSI_BT_TASK_STACK_SIZE, NULL, RSI_BT_TASK_PRIORITY, &bt_task_handle);

  //! Task created for Driver task
  rsi_task_create(rsi_wireless_driver_task, "driver_task",RSI_DRIVER_TASK_STACK_SIZE, NULL, RSI_DRIVER_TASK_PRIORITY, &driver_task_handle);

  //! OS TAsk Start the scheduler
  rsi_start_os_scheduler();

  return status;
#endif
}


