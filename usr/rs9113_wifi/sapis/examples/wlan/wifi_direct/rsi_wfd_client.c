/**
 * @file    rsi_wfd_client.c
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
 *  @brief : This file contains example application for TCP server socket in WiFi-Direct client mode
 *
 *  @section Description  This file contains example application for TCP server socket WiFi-Direct client in mode
 *
 *
 */

/**
 * Include files
 * */
//! include file to refer data types
#include "rsi_data_types.h"

//! COMMON include file to refer wlan APIs
#include "rsi_common_apis.h"

//! WLAN include file to refer wlan APIs
#include "rsi_wlan_apis.h"

//! socket include file to refer socket APIs
#include "rsi_socket.h"

//! Error include files
#include "rsi_error.h"

//! OS include file to refer OS specific functionality
#include "rsi_os.h"
//! connection type
#define CONNECTION_TYPE    SEND_CONNECTION_REQ

//! Wifi-Direct device to connect
#define REMOTE_DEVICE       "WFD_GO"

//! Wifi-Direct device name
#define RSI_DEVICE_NAME       "REDPINE_WFD"

//! Post fix SSID
#define POST_FIX_SSID       "REDPINE_DEV"

//! GO Intent
#define GO_INTENT          0

//! operating channel
#define CHANNEL            11

//! Password
#define PSK               "1234567890"

//! Device port number
#define DEVICE_PORT       5001

//! DHCP mode 1- Enable 0- Disable
#define DHCP_MODE         1 

//! If DHCP mode is disabled given IP statically
#if !(DHCP_MODE)

//! IP address of the module 
//! E.g: 0x650AA8C0 == 192.168.10.101
#define DEVICE_IP         0x650AA8C0

//! IP address of Gateway
//! E.g: 0x010AA8C0 == 192.168.10.1
#define GATEWAY           0x010AA8C0

//! IP address of netmask
//! E.g: 0x00FFFFFF == 255.255.255.0
#define NETMASK           0x00FFFFFF 

#endif


//! accept connection req
#define ACCEPT_CONNECTION_REQ   0

//! send connection req 
#define SEND_CONNECTION_REQ      1


//! Number of packets to receive
#define NUMBER_OF_PACKETS  1000

//! Receive data length
#define RECV_BUFFER_SIZE   1000 

//! Memory length for driver
#define GLOBAL_BUFF_LEN   8000

//! Wlan task priority
#define RSI_WLAN_TASK_PRIORITY   1

//! Wireless driver task priority
#define RSI_DRIVER_TASK_PRIORITY   2

//! Wlan task stack size
#define RSI_WLAN_TASK_STACK_SIZE  500

//! Wireless driver task stack size
#define RSI_DRIVER_TASK_STACK_SIZE  500

//! Memory to initialize driver
uint8_t global_buf[GLOBAL_BUFF_LEN];
uint8_t device_name[30];
volatile uint8_t connection_event;
volatile int8_t  join_rsp_received;
extern void rsi_wlan_app_callbacks_init(void);
extern void rsi_wlan_wfd_connection_request_notify_handler(uint16_t status,const uint8_t *buffer, const uint16_t length);
extern void rsi_wlan_wfd_discovery_notify_handler(uint16_t status, const uint8_t *buffer, const uint16_t length);
extern void rsi_join_response_handler(uint16_t status,const uint8_t *buffer, const uint16_t length);

void rsi_wlan_app_callbacks_init(void)
{
  //! Initialze wfd discovery notify callback
  rsi_wlan_register_callbacks(RSI_WLAN_WFD_DISCOVERY_NOTIFY_CB, rsi_wlan_wfd_discovery_notify_handler);

  //! Initialze wfd connection request notify callback
  rsi_wlan_register_callbacks(RSI_WLAN_WFD_CONNECTION_REQUEST_NOTIFY_CB, rsi_wlan_wfd_connection_request_notify_handler);
}

int32_t rsi_wfd_client()
{
  int32_t     server_socket, new_socket;
  struct      sockaddr_in server_addr, client_addr;
  int32_t     status       = RSI_SUCCESS;
  int32_t     packet_count = 0, recv_size  = 0, addr_size;
#if !(DHCP_MODE)
  uint32_t    ip_addr      = DEVICE_IP;
  uint32_t    network_mask = NETMASK;
  uint32_t    gateway      = GATEWAY;
#endif
  //! buffer to receive data over TCP client socket
  int8_t recv_buffer[RECV_BUFFER_SIZE];

  //! WC initialization
  status = rsi_wireless_init(1, 0);
  if(status != RSI_SUCCESS)
  {
    return status;
  }

  //! register call backs
  rsi_wlan_app_callbacks_init();

  
  //! Configure WiFi-Direct Device and start discovery 
  status = rsi_wlan_wfd_start_discovery(GO_INTENT, RSI_DEVICE_NAME,CHANNEL, POST_FIX_SSID , PSK,
                                        rsi_wlan_wfd_discovery_notify_handler,
                                        rsi_wlan_wfd_connection_request_notify_handler);
  do
  {
#ifndef RSI_WITH_OS
    //! event loop 
    rsi_wireless_driver_task();
#endif
  }while(!connection_event);


  status = rsi_wlan_wfd_connect(device_name, rsi_join_response_handler);
  if(status != RSI_SUCCESS)
  {
    return status;
  }

  do
  {
#ifndef RSI_WITH_OS
    //! event loop 
    rsi_wireless_driver_task();
#endif
  
  }while(!join_rsp_received);


  if(join_rsp_received != 1)
  {
   return  rsi_wlan_get_status();
  }

  //! Configure IP 
#if DHCP_MODE
  status = rsi_config_ipaddress(RSI_IP_VERSION_4, RSI_DHCP,0, 0, 0, NULL, 0,0);
#else
  status = rsi_config_ipaddress(RSI_IP_VERSION_4, RSI_STATIC, (uint8_t *)&ip_addr, (uint8_t *)&network_mask, (uint8_t *)&gateway, NULL, 0,0);
#endif
  if(status != RSI_SUCCESS)
  {
    return status;
  }

  //! Create socket
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if(server_socket < 0)
  {
    status = rsi_wlan_get_status();
    return status;
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
    return status;
  }

  //! Socket listen
  status = listen(server_socket, 1);
  if(status != RSI_SUCCESS)
  {
    status = rsi_wlan_get_status();
    shutdown(server_socket, 0);
    return status;
  }

  addr_size = sizeof(server_socket);

  //! Socket accept
  new_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size);
  if(new_socket < 0)
  {
    status = rsi_wlan_get_status();
    shutdown(server_socket, 0);
    return status;
  }

  while(1)//packet_count < NUMBER_OF_PACKETS)
  {
    recv_size = RECV_BUFFER_SIZE;

    do
    {
      //! Receive data on socket
      status = recvfrom(new_socket, recv_buffer, recv_size, 0, (struct sockaddr *)&client_addr, &addr_size);
      if(status < 0)
      {
        status = rsi_wlan_get_status();
        shutdown(server_socket, 0);
        return status;
      }

      //! subtract received bytes 
      recv_size -= status;

    } while(recv_size > 0);

    packet_count++;
  }
  return 0;
}

void main_loop(void)
{
  while(1)
  {
    ////////////////////////
    //! Application code ///
    ////////////////////////

    //! event loop 
    rsi_wireless_driver_task();

  }
}

int main()
{
  int32_t status;
#ifdef RSI_WITH_OS

  rsi_task_handle_t wlan_task_handle = NULL;

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

#ifdef RSI_WITH_OS
  //! OS case
  //! Task created for WLAN task
  rsi_task_create(rsi_wfd_client, "wlan_task", RSI_WLAN_TASK_STACK_SIZE, NULL, RSI_WLAN_TASK_PRIORITY, &wlan_task_handle);

  //! Task created for Driver task
  rsi_task_create(rsi_wireless_driver_task, "driver_task",RSI_DRIVER_TASK_STACK_SIZE, NULL, RSI_DRIVER_TASK_PRIORITY, &driver_task_handle);

  //! OS TAsk Start the scheduler
  rsi_start_os_scheduler();

#else
  //! NON - OS case
  //! Call wifidirect application
  status = rsi_wfd_client();

  //! Application main loop
  main_loop();
#endif
  return status;

}

//! callback functions

//! Parse WFD Device discovery response 
void rsi_wlan_wfd_discovery_notify_handler(uint16_t status, const uint8_t *buffer, const uint16_t length)
{
  rsi_rsp_wfd_device_info_t  wfd_dev_rsp;
  uint8_t ii;

  wfd_dev_rsp.device_count = length/41;
  /* 1 byte(devState) + 32 byte(devName) + 6 byte(macAddress) + 2 byte (devtype)*/

  for(ii=0; ii< wfd_dev_rsp.device_count; ii++)
  {
    wfd_dev_rsp.wfd_dev_info[ii].device_state = *(buffer);
    buffer += 1;
    memcpy(wfd_dev_rsp.wfd_dev_info[ii].device_name, buffer, 32);
    buffer += 32;
    memcpy(wfd_dev_rsp.wfd_dev_info[ii].mac_address, buffer, 6);
    buffer += 6;
    memcpy(wfd_dev_rsp.wfd_dev_info[ii].device_type, buffer, 2);
    buffer += 2;
  }


  for(ii=0; ii< wfd_dev_rsp.device_count; ii++)
  {
    if(!(strcmp((void *)wfd_dev_rsp.wfd_dev_info[ii].device_name, REMOTE_DEVICE)))
    {
      if(CONNECTION_TYPE == SEND_CONNECTION_REQ)
      {
        strcpy(device_name,REMOTE_DEVICE);
        connection_event = 1; 
      }
    }
  }

}

//! Notify connection request to Host
void rsi_wlan_wfd_connection_request_notify_handler(uint16_t status, const uint8_t *buffer, const uint16_t length)
{
  if(CONNECTION_TYPE == ACCEPT_CONNECTION_REQ)
  {
    strcpy(device_name,buffer);
    connection_event = 1;
  }

}

void rsi_join_response_handler(uint16_t status,const uint8_t *buffer, const uint16_t length)
{
  if(status == 0)
  {
    join_rsp_received = 1;
  }
  else
  {
    join_rsp_received = 2;
  }

}
