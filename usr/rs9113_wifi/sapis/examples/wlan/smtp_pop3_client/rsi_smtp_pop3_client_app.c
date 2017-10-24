/**
 * @file    rsi_pop3_client_app.c
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
 *  @brief : This file contains example application for pop3 client functionality 
 *
 *  @section Description  This file contains example application for pop3 client functionality 
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
#include "rsi_pop3_client.h"

//! socket include file to refer socket APIs
#include "rsi_socket.h"

//! socket include file to refer socket APIs
#include "rsi_smtp_client.h"

//! Error include files
#include "rsi_error.h"

//! OS include file to refer OS specific functionality
#include "rsi_os.h"

#include "rsi_nwk.h"

//! configurattion Parameters

//! Access point SSID to connect
#define SSID              "REDPINE_AP"

//!Scan Channel number , 0 - to scan all channels
#define CHANNEL_NO              0

//! Security type
#define SECURITY_TYPE     RSI_OPEN

//! Password
#define PSK               ""

//! flags
#define FLAGS                  0

//! POP3 authentication type
#define POP3_AUTH_TYPE         0

//! pop3 server port no
#define POP3_PORT              110

//! username
#define POP3_USERNAME               "username" 

//! password
#define POP3_PASSWORD               "test123" 

//! server ip
#define SERVER_IP               0x5CA1C4CB 
  
//! smtp port no
#define SMTP_PORT              25 

//! username
#define SMTP_USERNAME          "username" 

//! password
#define SMTP_PASSWORD          "test123" 

//! from address
#define FROM_ADDRESS           "from@mail.local"
 
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


//! mail index
#define MAIL_INDEX              200

//! End of mail 
#define EOM                     3

//! DHCP mode 1- Enable 0- Disable
#define DHCP_MODE         1

//! If DHCP mode is disabled given IP statically
#if !(DHCP_MODE)

//! IP address of the module 
//! E.g: 0x650AA8C0 == 192.168.10.101
#define DEVICE_IP              0x6502A8C0

//! IP address of Gateway
//! E.g: 0x010AA8C0 == 192.168.10.1
#define GATEWAY                0x0102A8C0

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

rsi_pop3_client_resp_t pop3_resp;


rsi_pop3_mail_data_resp_t pop3_mail_content_resp;

//! Memory to initialize driver
uint8_t global_buf[GLOBAL_BUFF_LEN];

//! Function prototypes
void main_loop(void);
void rsi_pop3_client_mail_response_handler(uint16_t status, uint8_t cmd_type, const uint8_t *buffer);
void rsi_smtp_client_mail_send_response_handler(uint16_t status, const uint8_t *buffer, const uint16_t length);
void rsi_smtp_client_delete_response_handler(uint16_t status, const uint8_t *buffer, const uint16_t length);

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
  status = rsi_smtp_client_create((uint8_t )FLAGS, (uint8_t *)SMTP_USERNAME, (uint8_t *)SMTP_PASSWORD,(uint8_t *)FROM_ADDRESS, 
      (uint8_t *)CLIENT_DOMAIN, (uint8_t)AUTH_TYPE, (uint8_t *)&server_ip, (uint16_t)SMTP_PORT);
  if(status != RSI_SUCCESS)
  {
    return status;
  }

  //! send mail to a SMTP server from our client
  status = rsi_smtp_client_mail_send_async((uint8_t *)MAIL_RECIPIENT_ADDRESS, (uint8_t)PRIORITY, (uint8_t *)MAIL_SUBJECT,
      (uint8_t *)MAIL_BODY, strlen(MAIL_BODY),rsi_smtp_client_mail_send_response_handler); 
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
  return 0;
}




//! pop3 client mail dowmload Application 
int32_t rsi_pop3_client_app()
{
  int32_t     status       = RSI_SUCCESS;
#if !(DHCP_MODE)
  uint32_t    ip_addr      = DEVICE_IP;
  uint32_t    network_mask = NETMASK;
  uint32_t    gateway      = GATEWAY;
#endif
  uint32_t     server_ip   = SERVER_IP;

  rsp_received = 0;

  //! Create pop3 client session
  status = rsi_pop3_session_create_async((uint8_t )FLAGS, (uint8_t *)&server_ip, (uint16_t)POP3_PORT, (uint8_t)POP3_AUTH_TYPE, 
      (uint8_t *)POP3_USERNAME, (uint8_t *)POP3_PASSWORD, rsi_pop3_client_mail_response_handler);
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

  //! Get mail stats from POP3 server
  status = rsi_pop3_get_mail_stats(); 
  if(status != RSI_SUCCESS)
  {
    return status;
  }

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

  //! reset rsp received
  rsp_received = 0;

  //! Get mail list
  status = rsi_pop3_get_mail_list(*(uint16_t *) pop3_resp.mail_count);

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

  //! reset rsp received
  rsp_received = 0;

  //! Get mail content
  status = rsi_pop3_retrive_mail(*(uint16_t *) pop3_resp.mail_count);
  
  //! wait for the success response 
  do
  {
    //! event loop 
#ifndef RSI_WITH_OS
    rsi_wireless_driver_task();
#endif

    //! "pop3_mail_content_resp" structure varible contains the mail content response

  }while(rsp_received != EOM);

  if(rsp_received == 2)
  {
   return  rsi_wlan_get_status();
  }

  //! reset rsp received
  rsp_received = 0;

  //! POP3 mark mail request
  status = rsi_pop3_mark_mail(*(uint16_t *) pop3_resp.mail_count); 

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

  //! reset rsp received
  rsp_received = 0;

  //! Get POP3 server status
  status = rsi_pop3_get_server_status();

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

  //! reset rsp received
  rsp_received = 0;

  //! Delete POP3 client session
  status = rsi_pop3_session_delete();

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
  
  //! reset rsp received
  rsp_received = 0;

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
  int32_t status = 0;
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

  //! OS case
  //! Task created for WLAN task
  rsi_task_create(rsi_pop3_client_app, "wlan_task", RSI_WLAN_TASK_STACK_SIZE, NULL, RSI_WLAN_TASK_PRIORITY, &wlan_task_handle);

  //! Task created for Driver task
  rsi_task_create(rsi_wireless_driver_task, "driver_task",RSI_DRIVER_TASK_STACK_SIZE, NULL, RSI_DRIVER_TASK_PRIORITY, &driver_task_handle);

  //! OS Task Start the scheduler
  rsi_start_os_scheduler();

#else


  //! NON - OS case
  //! Call smtp client application
  status = rsi_smtp_client_app();
  if(status != RSI_SUCCESS)
  {
    return status;
  }

  //! NON - OS case
  //! Call pop3 client application
  status = rsi_pop3_client_app();
  if(status != RSI_SUCCESS)
  {
    return status;
  }

  //! Application main loop
  main_loop();
#endif
  return status;

}

//! pop3 client mail response handler 
void rsi_pop3_client_mail_response_handler(uint16_t status, uint8_t cmd_type, const uint8_t *buffer)
{

  if(status == RSI_SUCCESS)
  {
    rsp_received = 1;

    switch(cmd_type)
    {
      case POP3_CLIENT_SESSION_CREATE:
        {

        }
        break;
      case POP3_CLIENT_GET_MAIL_STATS:
        {
          memcpy(&pop3_resp, buffer, sizeof(rsi_pop3_client_resp_t)); 
        }
        break;
      case POP3_CLIENT_GET_MAIL_LIST:
        {
          //memcpy(&pop3_resp, buffer, sizeof(rsi_pop3_client_resp_t)); 
        }
        break;
      case POP3_CLIENT_RETR_MAIL:
        {
          memcpy(&pop3_mail_content_resp, buffer, sizeof(rsi_pop3_mail_data_resp_t)); 

          if(pop3_mail_content_resp.more)
          {
            //! End of the mail content chunk
            rsp_received = EOM ;
          }
          else
          {
            rsp_received = 1;
          }

        }
        break;

      case POP3_CLIENT_MARK_MAIL:
        {

        }
        break;
      case POP3_CLIENT_UNMARK_MAIL:
        {

        }
        break;
      case POP3_CLIENT_SESSION_DELETE:
        {


        }
        break;
Default:
        break; 
    }
  }
  else
  {
    rsp_received = 2;
  }
}


//! smtp client mail send response handler 
void rsi_smtp_client_mail_send_response_handler(uint16_t status, const uint8_t *buffer, const uint16_t length)
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
void rsi_smtp_client_delete_response_handler(uint16_t status, const uint8_t *buffer, const uint16_t length)
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
