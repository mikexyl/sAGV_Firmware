/**
 * @file    main.c
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
 *  @brief : This file contains example application for TCP server socket
 *
 *  @section Description  This file contains example application for TCP server socket 
 *
 *
 */

#include "rsi_driver.h"

//! Memory length for driver
#define GLOBAL_BUFF_LEN    10000

//! Wlan client mode 
#define RSI_WLAN_CLIENT_MODE    0

//! ZIGBEE Coex mode 
#define RSI_ZIGBEE_MODE           3

//! Parameter to run forever loop
#define RSI_FOREVER             1 

//! Memory to initialize driver
uint8_t global_buf[GLOBAL_BUFF_LEN];

//! Function prototypes
extern void rsi_wlan_app_task(void);
extern int rsi_zigbee_app_init (void);
extern void rsi_zigbee_app_task (void);

int32_t rsi_wlan_zigbee_app(void)
{
  int32_t     status = RSI_SUCCESS;
  //uint8_t     data[0]= {2};
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

  //! WiSeConnect initialization
  status = rsi_wireless_init(RSI_WLAN_CLIENT_MODE, RSI_ZIGBEE_MODE);
  if(status != RSI_SUCCESS)
  {
    return status;
  }

  //! zigbee initialization
  if(rsi_zigbee_app_init() == -1)
  {
    printf("\n### ZigBee not ready"); 
  }
  else
  {
    printf("\n### ZigBee Ready"); 
  }
  
  while(RSI_FOREVER)
  {
    //! WLAN application tasks
    rsi_wlan_app_task();

    //! zigbee application tasks
    rsi_zigbee_app_task();

    //! wireless driver tasks
    rsi_wireless_driver_task();

  }
  return 0;
}

//! Forever in wireless driver task 
void main_loop()
{
  while (RSI_FOREVER)
  {
    rsi_wireless_driver_task ();
  }
}

//! main funtion definition
int main(void)
{
  int32_t status;

#ifndef RSI_SAMPLE_HAL
  //! Board Initialization
  Board_init();
#endif

  //! Call WLAN zigbee application
  status = rsi_wlan_zigbee_app();

  //! Application main loop
  main_loop();

  return status;

}
