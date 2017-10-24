/**
 * @file    rsi_store_config_profile_app.c
 * @version 0.1
 * @date    6 Feb 2016
 *
 *  Copyright(C) Redpine Signals 2015
 *  All rights reserved by Redpine Signals.
 *
 *  @section License
 *  This program should be used on your own responsibility.
 *  Redpine Signals assumes no responsibility for any losses
 *  incurred by customers or third parties arising from the use of this file.
 *
 *  @brief : This file contains example application for store config profile functionality 
 *
 *  @section Description  This file contains example application for store config profile functionality 
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

#include "rsi_wlan_config.h"

//! socket include file to refer socket APIs
#include "rsi_socket.h"

//! Error include files
#include "rsi_error.h"

//! OS include file to refer OS specific functionality
#include "rsi_os.h"

//! configurattion Parameters

//! Set profile type
#define PROFILE_TYPE      RSI_WLAN_PROFILE_AP 

//! Enable config 2- Enable 0- Disable
#define AUTO_CONFIG_ENABLE            2

//! Auto configuration failed
#define RSI_AUTO_CONFIG_FAILED            BIT(1)

//! Auto configuration going on 
#define RSI_AUTO_CONFIG_GOING_ON          BIT(2)

//! Auto configuration done
#define RSI_AUTO_CONFIG_DONE              BIT(3)

#define RSI_DRIVER_TASK_STACK_SIZE  500

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

//! Memory for wlan profile
uint8_t wlan_profile[500];

//! Memory for network profile
uint8_t network_profile[100];

//! Config profile global response object
uint8_t config_profile_rsp[1000];

//! Function prototypes
void main_loop(void);

//! Pointer to wlan profile 
uint8_t     *wlan_profile_ptr = NULL;

//! Auto configuration handler
void rsi_auto_config_response_handler(uint16_t status, uint8_t state);

//! store config profile Application 
int32_t rsi_store_config_profile_app()
{
  int32_t     status            = RSI_SUCCESS;
  uint8_t     *nwk_profile_ptr  = NULL;
  uint8_t     state = 0;
   
  //! Register auto config handler
  rsi_register_auto_config_rsp_handler(rsi_auto_config_response_handler);

  //! WC initialization
  status = rsi_wireless_init(0, 0);
  if(status != RSI_SUCCESS)
  {
    return status;
  }

  //! Fill wlan config profile
  wlan_profile_ptr =  rsi_fill_config_profile((uint32_t) PROFILE_TYPE, (uint8_t *)wlan_profile);

  if(wlan_profile_ptr == NULL)
  {
    //! Invalid profile type
    return -1;
  }

  //! Add config profile
  status = rsi_wlan_add_profile((uint32_t) PROFILE_TYPE, wlan_profile_ptr);

  //! Check for success status
  if(status != RSI_SUCCESS)
  {
    return status;
  }

  //! Get config profile
  status = rsi_wlan_get_profile((uint32_t) PROFILE_TYPE, &config_profile_rsp, sizeof(config_profile_rsp));
  
  if(status != RSI_SUCCESS)
  {
    return status;
  }

  //! Enable/Disable auto configuration
  status = rsi_wlan_enable_auto_config((uint8_t) AUTO_CONFIG_ENABLE, (uint32_t) PROFILE_TYPE);

  if(status != RSI_SUCCESS)
  {
    return status;
  }

  /////////////////////////////////////////////
  /////      !Enable below code to        /////
  /////      Delete config profile        ///// 
  /////////////////////////////////////////////

#if 0
  //! Delete config profile
  status = rsi_wlan_delete_profile((uint32_t) PROFILE_TYPE);

  if(status != RSI_SUCCESS)
  {
    return status;
  }
#endif

  //! WC Deinitialization
  status = rsi_wireless_deinit();
  
  if(status != RSI_SUCCESS)
  {
    return status;
  }

  //! WC initialization
  status = rsi_wireless_init(0, 0);
  
  if(status != RSI_SUCCESS)
  {
    return status;
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
  rsi_task_create(rsi_store_config_profile_app, "wlan_task", RSI_WLAN_TASK_STACK_SIZE, NULL, RSI_WLAN_TASK_PRIORITY, &wlan_task_handle);

  //! Task created for Driver task
  rsi_task_create(rsi_wireless_driver_task, "driver_task",RSI_DRIVER_TASK_STACK_SIZE, NULL, RSI_DRIVER_TASK_PRIORITY, &driver_task_handle);

  //! OS TAsk Start the scheduler
  rsi_start_os_scheduler();

#else
  //! NON - OS case
  //! Call store config profile application
  status = rsi_store_config_profile_app();

  //! Application main loop
  main_loop();
#endif
  return status;

}

void rsi_auto_config_response_handler(uint16_t status, uint8_t state)
{
  if(state == RSI_AUTO_CONFIG_FAILED)
  {
#ifdef  RSI_ENABLE_DEBUG_PRINT
    printf("\n ____ AUTO CONFIG FAILED ____ \n");
#endif

  }
  if(state == RSI_AUTO_CONFIG_GOING_ON)
  {
#ifdef  RSI_ENABLE_DEBUG_PRINT
    printf("\n ____ AUTO CONFIG GOING ON ____ \n");
#endif

  }

  if(state == RSI_AUTO_CONFIG_DONE)
  {

#ifdef  RSI_ENABLE_DEBUG_PRINT
    printf("\n ____ AUTO CONFIG DONE ____  \n",state); 
#endif
  }

  return;
}

