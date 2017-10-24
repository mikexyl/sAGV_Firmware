/**
 * @file    rsi_ble_smp.c
 * @version 0.1
 * @date    09 Sep 2015
 *
 *  Copyright(C) Redpine Signals 2015
 *  All rights reserved by Redpine Signals.
 *
 *  @section License
 *  This program should be used on your own responsibility.
 *  Redpine Signals assumes no responsibility for any losses
 *  incurred by customers or third parties arising from the use of this file.
 *
 *  @brief : This file contains example application for BLE simple chat
 *
 *  @section Description  This application act as central role and initiate smp after connection
 *
 */

/**
 * Include files
 * */

//! BLE include file to refer BLE APIs
#include<rsi_ble_apis.h>
#include<rsi_ble_config.h>
#include<rsi_bt_common_apis.h>

//! Common include file
#include<rsi_common_apis.h>

#ifdef RSI_SAMPLE_HAL
#define LOG_PRINT  printf 
#else 
#define LOG_PRINT
#endif

//! remote device adress
#define  RSI_BLE_REMOTE_ADDR           "B4:99:4C:64:BC:AF"

//! Memory length for driver
#define BT_GLOBAL_BUFF_LEN              10000

//! Memory to initialize driver
uint8_t bt_global_buf[BT_GLOBAL_BUFF_LEN];

//! local device name
#define RSI_BLE_DEVICE_NAME            "BLE_SIMPLE_SMP"

//! local device IO capability
#define  RSI_BLE_SMP_IO_CAPABILITY      0x00
#define  RSI_BLE_SMP_PASSKEY            0

//! application event list
#define  RSI_BLE_CONN_EVENT             0x01
#define  RSI_BLE_DISCONN_EVENT          0x02
#define  RSI_BLE_SMP_REQ_EVENT          0x03
#define  RSI_BLE_SMP_RESP_EVENT         0x04
#define  RSI_BLE_SMP_PASSKEY_EVENT      0x05
#define  RSI_BLE_SMP_FAILED_EVENT       0x06
#define  RSI_BLE_ENCRYPT_STARTED_EVENT  0x07

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
//! global parameters list
static uint32_t ble_app_event_map;
static uint32_t ble_app_event_mask;
static uint8_t str_remote_address[18];

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
	ble_app_event_map = 0;
	ble_app_event_mask = 0xFFFFFFFF;
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
	ble_app_event_map |= BIT(event_num);
	return;
}

/*==============================================*/
/**
 * @fn         rsi_ble_app_clear_event
 * @brief      clears the specific event.
 * @param[in]  event_num, specific event number.
 * @return     none.
 * @section description
 * This function is used to clear the specific event.
 */
static void rsi_ble_app_clear_event(uint32_t event_num)
{
	ble_app_event_map &= ~BIT(event_num);
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
		if (ble_app_event_map & (1 << ix))
		{
		   return ix;
		}
	}

	return (-1);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_connect_event
 * @brief      invoked when connection complete event is received
 * @param[out] resp_conn, connected remote device information
 * @return     none.
 * @section description
 * This callback function indicates the status of the connection
 */
static void rsi_ble_on_connect_event(rsi_ble_event_conn_status_t *resp_conn)
{
  rsi_6byte_dev_address_to_ascii (str_remote_address, resp_conn->dev_addr);
  LOG_PRINT ("connect - str_remote_address : %s\r\n", str_remote_address);

	rsi_ble_app_set_event (RSI_BLE_CONN_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_disconnect_event
 * @brief      invoked when disconnection event is received
 * @param[in]  resp_disconnect, disconnected remote device information
 * @param[in]  reason, reason for disconnection.
 * @return     none.
 * @section description
 * This callback function indicates disconnected device information and status
 */
static void rsi_ble_on_disconnect_event(rsi_ble_event_disconnect_t *resp_disconnect, uint16_t reason)
{
  rsi_6byte_dev_address_to_ascii (str_remote_address, resp_disconnect->dev_addr);
  LOG_PRINT ("connect - str_remote_address : %s\r\n", str_remote_address);

	rsi_ble_app_set_event (RSI_BLE_DISCONN_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_smp_request 
 * @brief      its invoked when smp request event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when SMP request events is received(we are in Master mode)
 * Note: slave requested to start SMP request, we have to send SMP request command
 */
void rsi_ble_on_smp_request (rsi_bt_event_smp_req_t  *remote_dev_address)
{
  rsi_6byte_dev_address_to_ascii (str_remote_address, remote_dev_address->dev_addr);
  LOG_PRINT ("smp_req - str_remote_address : %s\r\n", str_remote_address);
  rsi_ble_app_set_event (RSI_BLE_SMP_REQ_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_smp_response 
 * @brief      its invoked when smp response event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when SMP response events is received(we are in slave mode) 
 * Note: Master initiated SMP protocol, we have to send SMP response command
 */
void rsi_ble_on_smp_response (rsi_bt_event_smp_resp_t  *remote_dev_address)
{
  rsi_6byte_dev_address_to_ascii (str_remote_address, remote_dev_address->dev_addr);
  LOG_PRINT ("smp_resp - str_remote_address : %s\r\n", str_remote_address);
  rsi_ble_app_set_event (RSI_BLE_SMP_RESP_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_smp_passkey 
 * @brief      its invoked when smp passkey event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when SMP passkey events is received
 * Note: We have to send SMP passkey command
 */
void rsi_ble_on_smp_passkey (rsi_bt_event_smp_passkey_t  *remote_dev_address)
{
  rsi_6byte_dev_address_to_ascii (str_remote_address, remote_dev_address->dev_addr);
  LOG_PRINT ("smp_passkey - str_remote_address : %s\r\n", str_remote_address);
  rsi_ble_app_set_event (RSI_BLE_SMP_PASSKEY_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_smp_failed 
 * @brief      its invoked when smp failed event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when SMP failed events is received
 */
void rsi_ble_on_smp_failed (uint16_t  status, rsi_bt_event_smp_failed_t  *remote_dev_address)
{
  rsi_6byte_dev_address_to_ascii (str_remote_address, remote_dev_address->dev_addr);
  LOG_PRINT ("smp_failed status: 0x%x, str_remote_address: %s\r\n", status, str_remote_address);
  rsi_ble_app_set_event (RSI_BLE_SMP_FAILED_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_encrypt_started 
 * @brief      its invoked when encryption started event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when encryption started events is received
 */
void rsi_ble_on_encrypt_started (uint16_t  status)
{
  LOG_PRINT ("start encrypt status: %d \r\n", status);
  rsi_ble_app_set_event (RSI_BLE_ENCRYPT_STARTED_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_smp_test_app
 * @brief      This application is used for testing the SMP protocol.
 * @param[in]  none. 
 * @return     none.
 * @section description
 * This function is used to test the SMP Protocol.
 */
void rsi_ble_smp_test_app (void)
{
	int32_t status = 0;
	int32_t  event_id;

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

	//! registering the GAP callback functions
	rsi_ble_gap_register_callbacks(
			NULL,
			rsi_ble_on_connect_event,
			rsi_ble_on_disconnect_event,
            NULL);

	//! registering the SMP callback functions
	rsi_ble_smp_register_callbacks(
			rsi_ble_on_smp_request,
			rsi_ble_on_smp_response,
			rsi_ble_on_smp_passkey,
			rsi_ble_on_smp_failed,
      rsi_ble_on_encrypt_started);

	//!  initializing the application events map
	rsi_ble_app_init_events ();

	//! Set local name
	rsi_bt_set_local_name(RSI_BLE_DEVICE_NAME);

  //! initiate BLE connection
  rsi_ble_connect (0, RSI_BLE_REMOTE_ADDR);

	//! waiting for events from controller.
	while (1) {

		//! Application main loop
#ifndef RSI_WITH_OS
		rsi_wireless_driver_task();
#endif
		//! checking for events list
		event_id = rsi_ble_app_get_event ();

		if (event_id == -1) {
			continue;
		}

		switch (event_id) {
			case RSI_BLE_CONN_EVENT:
			{
				//! event invokes when connection was completed

				//! clear the served event
				rsi_ble_app_clear_event (RSI_BLE_CONN_EVENT);

        //! initiating the SMP pairing process
        status = rsi_ble_smp_pair_request (str_remote_address, RSI_BLE_SMP_IO_CAPABILITY);
			}
			break;

			case RSI_BLE_DISCONN_EVENT:
			{
				//! event invokes when disconnection was completed

				//! clear the served event
				rsi_ble_app_clear_event (RSI_BLE_DISCONN_EVENT);
			}
			break;

      case RSI_BLE_SMP_REQ_EVENT:
      {
        //! initiate SMP protocol as a Master

				//! clear the served event
				rsi_ble_app_clear_event (RSI_BLE_SMP_REQ_EVENT);

        //! initiating the SMP pairing process
        status = rsi_ble_smp_pair_request (str_remote_address, RSI_BLE_SMP_IO_CAPABILITY);
      }
      break;

      case RSI_BLE_SMP_RESP_EVENT:
      {
        //! initiate SMP protocol as a Master

				//! clear the served event
				rsi_ble_app_clear_event (RSI_BLE_SMP_RESP_EVENT);

        //! initiating the SMP pairing process
        status = rsi_ble_smp_pair_response (str_remote_address, RSI_BLE_SMP_IO_CAPABILITY);
      }
      break;

      case RSI_BLE_SMP_PASSKEY_EVENT:
      {
        //! initiate SMP protocol as a Master

				//! clear the served event
				rsi_ble_app_clear_event (RSI_BLE_SMP_PASSKEY_EVENT);

        //! initiating the SMP pairing process
        status = rsi_ble_smp_passkey (str_remote_address, RSI_BLE_SMP_PASSKEY);
      }
      break;

      case RSI_BLE_SMP_FAILED_EVENT:
      {
        //! initiate SMP protocol as a Master

				//! clear the served event
				rsi_ble_app_clear_event (RSI_BLE_SMP_FAILED_EVENT);
      }
      break;

      case RSI_BLE_ENCRYPT_STARTED_EVENT:
      {
        //! start the encrypt event
        
				//! clear the served event
				rsi_ble_app_clear_event (RSI_BLE_ENCRYPT_STARTED_EVENT);
      }
      break;

		}
	}

	return;
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
#ifdef RSI_WITH_OS
  int32_t status;
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
  //! Call Simple SMP protocol test Application
  rsi_ble_smp_test_app();

  //! Application main loop
  main_loop();

  return 0;
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
  rsi_task_create(rsi_ble_smp_test_app, "ble_task", RSI_BT_TASK_STACK_SIZE, NULL, RSI_BT_TASK_PRIORITY, &bt_task_handle);

  //! Task created for Driver task
  rsi_task_create(rsi_wireless_driver_task, "driver_task",RSI_DRIVER_TASK_STACK_SIZE, NULL, RSI_DRIVER_TASK_PRIORITY, &driver_task_handle);

  //! OS TAsk Start the scheduler
  rsi_start_os_scheduler();

  return status;
#endif
}
