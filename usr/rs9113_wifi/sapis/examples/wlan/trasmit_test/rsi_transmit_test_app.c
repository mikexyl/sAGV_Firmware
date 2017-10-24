/**
 * @file    rsi_transmit_test_app.c
 * @version 0.1
 * @date    7 Oct 2015
 *
 *  Copyright(C) Redpine Signals 2015
 *  All rights reserved by Redpine Signals.
 *
 *  @section License
 *  This program should be used on your own responsibility.
 *  Redpine Signals assumes no responsibility for any losses
 *  incurred by customers or third parties arising from the use of this file.
 *
 *  @brief : This file contains example application for transmit test mode.
 *  @section Description  This file contains example application for transmit test.
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

//! Error include files
#include "rsi_error.h"

//! OS include file to refer OS specific functionality
#include "rsi_os.h"

//! Transmit test power 
#define RSI_TX_TEST_POWER             4                      

//! Transmit test rate
#define RSI_TX_TEST_RATE              RSI_RATE_1              

//! Transmit test length
#define RSI_TX_TEST_LENGTH            30                      

//! Transmit test mode
#define RSI_TX_TEST_MODE              RSI_BURST_MODE      

//! Transmit test channel
#define RSI_TX_TEST_CHANNEL           1                       

//! Select Intenal antenna or uFL connector
#define RSI_ANTENNA           1

//! Antenna gain in 2.4GHz band
#define RSI_ANTENNA_GAIN_2G           0
                       
//! Antenna gain in 5GHz band
#define RSI_ANTENNA_GAIN_5G           0

//! Memory length for driver
#define GLOBAL_BUFF_LEN 8000

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

int32_t rsi_transmit_test_app()
{
  int32_t     status       = RSI_SUCCESS;


  //! WC initialization
  status = rsi_wireless_init(8, 0);
  if(status != RSI_SUCCESS)
  {
    return status;
  }
 
  //! To selct Internal antenna or uFL connector 
  status = rsi_wireless_antenna(RSI_ANTENNA, RSI_ANTENNA_GAIN_2G , RSI_ANTENNA_GAIN_5G);
  if(status != RSI_SUCCESS)
  {
    return status;
  }

  //! transmit test start
  status = rsi_transmit_test_start(RSI_TX_TEST_POWER, RSI_TX_TEST_RATE, RSI_TX_TEST_LENGTH,
                                    RSI_TX_TEST_MODE, RSI_TX_TEST_CHANNEL);
  if(status != RSI_SUCCESS)
  {
    return status;
  }

  return status;
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
  rsi_task_create(rsi_transmit_test_app, "wlan_task", RSI_WLAN_TASK_STACK_SIZE, NULL, RSI_WLAN_TASK_PRIORITY, &wlan_task_handle);

  //! Task created for Driver task
  rsi_task_create(rsi_wireless_driver_task, "driver_task",RSI_DRIVER_TASK_STACK_SIZE, NULL, RSI_DRIVER_TASK_PRIORITY, &driver_task_handle);

  //! OS TAsk Start the scheduler
  rsi_start_os_scheduler();

#else
  //! NON - OS case
  //! Call Transmit test application
  status = rsi_transmit_test_app();

  //! Application main loop
  main_loop();
#endif
  return status;

}
