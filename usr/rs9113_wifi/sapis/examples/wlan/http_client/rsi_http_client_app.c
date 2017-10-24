/**
 * @file    rsi_http_client_app.c
 * @version 0.1
 * @date    2 Feb 2016
 *
 *  Copyright(C) Redpine Signals 2015
 *  All rights reserved by Redpine Signals.
 *
 *  @section License
 *  This program should be used on your own responsibility.
 *  Redpine Signals assumes no responsibility for any losses
 *  incurred by customers or third parties arising from the use of this file.
 *
 *  @brief : This file contains example application for HTTP client functionality 
 *
 *  @section Description  This file contains example application for HTTP client functionality 
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
#include "rsi_http_client.h"

//! socket include file to refer socket APIs
#include "rsi_socket.h"

#include "rsi_nwk.h"
//! Error include files
#include "rsi_error.h"

//! OS include file to refer OS specific functionality
#include "rsi_os.h"

//! include the certificate 
#include "cacert.pem"

//! include the index html page
#include "index.txt"

//! standard defines

//! to Enable IPv6 set this bit in FLAGS, Default is IPv4 
#define   HTTPV6          1

//! Set HTTPS_SUPPORT to use HTTPS feature
#define   HTTPS_SUPPORT   2

//! Set HTTPS_SUPPORT to use HTTPS feature
#define   HTTPS_SUPPORT   2

//! Set HTTP_POST_DATA to use HTTP POST LARGE DATA feature
#define   HTTP_POST_DATA  32

//! Set HTTP_V_1_1 to use HTTP version 1.1
#define   HTTP_V_1_1      64

//! configurattion Parameters

//! Access point SSID to connect
#define SSID              "REDPINE_AP"

//!Scan Channel number , 0 - to scan all channels
#define CHANNEL_NO              0

//! Security type
#define SECURITY_TYPE     RSI_OPEN

//! Password
#define PSK               NULL

//! Flags
//! for example select required flag bits,  Eg:(HTTPS_SUPPORT | HTTPV6)
#define FLAGS         0  

//! Server port number
#define HTTP_PORT            80

//! HTTP Server IP address.
#define HTTP_SERVER_IP_ADDRESS "192.168.0.111"

//! HTTP resource name
#define HTTP_URL           "/index.html"           

//! set HTTP hostname
#define HTTP_HOSTNAME     "192.168.0.111" 

//! set HTTP extended header
//! if NULL , driver fills default extended header
#define HTTP_EXTENDED_HEADER    NULL  

//! set HTTP hostname
#define USERNAME               "admin" 

//! set HTTP hostname
#define PASSWORD               "admin" 

//! HTTP post data
#define HTTP_DATA          "employee_name=MR.REDDY&employee_id=RSXYZ123&designation=Engineer&company=REDPINE&location=Hyderabad"

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

//! Load certificate to device flash :
//! Certificate could be loaded once and need not be loaded for every boot up
#define LOAD_CERTIFICATE  1

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

//! Application buffer length
#define APP_BUFF_LEN           2000

//! Max HTTP PUT buffer length
#define MAX_HTTP_CLIENT_PUT_BUFFER_LENGTH 900

//! HTTP response received
volatile uint8_t  rsp_received;

//! End of http put file/content
volatile uint8_t  end_of_file = 0;
//! http get receive done
volatile uint8_t http_recv_done;

//! data buffer length
uint32_t buf_len;

//! Application Buffer
uint8_t app_buf[APP_BUFF_LEN];

//! Memory to initialize driver
uint8_t global_buf[GLOBAL_BUFF_LEN];

//! prototypes
void rsi_http_client_put_response_handler(uint16_t status, const uint8_t type, const uint8_t end_of_put_pkt);
void rsi_http_get_response_handler(uint16_t status, const uint8_t *buffer, const uint16_t length, uint32_t moredata);
void rsi_http_post_response_handler(uint16_t status, const uint8_t *buffer, const uint16_t length, uint32_t moredata);

//! Http client Application 
int32_t rsi_http_client_app()
{
  int32_t     status       = RSI_SUCCESS;
#if !(DHCP_MODE)
  uint32_t    ip_addr      = DEVICE_IP;
  uint32_t    network_mask = NETMASK;
  uint32_t    gateway      = GATEWAY;
#endif
  uint8_t      flags       = FLAGS;

  uint32_t offset          = 0;
  uint32_t chunk_length    = 0;
  uint8_t  *file_content   = NULL;
  
  //! Get the length of the index html page
  uint32_t file_size = (sizeof(index) - 1);

  //! WC initialization
  status = rsi_wireless_init(0, 0);
  if(status != RSI_SUCCESS)
  {
    return status;
  }

#if LOAD_CERTIFICATE
  if(flags & HTTPS_SUPPORT)
  {
    //! Load certificates				
    status  = rsi_wlan_set_certificate(5, cacert, (sizeof(cacert)-1));
    if(status != RSI_SUCCESS)
    {
      return status;
    }
  }
#endif          
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

  //! Create HTTP client for PUT method
  status = rsi_http_client_put_create();
  if(status != RSI_SUCCESS)
  {
    return status;
  }

  //! Start HTTP client PUT method for the given URL
  status = rsi_http_client_put_start((uint8_t)flags, (uint8_t *)HTTP_SERVER_IP_ADDRESS, (uint32_t)HTTP_PORT, 
      (uint8_t *)HTTP_URL, (uint8_t *)HTTP_HOSTNAME,
      (uint8_t *)HTTP_EXTENDED_HEADER, (uint8_t *)USERNAME, (uint8_t *)PASSWORD, (uint32_t)(sizeof(index) - 1),
      rsi_http_client_put_response_handler);

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

  rsp_received = 0;

  //! HTTP client PUT packet
  while(!end_of_file)
  {
    //! Get the current length that you want to send
    chunk_length = ((file_size - offset) > MAX_HTTP_CLIENT_PUT_BUFFER_LENGTH) 
      ? MAX_HTTP_CLIENT_PUT_BUFFER_LENGTH : (file_size - offset);

    file_content = (uint8_t *)(index + offset);

    //! Send resource content to the HTTP server for the given URL and total content length 
    status = rsi_http_client_put_pkt((uint8_t *)file_content, (uint16_t)chunk_length);

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

    rsp_received = 0;

    //! Move content offset by chunk length
    offset += chunk_length;

    rsp_received = 0;
  }

  //! Delete HTTP Client
  status = rsi_http_client_put_delete();


  //! wait for Driver task to complete
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

  rsp_received = 0;


  //! send http get request for the given url
  status = rsi_http_client_get_async((uint8_t)flags, (uint8_t *)HTTP_SERVER_IP_ADDRESS, (uint16_t)HTTP_PORT, 
      (uint8_t *)HTTP_URL, (uint8_t *)HTTP_HOSTNAME,
      (uint8_t *)HTTP_EXTENDED_HEADER, (uint8_t *)USERNAME, (uint8_t *)PASSWORD,
      rsi_http_get_response_handler);

  //! wait for Driver task to complete
  do
  {
    //! event loop 
    rsi_wireless_driver_task();

  }while(!http_recv_done);


  //! resetting buffer length
  buf_len = 0;

  //! resetting http recv done
  http_recv_done = 0;

  //! send http post request for the given url with given http data
  status = rsi_http_client_post_async((uint8_t)FLAGS, (uint8_t *)HTTP_SERVER_IP_ADDRESS, (uint16_t)HTTP_PORT, 
      (uint8_t *)HTTP_URL, (uint8_t *)HTTP_HOSTNAME,
      (uint8_t *)HTTP_EXTENDED_HEADER, (uint8_t *)USERNAME,
      (uint8_t *)PASSWORD,(uint8_t *)HTTP_DATA , strlen(HTTP_DATA), rsi_http_post_response_handler);

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
  rsi_task_create(rsi_http_client_app, "wlan_task", RSI_WLAN_TASK_STACK_SIZE, NULL, RSI_WLAN_TASK_PRIORITY, &wlan_task_handle);

  //! Task created for Driver task
  rsi_task_create(rsi_wireless_driver_task, "driver_task",RSI_DRIVER_TASK_STACK_SIZE, NULL, RSI_DRIVER_TASK_PRIORITY, &driver_task_handle);

  //! OS TAsk Start the scheduler
  rsi_start_os_scheduler();

#else
  //! NON - OS case
  //! Call HTTP client application
  status = rsi_http_client_app();

  //! Application main loop
  main_loop();
#endif
  return status;

}

//! http put response notify call back handler 
void rsi_http_client_put_response_handler(uint16_t status, const uint8_t type, const uint8_t end_of_put_pkt)
{
  //! Check for SUCCESS
  if(status == RSI_SUCCESS)
  {
    //! Set success response 
    rsp_received = 1;

    if(type == HTTP_CLIENT_PUT_PKT) 
    {
      //! Get end of file/resource 
      end_of_file = end_of_put_pkt;
    }
  }
  else
  {
    //! Set failure response 
    rsp_received = 2;
    end_of_file  = 1;
  }
}


//! http get response notify call back handler 
void rsi_http_get_response_handler(uint16_t status, const uint8_t *buffer, const uint16_t length, const uint32_t moredata)
{
  if(buf_len + length > APP_BUFF_LEN)
  {
    //! Application buffer over flowed
    return ;
  }
  if(!moredata)
  {
    //! copy received data to application buffer
    memcpy(app_buf + buf_len, buffer, length);

    //! update buffer length
    buf_len += length;
  }
  else
  {
    if(length)
    {
      //! copy received data to application buffer
      memcpy(app_buf + buf_len, buffer, length);

      //! update buffer length
      buf_len += length;
    }
    //! set to exit from driver task
    http_recv_done = 1;
  }
}

//! http post response notify call back handler 
void rsi_http_post_response_handler(uint16_t status, const uint8_t *buffer, const uint16_t length, const uint32_t moredata)
{
  if(buf_len + length > APP_BUFF_LEN)
  {
    //! Application buffer over flowed
    return ;
  }
  if(!moredata)
  {
    //! copy received data to application buffer
    memcpy(app_buf + buf_len, buffer, length);

    //! update buffer length
    buf_len += length;
  }
  else
  {
    if(length)
    {
      //! copy received data to application buffer
      memcpy(app_buf + buf_len, buffer, length);

      //! update buffer length
      buf_len += length;
    }
    //! set to exit from driver task
    http_recv_done = 1;
  }

}
