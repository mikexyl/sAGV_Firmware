/*
 * @file    rsi_ota_firmware_upgradation_app.c
 * @version 0.2
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
 *  @brief : This file contains example application for firmware upgradation
 *
 *  @section Description  This file contains example application for firmwate upgradation
 *  This application uses TCP client to get the firmware file from remote server and uses 
 *  firmware upgradation APIs to upgrade
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


//! socket include file to firmware upgrade APIs
#include "rsi_firmware_upgradation.h"

//! Access point SSID to connect
#define SSID              "REDPINE_AP"

//! Security type
#define SECURITY_TYPE     RSI_OPEN

//! Password
#define PSK               ""


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

//! Device port number
#define DEVICE_PORT       5001

//! Server port number
#define SERVER_PORT       5001

//! Server IP address. Should be in reverse long format
//! E.g: 0x640AA8C0 == 192.168.0.101
#define SERVER_IP_ADDRESS 0x6500A8C0

//! Receive data length
#define RECV_BUFFER_SIZE   1027

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

//! OTAF Server port number
#define OTAF_SERVER_PORT                  5001

//! OTAF TCP receive timeout
#define OTAF_RX_TIMEOUT                   200

//! OTAF TCP retry count
#define OTAF_TCP_RETRY_COUNT              20

//! OTA Firmware upgradation retry count
#define OTAF_RETRY_COUNT                  7000

//! OTAF response handler
void rsi_ota_fw_up_response_handler(uint16_t status, uint16_t chunk_number);

//! Memory to initialize driver
uint8_t global_buf[GLOBAL_BUFF_LEN];

//! standard defines
volatile uint8_t  rsp_received;

//! OTAF retry count
volatile uint16_t otaf_retry_cnt;

static uint16_t    chunk_number = 1;

int32_t rsi_firmware_upgradation_app()
{
  int32_t     client_socket;
  struct      sockaddr_in server_addr, client_addr;
  int32_t     status       = RSI_SUCCESS;
  int32_t     recv_size = 0;
#if !(DHCP_MODE)
  uint32_t    ip_addr      = DEVICE_IP;
  uint32_t    network_mask = NETMASK;
  uint32_t    gateway      = GATEWAY;
#endif
  uint8_t     send_buffer[3];
  int8_t      recv_buffer[RECV_BUFFER_SIZE];
  uint16_t    fwup_chunk_length, recv_offset = 0, fwup_chunk_type;
  uint32_t    otaf_server_addr  = SERVER_IP_ADDRESS;

  //! Set OTAF retry count
  otaf_retry_cnt = OTAF_RETRY_COUNT;

  //! WC initialization
  status = rsi_wireless_init(0, 0);
  if(status != RSI_SUCCESS)
  {
    return status;
  }

  //! Scan for Access points
  status = rsi_wlan_scan((int8_t *)SSID, 0, NULL, 0);
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
  status = rsi_config_ipaddress(RSI_IP_VERSION_4, RSI_DHCP, 0, 0, 0, NULL, 0, 0);
#else
  status = rsi_config_ipaddress(RSI_IP_VERSION_4, RSI_STATIC, (uint8_t *)&ip_addr, (uint8_t *)&network_mask, (uint8_t *)&gateway, NULL, 0, 0);
#endif
  if(status != RSI_SUCCESS)
  {
    return status;
  }
 


  do
  {
    status = rsi_ota_firmware_upgradation(RSI_IP_VERSION_4, (uint8_t *)&otaf_server_addr, (uint32_t *)OTAF_SERVER_PORT, 
        (uint16_t *)chunk_number, (uint16_t *)OTAF_RX_TIMEOUT, (uint16_t *)OTAF_TCP_RETRY_COUNT, rsi_ota_fw_up_response_handler);

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

    //! Check for FWUP success
    if(rsp_received == 1)
    {
      break;
    }

    //! Check for FWUP failure 
    if((rsp_received == 2) && (otaf_retry_cnt))
    {
      otaf_retry_cnt--;
    }
    
    //! Check for FWUP failure  with retry count
    if((rsp_received== 2) && (otaf_retry_cnt == 0))
    {
      return  rsi_wlan_get_status();
    }

    //! reset rsp received
    rsp_received = 0;

  } while(otaf_retry_cnt);

    
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
  rsi_task_create( rsi_firmware_upgradation_app, "wlan_task", RSI_WLAN_TASK_STACK_SIZE, NULL, RSI_WLAN_TASK_PRIORITY, &wlan_task_handle);

  //! Task created for Driver task
  rsi_task_create(rsi_wireless_driver_task, "driver_task",RSI_DRIVER_TASK_STACK_SIZE, NULL, RSI_DRIVER_TASK_PRIORITY, &driver_task_handle);

  //! OS TAsk Start the scheduler
  rsi_start_os_scheduler();

#else
  //! NON - OS case
  //! Call firware upgradation application
  status = rsi_firmware_upgradation_app();

  //! Application main loop
  main_loop();
#endif
  return status;

}


//! OTAF respone handler
void rsi_ota_fw_up_response_handler(uint16_t status, uint16_t chunk_no)
{

  if(status == RSI_SUCCESS)
  {
    rsp_received = 1;
  }
  else
  { 
    rsp_received = 2;
    chunk_number = chunk_no;
  } 

  return;
}

