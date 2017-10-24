/**
 * @file    rsi_wlan_ap_app_task.c
 * @version 0.4
 * @date    16 May 2016
 *
 *  Copyright(C) Redpine Signals 2015
 *  All rights reserved by Redpine Signals.
 *
 *  @section License
 *  This program should be used on your own responsibility.
 *  Redpine Signals assumes no responsibility for any losses
 *  incurred by customers or third parties arising from the use of this file.
 *
 *  @brief : This file contains wlan related operations to create ap and send data to client
 *
 *  @section Description  This file contains wlan releated apis to create ap and communicate to its  client over tcp client socket
 *
 *
 */
/**
 * Include files
 * */

//! Driver Header file
#include "rsi_driver.h"

//! WLAN include file to refer wlan APIs
#include "rsi_wlan_apis.h"

//! socket include file to refer socket APIs
#include "rsi_socket.h"

#define RSI_APP_BUF_SIZE        1600

//! SEND event number used in the application
#define RSI_SEND_EVENT                  BIT(0)

//! Access point SSID to connect
#define SSID               "REDPINE_AP"

//! Channel Number
#define CHANNEL_NO             11 

//! Security Type
#define SECURITY_TYPE        RSI_WPA2   

//! Encryption Type
#define ENCRYPTION_TYPE      RSI_CCMP      

//! Password
#define PSK                "1234567890"

//! Beacon Interval
#define BEACON_INTERVAL        100

//! DTIM Count
#define DTIM_COUNT             4  

//! IP address of the module 
//! E.g: 0x650AA8C0 == 192.168.10.101
#define DEVICE_IP          0x010AA8C0

//! IP address of Gateway
//! E.g: 0x010AA8C0 == 192.168.10.1
#define GATEWAY            0x010AA8C0

//! IP address of netmask
//! E.g: 0x00FFFFFF == 255.255.255.0
#define NETMASK            0x00FFFFFF 

//! Device port number
#define DEVICE_PORT        5001




//! client socket id
int32_t     client_socket,server_socket,new_socket, addr_size;

//! stations count
uint8_t rsi_stations_count;

//! server and client ip addresses
struct      sockaddr_in server_addr, client_addr;

#if !(DHCP_MODE)

//! device static ip address
uint32_t    ip_addr      = DEVICE_IP;

//! Network mask
uint32_t    network_mask = NETMASK;

//! Gateway
uint32_t    gateway      = GATEWAY;
#endif

//! Enumeration for states in applcation 
typedef enum rsi_wlan_app_state_e
{
  RSI_WLAN_INITIAL_STATE              = 0,
  RSI_WLAN_AP_UP_STATE                = 1,
  RSI_WLAN_SOCKET_CONNECTED_STATE     = 2

}rsi_wlan_app_state_t;

//! wlan application control block
typedef struct rsi_wlan_app_cb_s
{
  //! wlan application state 
  rsi_wlan_app_state_t state;

  //! length of buffer to copy
  uint32_t length;

  //! application buffer
  uint8_t buffer[RSI_APP_BUF_SIZE];

  //! to check application buffer availability
  uint8_t buf_in_use;

  //! application events bit map 
  uint32_t event_map; 

}rsi_wlan_app_cb_t;

//! application control block
rsi_wlan_app_cb_t rsi_wlan_app_cb;

//! Enumeration for commands used in application
typedef enum rsi_app_cmd_e
{
  RSI_DATA = 0
}rsi_app_cmd_t;

//! Function prototypes
extern void rsi_wlan_ap_app_task(void);
extern int32_t rsi_ble_app_send_to_wlan(uint8_t msg_type,uint8_t *buffer, uint32_t length);
extern void rsi_wlan_async_data_recv(uint32_t sock_no, const struct sockaddr* addr, uint8_t *buffer, uint32_t length);
extern void rsi_wlan_app_callbacks_init(void);
extern void rsi_stations_connect_notify_handler(uint16_t status,const uint8_t *buffer, const uint16_t length);
extern void rsi_stations_disconnect_notify_handler(uint16_t status,const uint8_t *buffer, const uint16_t length);


void rsi_wlan_app_callbacks_init(void)
{

  //! Initialze ip change notify call back
  rsi_wlan_register_callbacks(RSI_STATIONS_CONNECT_NOTIFY_CB, rsi_stations_connect_notify_handler);

  //! Initialze ip change notify call back
  rsi_wlan_register_callbacks(RSI_STATIONS_DISCONNECT_NOTIFY_CB, rsi_stations_disconnect_notify_handler);
}

#ifdef RSI_WITH_OS
extern rsi_semaphore_handle_t wlan_thread_sem, ble_thread_sem;
#endif

void  rsi_wlan_ap_app_task(void)
{
  int32_t     status = RSI_SUCCESS;
#ifdef RSI_WITH_OS
	while(1)
	{
#endif
  switch(rsi_wlan_app_cb.state)
  {
    case RSI_WLAN_INITIAL_STATE:
      {
        //! register call backs
        rsi_wlan_app_callbacks_init();

        //! Configure IP 
        status = rsi_config_ipaddress(RSI_IP_VERSION_4, RSI_STATIC, (uint8_t *)&ip_addr, (uint8_t *)&network_mask, (uint8_t *)&gateway, NULL, 0,0);
        if(status != RSI_SUCCESS)
        {
          break;
        }

        //! Start Access point
        status =  rsi_wlan_ap_start((int8_t *)SSID, CHANNEL_NO, SECURITY_TYPE, ENCRYPTION_TYPE, PSK, BEACON_INTERVAL, DTIM_COUNT);
        if(status != RSI_SUCCESS)
        {
          break;
        }

        else
        {
          //! update wlan application state
          rsi_wlan_app_cb.state = RSI_WLAN_AP_UP_STATE; 
        }
      }
    case RSI_WLAN_AP_UP_STATE:
      { 


        //! Create socket
        server_socket = socket_async(AF_INET, SOCK_STREAM, 0, rsi_wlan_async_data_recv);
        if(server_socket < 0)
        {
          status = rsi_wlan_get_status();
          break; 
        }

        //! Set server structure
        memset(&server_addr, 0, sizeof(server_addr));

        //! Set family type
        server_addr.sin_family= AF_INET;

        //! Set local port number
        server_addr.sin_port = htons(DEVICE_PORT);



        //! Bind socket
        status = bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr));
        if(status != RSI_SUCCESS)
        {
          status = rsi_wlan_get_status();
          shutdown(server_socket, 0);
          break; 
        }

        //! Socket listen
        status = listen(server_socket, 1);
        if(status != RSI_SUCCESS)
        {
          status = rsi_wlan_get_status();
          shutdown(server_socket, 0);
          break; 
        }

        addr_size = sizeof(server_socket);

        //! Socket accept
        new_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size);
        if(new_socket < 0)
        {
          status = rsi_wlan_get_status();
          shutdown(server_socket, 0);
          break; 
        }
        else
        {
          //! update wlan application state
          rsi_wlan_app_cb.state = RSI_WLAN_SOCKET_CONNECTED_STATE; 
        }
      }
    case RSI_WLAN_SOCKET_CONNECTED_STATE:
      {
        if( rsi_wlan_app_cb.event_map & RSI_SEND_EVENT)
        {
          //! send data on socket
          status = send(new_socket, (const int8_t *)rsi_wlan_app_cb.buffer, rsi_wlan_app_cb.length, 0);
          if(status < 0)
          {
            status = rsi_wlan_get_status();
            break;
          }
          else
          {
            //! make buffer in use
            rsi_wlan_app_cb.buf_in_use = 0;

            //! clear send event after data send is successful
            rsi_wlan_app_cb.event_map &= ~(RSI_SEND_EVENT);

          }
        }
        else
        {
#ifdef RSI_WITH_OS
        	rsi_semaphore_wait(&wlan_thread_sem,0);
#endif
        }
      }
    default:
      break;
  }
#ifdef RSI_WITH_OS
	}
#endif
}

int32_t rsi_ble_app_send_to_wlan(uint8_t msg_type, uint8_t *buffer, uint32_t length)
{
  switch(msg_type)
  {
    case RSI_DATA:
      {
        //! buffer is in use or not
        if(!rsi_wlan_app_cb.buf_in_use)
        {
          //! if not in use

          //! copy the buffer to wlan app cb tx buffer
          memcpy(rsi_wlan_app_cb.buffer, buffer, length); 

          //! hold length information
          rsi_wlan_app_cb.length = length;

          //! make buffer in use
          rsi_wlan_app_cb.buf_in_use = 1;

          //! raise event to wlan app task
          rsi_wlan_app_cb.event_map |= RSI_SEND_EVENT;
#ifdef RSI_WITH_OS
          rsi_semaphore_post(&wlan_thread_sem);
#endif
        }
        else
          //!if buffer is in use
        {
          return -1;
          //! return error 
        }
      }
  }
return 0;
}

void rsi_wlan_async_data_recv(uint32_t sock_no, const struct sockaddr* addr, uint8_t *buffer, uint32_t length)
{
  //! send packet to ble
  rsi_wlan_app_send_to_ble(RSI_DATA, buffer, length);
}


//! callback functions

//! stations connect notify call back handler in AP mode
void rsi_stations_connect_notify_handler(uint16_t status, const uint8_t *buffer, const uint16_t length)
{

	//! increment connected stations count
    rsi_stations_count++;

}

//! stations disconnect notify call back handler in AP mode
void rsi_stations_disconnect_notify_handler(uint16_t status, const uint8_t *buffer, const uint16_t length)
{
   //! decrement connected stations count
   rsi_stations_count--;
}

