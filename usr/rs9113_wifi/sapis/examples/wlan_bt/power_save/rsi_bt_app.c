/**
 * @file    rsi_bt_spp_slave.c
 * @version 0.1
 * @date    xx Oct 2015
 *
 *  Copyright(C) Redpine Signals 2015
 *  All rights reserved by Redpine Signals.
 *
 *  @section License
 *  This program should be used on your own responsibility.
 *  Redpine Signals assumes no responsibility for any losses
 *  incurred by customers or third parties arising from the use of this file.
 *
 *  @brief : This file contains example application for BT Classic SPP Slave Role
 *
 *  @section Description  This application serves as a BT SPP Slave.
 *
 */

/**
 * Include files
 * */

//! BT include file to refer BT APIs
#include<rsi_bt_apis.h>
#include<rsi_bt_common_apis.h>
#include<rsi_bt_config.h>
#include<rsi_bt.h>
#include <stdio.h>


//! application defines 
#ifdef RSI_SAMPLE_HAL
#define LOG_PRINT                     printf
#else 
#define LOG_PRINT
#endif
#define RSI_BT_LOCAL_NAME             "CoEx_Power_Save"
#define PIN_CODE                      "4321"

//! application events list
#define RSI_APP_EVENT_CONNECTED       1
#define RSI_APP_EVENT_PINCODE_REQ     2
#define RSI_APP_EVENT_LINKKEY_SAVE    3
#define RSI_APP_EVENT_AUTH_COMPLT     4
#define RSI_APP_EVENT_DISCONNECTED    5
#define RSI_APP_EVENT_LINKKEY_REQ     6
#define RSI_APP_EVENT_SPP_CONN        7
#define RSI_APP_EVENT_SPP_DISCONN     8
#define RSI_APP_EVENT_SPP_RX          9
/*** SNIFF RELATED DEFINES********/
#define RSI_APP_EVENT_MODE_CHANGED    10
#define RSI_APP_EVENT_SNIFF_SUBRATING 11

//! Memory length for driver
#define BT_GLOBAL_BUFF_LEN            10000

//! Power Save Profile Mode
#define PSP_TYPE              RSI_MAX_PSP

//! Sniff Parameters
#define SNIFF_MAX_INTERVAL    0xA0 
#define SNIFF_MIN_INTERVAL    0XA0
#define SNIFF_ATTEMPT         0X04
#define SNIFF_TIME_OUT        0X02

//! Enumeration for commands used in application
typedef enum rsi_app_cmd_e
{
  RSI_DATA = 0
}rsi_app_cmd_t;

//! Memory to initialize driver
uint8_t bt_global_buf[BT_GLOBAL_BUFF_LEN] = {0};

//! Application global parameters.
static uint32_t rsi_app_async_event_map = 0;
static rsi_bt_resp_get_local_name_t   local_name = {0};
static uint8_t str_conn_bd_addr[18];
static uint8_t local_dev_addr[RSI_DEV_ADDR_LEN] = {0};
static uint8_t spp_data[200];
static uint16_t spp_data_len;

/*==============================================*/
/**
 * @fn         rsi_bt_app_init_events
 * @brief      initializes the event parameter.
 * @param[in]  none.
 * @return     none.
 * @section description
 * This function is used during BT initialization.
 */
static void rsi_bt_app_init_events()
{
	rsi_app_async_event_map = 0;
	return;
}

/*==============================================*/
/**
 * @fn         rsi_bt_app_set_event
 * @brief      sets the specific event.
 * @param[in]  event_num, specific event number.
 * @return     none.
 * @section description
 * This function is used to set/raise the specific event.
 */
static void rsi_bt_app_set_event(uint32_t event_num)
{
	rsi_app_async_event_map |= BIT(event_num);
	return;
}

/*==============================================*/
/**
 * @fn         rsi_bt_app_clear_event
 * @brief      clears the specific event.
 * @param[in]  event_num, specific event number.
 * @return     none.
 * @section description
 * This function is used to clear the specific event.
 */
static void rsi_bt_app_clear_event(uint32_t event_num)
{
	rsi_app_async_event_map &= ~BIT(event_num);
	return;
}

/*==============================================*/
/**
 * @fn         rsi_bt_app_get_event
 * @brief      returns the first set event based on priority
 * @param[in]  none.
 * @return     int32_t
 *             > 0  = event number
 *             -1   = not received any event
 * @section description
 * This function returns the highest priority event among all the set events
 */
static int32_t rsi_bt_app_get_event(void)
{
	uint32_t  ix;

	for (ix = 0; ix < 32; ix++)
	{
		if (rsi_app_async_event_map & (1 << ix))
		{
		   return ix;
		}
	}

	return (RSI_FAILURE);
}

/*==============================================*/
/**
 * @fn         rsi_bt_app_on_conn
 * @brief      invoked when connection complete event is received
 * @param[out] resp_status, connection status of the connected device.
 * @param[out] conn_event, connected remote device information
 * @return     none.
 * @section description
 * This callback function indicates the status of the connection
 */
void rsi_bt_app_on_conn (uint16_t resp_status, rsi_bt_event_bond_t *conn_event)
{
	rsi_bt_app_set_event (RSI_APP_EVENT_CONNECTED);
  rsi_6byte_dev_address_to_ascii(str_conn_bd_addr, conn_event->dev_addr);
  LOG_PRINT ("on_conn: str_conn_bd_addr: %s\r\n", str_conn_bd_addr);
}

/*==============================================*/
/**
 * @fn         rsi_bt_app_on_pincode_req
 * @brief      invoked when pincode request event is received
 * @param[out] user_pincode_request, pairing remote device information
 * @return     none.
 * @section description
 * This callback function indicates the pincode request from remote device
 */
void rsi_bt_app_on_pincode_req(uint16_t resp_status, rsi_bt_event_user_pincode_request_t *user_pincode_request)
{
	rsi_bt_app_set_event (RSI_APP_EVENT_PINCODE_REQ);
  rsi_6byte_dev_address_to_ascii(str_conn_bd_addr, user_pincode_request->dev_addr);
  LOG_PRINT ("on_pin_coe_req: str_conn_bd_addr: %s\r\n", str_conn_bd_addr);
}

/*==============================================*/
/**
 * @fn         rsi_bt_app_on_linkkey_req
 * @brief      invoked when linkkey request event is received
 * @param[out] user_linkkey_req, pairing remote device information
 * @return     none.
 * @section description
 * This callback function indicates the linkkey request from remote device
 */
void rsi_bt_app_on_linkkey_req (uint16_t status, rsi_bt_event_user_linkkey_request_t  *user_linkkey_req)
{
	rsi_bt_app_set_event (RSI_APP_EVENT_LINKKEY_REQ);
  rsi_6byte_dev_address_to_ascii(str_conn_bd_addr, user_linkkey_req->dev_addr);
  LOG_PRINT ("linkkey_req: str_conn_bd_addr: %s\r\n", str_conn_bd_addr);
}

/*==============================================*/
/**
 * @fn         rsi_bt_app_on_linkkey_save
 * @brief      invoked when linkkey save event is received
 * @param[out] user_linkkey_req, paired remote device information
 * @return     none.
 * @section description
 * This callback function indicates the linkkey save from local device
 */
void rsi_bt_app_on_linkkey_save (uint16_t status, rsi_bt_event_user_linkkey_save_t *user_linkkey_save)
{
	rsi_bt_app_set_event (RSI_APP_EVENT_LINKKEY_SAVE);
  rsi_6byte_dev_address_to_ascii(str_conn_bd_addr, user_linkkey_save->dev_addr);
  LOG_PRINT ("linkkey_save: str_conn_bd_addr: %s\r\n", str_conn_bd_addr);
}

/*==============================================*/
/**
 * @fn         rsi_bt_app_on_auth_complete
 * @brief      invoked when authentication event is received
 * @param[out] resp_status, authentication status
 * @param[out] auth_complete, paired remote device information
 * @return     none.
 * @section description
 * This callback function indicates the pairing status and remote device information
 */
void rsi_bt_app_on_auth_complete (uint16_t resp_status, rsi_bt_event_auth_complete_t *auth_complete)
{
	rsi_bt_app_set_event (RSI_APP_EVENT_AUTH_COMPLT);
  rsi_6byte_dev_address_to_ascii(str_conn_bd_addr, auth_complete->dev_addr);
  LOG_PRINT ("auth_complete: str_conn_bd_addr: %s\r\n", str_conn_bd_addr);
}

/*==============================================*/
/**
 * @fn         rsi_bt_app_on_disconn
 * @brief      invoked when disconnect event is received
 * @param[out] resp_status, disconnect status/error
 * @param[out] bt_disconnected, disconnected remote device information
 * @return     none.
 * @section description
 * This callback function indicates the disconnected device information
 */
void rsi_bt_app_on_disconn (uint16_t resp_status, rsi_bt_event_disconnect_t *bt_disconnected)
{
	rsi_bt_app_set_event (RSI_APP_EVENT_DISCONNECTED);
  rsi_6byte_dev_address_to_ascii(str_conn_bd_addr, bt_disconnected->dev_addr);
  LOG_PRINT ("on_disconn: str_conn_bd_addr: %s\r\n", str_conn_bd_addr);
}

/*==============================================*/
/**
 * @fn         rsi_bt_app_on_spp_connect
 * @brief      invoked when spp profile connected event is received
 * @param[out] spp_connect, spp connected remote device information
 * @return     none.
 * @section description
 * This callback function indicates the spp connected remote device information
 */
void rsi_bt_app_on_spp_connect (uint16_t resp_status, rsi_bt_event_spp_connect_t *spp_connect)
{
	rsi_bt_app_set_event (RSI_APP_EVENT_SPP_CONN);
  rsi_6byte_dev_address_to_ascii(str_conn_bd_addr, spp_connect->dev_addr);
  LOG_PRINT ("spp_conn: str_conn_bd_addr: %s\r\n", str_conn_bd_addr);
}

/*==============================================*/
/**
 * @fn         rsi_bt_app_on_spp_disconnect
 * @brief      invoked when spp profile disconnected event is received
 * @param[out] spp_disconn, spp disconnected remote device information
 * @return     none.
 * @section description
 * This callback function indicates the spp disconnected event
 */
void rsi_bt_app_on_spp_disconnect (uint16_t resp_status, rsi_bt_event_spp_disconnect_t *spp_disconn)
{
	rsi_bt_app_set_event (RSI_APP_EVENT_SPP_DISCONN);
  rsi_6byte_dev_address_to_ascii(str_conn_bd_addr, spp_disconn->dev_addr);
  LOG_PRINT ("spp_disconn: str_conn_bd_addr: %s\r\n", str_conn_bd_addr);
}

/*==============================================*/
/**
 * @fn         rsi_bt_on_mode_change
 * @brief      invoked when mode is changed event is received
 * @param[out] mode_change, mode related information
 * @return     none.
 * @section description
 * This callback function indicates the mode change in BT device.
 */

void rsi_bt_on_mode_change (uint16_t resp_status, rsi_bt_event_mode_change_t  *mode_change)
{
	uint8_t ix; 
	rsi_bt_app_set_event (RSI_APP_EVENT_MODE_CHANGED);
  rsi_6byte_dev_address_to_ascii(str_conn_bd_addr, mode_change->dev_addr);
  LOG_PRINT ("mode_change_event: str_conn_bd_addr: %s\r\n",str_conn_bd_addr);
}

/*==============================================*/
/**
 * @fn         rsi_bt_on_sniff_subrating
 * @brief      invoked when mode is changed(sniff subrating) event is received
 * @param[out] mode_change, mode related information
 * @return     none.
 * @section description
 * This callback function indicates the mode change in BT device.
 */
void rsi_bt_on_sniff_subrating (uint16_t resp_status,rsi_bt_event_sniff_subrating_t  *mode_change)
{
	uint8_t ix; 
	rsi_bt_app_set_event (RSI_APP_EVENT_SNIFF_SUBRATING);
  rsi_6byte_dev_address_to_ascii(str_conn_bd_addr, mode_change->dev_addr);
  LOG_PRINT ("mode_change_event: str_conn_bd_addr: %s\r\n",str_conn_bd_addr);
}

/*==============================================*/
/**
 * @fn         rsi_wlan_app_send_to_bt
 * @brief      this function is used to send data to ble app.
 * @param[in]   msg_type, it indicates write/notification event id.
 * @param[in]  data, raw data pointer.
 * @param[in]  data_len, raw data length.
 * @return     none.
 * @section description
 * This is a callback function
 */
void rsi_wlan_app_send_to_bt (uint16_t  msg_type, uint8_t *data, uint16_t data_len)
{
  spp_data_len = RSI_MIN (200, data_len);
  memcpy (spp_data, data, spp_data_len);
	rsi_bt_app_set_event (RSI_APP_EVENT_SPP_RX);
}


/*==============================================*/
/**
 * @fn         rsi_bt_app_on_spp_data_rx
 * @brief      invoked when spp data rx event is received
 * @param[out] spp_receive, spp data from remote device
 * @return     none.
 * @section description
 * This callback function indicates the spp data received event
 */
void rsi_bt_app_on_spp_data_rx (uint16_t resp_status, rsi_bt_event_spp_receive_t *spp_receive)
{
  uint16_t ix;

/*  LOG_PRINT ("spp_rx: data_len: %d, data: ", spp_data_len);
  for (ix = 0; ix < spp_receive->data_len; ix++)
  {
    LOG_PRINT (" 0x%02x,", spp_receive->data[ix]);
  }
  LOG_PRINT ("\r\n");
  LOG_PRINT ("data: %s", spp_receive->data);
*/

  rsi_bt_app_send_to_wlan(RSI_DATA, spp_receive->data, spp_receive->data_len);
}

/*==============================================*/
/**
 * @fn         rsi_bt_app_init
 * @brief      Tests the BT Classic SPP Slave role.
 * @param[in]  none
  * @return    none.
 * @section description
 * This function is used to test the SPP Slave role.
 */
int32_t rsi_bt_app_init (void)
{
  int32_t status = 0;
  int32_t temp_event_map = 0;
  uint8_t str_bd_addr[18] = {0};

  //! BT register GAP callbacks:
  rsi_bt_gap_register_callbacks(
      NULL, //role_change
      rsi_bt_app_on_conn, 
      NULL, //
      rsi_bt_app_on_disconn,
      NULL,//scan_resp
      NULL,//remote_name_req
      NULL,//passkey_display
      NULL,//remote_name_req+cancel
      NULL,//confirm req
      rsi_bt_app_on_pincode_req,
      NULL,//passkey request
      NULL,//inquiry complete
      rsi_bt_app_on_auth_complete,
      rsi_bt_app_on_linkkey_req,//linkkey request
      NULL,//ssp coplete
      rsi_bt_app_on_linkkey_save,
      NULL, //get services
      NULL,
      rsi_bt_on_mode_change,
      rsi_bt_on_sniff_subrating); //search service

  //! initialize the event map
  rsi_bt_app_init_events ();

  //! get the local device address(MAC address).
  status = rsi_bt_get_local_device_address(local_dev_addr);
  if(status != RSI_SUCCESS)
  {
    return status;
  }
  rsi_6byte_dev_address_to_ascii (str_bd_addr, local_dev_addr);
  LOG_PRINT ("\r\nlocal_bd_address: %s\r\n", str_bd_addr);

  //! set the local device name
  status = rsi_bt_set_local_name(RSI_BT_LOCAL_NAME);
  if(status != RSI_SUCCESS)
  {
    return status;
  }

  //! get the local device name
  status = rsi_bt_get_local_name(&local_name);
  if(status != RSI_SUCCESS)
  {
    return status;
  }
  LOG_PRINT ("\r\nlocal_name: %s\r\n", local_name.name);
  
  //! start the discover mode
  status = rsi_bt_start_discoverable();
  if(status != RSI_SUCCESS)
  {
    return status;
  }

  //! start the connectability mode
  status = rsi_bt_set_connectable();
  if(status != RSI_SUCCESS)
  {
    return status;
  }

  //! initilize the SPP profile
  status = rsi_bt_spp_init ();
  if(status != RSI_SUCCESS)
  {
    return status;
  }

  //! register the SPP profile callback's
  rsi_bt_spp_register_callbacks (rsi_bt_app_on_spp_connect,
                                 rsi_bt_app_on_spp_disconnect,
                                 rsi_bt_app_on_spp_data_rx);

  return;

}

void rsi_bt_app_task ()
{
  int32_t status = 0;
  int32_t temp_event_map = 0;
  uint8_t str_bd_addr[18] = {0};


  //! Application main loop

  //! checking for received events
  temp_event_map = rsi_bt_app_get_event ();
  if (temp_event_map == RSI_FAILURE) {
    //! if events are not received loop will be continued.
    return;
  }

  //! if any event is received, it will be served.
  switch(temp_event_map)
  {
    case RSI_APP_EVENT_CONNECTED:
      {
        //! remote device connected event

        //! clear the connected event.
        rsi_bt_app_clear_event (RSI_APP_EVENT_CONNECTED);
      }
      break;

    case RSI_APP_EVENT_PINCODE_REQ:
      {
        //! pincode request event
        uint8_t *pin_code = PIN_CODE;

        //! clear the pincode request event.
        rsi_bt_app_clear_event(RSI_APP_EVENT_PINCODE_REQ);

        //! sending the pincode requet reply
        status = rsi_bt_pincode_request_reply(str_conn_bd_addr, pin_code, 1);
        if(status != RSI_SUCCESS)
        {
          return status;
        }
      }
      break;
    case RSI_APP_EVENT_LINKKEY_SAVE:
      {
        //! linkkey save event

        //! clear the likkey save event.
        rsi_bt_app_clear_event (RSI_APP_EVENT_LINKKEY_SAVE);
      }
      break;
    case RSI_APP_EVENT_AUTH_COMPLT:
      {
        //! authentication complete event

        //! clear the authentication complete event.
        rsi_bt_app_clear_event (RSI_APP_EVENT_AUTH_COMPLT);
      }
      break;
    case RSI_APP_EVENT_DISCONNECTED:
      {
        //! remote device connected event

        //! clear the disconnected event.
        rsi_bt_app_clear_event (RSI_APP_EVENT_DISCONNECTED);
      }
      break;
    case RSI_APP_EVENT_LINKKEY_REQ:
      {
        //! linkkey request event

        //! clear the linkkey request event.
        rsi_bt_app_clear_event (RSI_APP_EVENT_LINKKEY_REQ);

        LOG_PRINT ("linkkey_req: %s\r\n", str_conn_bd_addr);
        //! sending the linkkey request negative reply
        rsi_bt_linkkey_request_reply (str_conn_bd_addr, NULL, 0);
      }
      break;

    case RSI_APP_EVENT_SPP_CONN:
      {
        //! spp connected event

        //! clear the spp connected event.
        rsi_bt_app_clear_event (RSI_APP_EVENT_SPP_CONN);

        /* here we call the sniff_mode command*/
        status = rsi_bt_sniff_mode(str_conn_bd_addr,SNIFF_MAX_INTERVAL,SNIFF_MIN_INTERVAL,SNIFF_ATTEMPT,SNIFF_TIME_OUT);  
        if(status != RSI_SUCCESS)
        {
          return status;
        }

        status = rsi_bt_power_save_profile(RSI_SLEEP_MODE_2, PSP_TYPE);          
        if(status != RSI_SUCCESS)
        {
          return status;
        }
      }
      break;

    case RSI_APP_EVENT_SPP_DISCONN:
      {
        //! spp disconnected event

        //! clear the spp disconnected event.
        rsi_bt_app_clear_event (RSI_APP_EVENT_SPP_DISCONN);
      }
      break;

    case RSI_APP_EVENT_SPP_RX:
      {
        //! spp receive event

        //! clear the spp receive event.
        rsi_bt_app_clear_event (RSI_APP_EVENT_SPP_RX);

        //! SPP data transfer (loop back)
        rsi_bt_spp_transfer (str_conn_bd_addr, spp_data, spp_data_len);
      }
      break;
  
    case RSI_APP_EVENT_MODE_CHANGED:
      {
        //! clear the ssp receive event.
        rsi_bt_app_clear_event (RSI_APP_EVENT_MODE_CHANGED);
        LOG_PRINT("MODE_CHANGE IS COMPLETED\n");
      }
      break;

    case RSI_APP_EVENT_SNIFF_SUBRATING:
      {
        //! clear the ssp receive event.
        rsi_bt_app_clear_event (RSI_APP_EVENT_SNIFF_SUBRATING);
        LOG_PRINT("SNIFF SUBRATING IS COMPLETED\n");
      }
      break;
  }

  return 0;
}


