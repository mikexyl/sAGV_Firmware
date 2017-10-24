/**
 * @file    rsi_smtp_client_app.c
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
 *  @brief : This file contains example application for smtp client functionality 
 *
 *  @section Description  This file contains example application for smtp client functionality 
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
#include "rsi_smtp_client.h"

//! socket include file to refer socket APIs
#include "rsi_socket.h"

//! Error include files
#include "rsi_error.h"

//! OS include file to refer OS specific functionality
#include "rsi_os.h"

#include "string.h"
//! configurattion Parameters

//! Access point SSID to connect
#define SSID              "REDPINE_AP"

//!Scan Channel number , 0 - to scan all channels
#define CHANNEL_NO              0

//! Security type
#define SECURITY_TYPE     RSI_OPEN

//! Password
#define PSK               NULL

//! flags
#define FLAGS                  0

//! smtp port no
#define SMTP_PORT              25 

//! username
#define USERNAME               "testuser"

//! password
#define PASSWORD               "password" 

//! from address
#define FROM_ADDRESS         "abc@mail.local"
 
//! server ip
#define SERVER_IP               0x9200A8C0 
  
//! authentication type
#define AUTH_TYPE              RSI_SMTP_CLIENT_AUTH_LOGIN         

//! priority
#define PRIORITY               RSI_SMTP_MAIL_PRIORITY_NORMAL

//! domain name
#define CLIENT_DOMAIN           "mymail.mail.com"    

//! mail recepient address
#define MAIL_RECIPIENT_ADDRESS  "test@mail.local"

//! mail subject
#define  MAIL_SUBJECT           "TEST"

//! message
#define  MAIL_BODY              "TEST BODY"


//! DHCP mode 1- Enable 0- Disable
#define DHCP_MODE         1 

//! If DHCP mode is disabled given IP statically
#if !(DHCP_MODE)

//! IP address of the module 
//! E.g: 0x650AA8C0 == 192.168.10.101
#define DEVICE_IP              0x650AA8C0

//! IP address of Gateway
//! E.g: 0x010AA8C0 == 192.168.10.1
#define GATEWAY                0x010AA8C0

//! IP address of netmask
//! E.g: 0x00FFFFFF == 255.255.255.0
#define NETMASK                0x00FFFFFF 

#endif

//! Memory length for driver
#define GLOBAL_BUFF_LEN        8000

//! Wlan task priority
#define RSI_WLAN_TASK_PRIORITY   1

//! Wireless driver task priority
#define RSI_DRIVER_TASK_PRIORITY   2

//! Wlan task stack size
#define RSI_WLAN_TASK_STACK_SIZE  500

//! Wireless driver task stack size
#define RSI_DRIVER_TASK_STACK_SIZE  500

//! standard defines
volatile uint8_t  rsp_received;

//! Memory to initialize driver
uint8_t global_buf[GLOBAL_BUFF_LEN];

//! Function prototypes
void main_loop(void);
void rsi_smtp_client_mail_send_response_handler(uint16_t status, const uint8_t cmd_type);
void rsi_smtp_client_delete_response_handler(uint16_t status, const uint8_t cmd_type);

//! smtp client get Application 
int32_t rsi_smtp_client_app()
{
  int32_t     status       = RSI_SUCCESS;
#if !(DHCP_MODE)
  uint32_t    ip_addr      = DEVICE_IP;
  uint32_t    network_mask = NETMASK;
  uint32_t    gateway      = GATEWAY;
#endif
  uint32_t     server_ip   = SERVER_IP;

  //! WC initialization
  status = rsi_wireless_init(0, 0);
  if(status != RSI_SUCCESS)
  {
    return status;
  }

  //! Scan for Access points
  status = rsi_wlan_scan((int8_t *)SSID, (uint8_t)CHANNEL_NO, NULL, 0);
  if(status != RSI_SUCCESS)
  {
    return status;
  }

  //! Connect to an Access point
  status = rsi_wlan_connect((int8_t *)SSID, SECURITY_TYPE, PSK);
  if(status != RSI_SUCCESS)
  {
    return status;
  }

  //! Configure IP 
#if DHCP_MODE
  status = rsi_config_ipaddress(RSI_IP_VERSION_4, RSI_DHCP, 0, 0, 0, NULL, 0,0);
#else
  status = rsi_config_ipaddress(RSI_IP_VERSION_4, RSI_STATIC, (uint8_t *)&ip_addr, (uint8_t *)&network_mask, (uint8_t *)&gateway, NULL, 0,0);
#endif
  if(status != RSI_SUCCESS)
  {
    return status;
  }

  //! create smtp client
  status = rsi_smtp_client_create((uint8_t )FLAGS, (uint8_t *)USERNAME, (uint8_t *)PASSWORD,(uint8_t *)FROM_ADDRESS, 
      (uint8_t *)CLIENT_DOMAIN, (uint8_t)AUTH_TYPE, (uint8_t *)&server_ip, (uint16_t)SMTP_PORT);
  if(status != RSI_SUCCESS)
  {
    return status;
  }

  //! send mail to a SMTP server from our client
  status = rsi_smtp_client_mail_send_async((uint8_t *)MAIL_RECIPIENT_ADDRESS, (uint8_t)PRIORITY, (uint8_t *)MAIL_SUBJECT,
      (uint8_t *)MAIL_BODY, strlen((const char*)MAIL_BODY),rsi_smtp_client_mail_send_response_handler); 
  if(status != RSI_SUCCESS)
  {
    return status;
  }

  //! wait for the success response 
  do
  {
    //! event loop 
#ifndef RSI_WITH_OS
    rsi_wireless_driver_task();
#endif

  }while(!rsp_received);

  if(rsp_received != 1)
  {
   return  rsi_wlan_get_status();
  }

  //! reset rsp received
  rsp_received = 0;

  //! send smtp client delete request
  status = rsi_smtp_client_delete_async(rsi_smtp_client_delete_response_handler);

  //! wait for the success response 
  do
  {
#ifndef RSI_WITH_OS
    //! event loop 
    rsi_wireless_driver_task();
#endif

  }while(!rsp_received);

  if(rsp_received != 1)
  {
   return  rsi_wlan_get_status();
  }
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
  rsi_task_create(rsi_smtp_client_app, "wlan_task", RSI_WLAN_TASK_STACK_SIZE, NULL, RSI_WLAN_TASK_PRIORITY, &wlan_task_handle);

  //! Task created for Driver task
  rsi_task_create(rsi_wireless_driver_task, "driver_task",RSI_DRIVER_TASK_STACK_SIZE, NULL, RSI_DRIVER_TASK_PRIORITY, &driver_task_handle);

  //! OS TAsk Start the scheduler
  rsi_start_os_scheduler();

#else
  //! NON - OS case
  //! Call smtp client application
  status = rsi_smtp_client_app();

  //! Application main loop
  main_loop();
#endif
  return status;

}

//! smtp client mail send response handler 
void rsi_smtp_client_mail_send_response_handler(uint16_t status, const uint8_t cmd_type)
{
  if(status == RSI_SUCCESS)
  {
    rsp_received = 1;
  }
  else
  {
    rsp_received = 2;
  }

}

//! smtp client delete response handler 
void rsi_smtp_client_delete_response_handler(uint16_t status, const uint8_t cmd_type)
{
  if(status == RSI_SUCCESS)
  {
    rsp_received = 1;
  }
  else
  {
    rsp_received = 2;
  }
}
