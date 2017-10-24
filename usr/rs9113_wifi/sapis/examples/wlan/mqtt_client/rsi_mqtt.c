/**
 * @file    rsi_mqtt.c
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
 *  @brief : This file contains example application for MQTT client demo
 *
 *  @section Description  This file contains example application for MQTT client demo 
 *  which connects to the MQTT broker ,subscribes to the topic and publishes data on the given topic
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

//! MQTT wrappers to include
#include "rsi_mqtt_client.h"

#include "rsi_wlan.h"


//! Access point SSID to connect
#define SSID              "REDPINE_AP"
//! Security type
#define SECURITY_TYPE     RSI_OPEN

//! Password
#define PSK               ""

//! DHCP mode 1- Enable 0- Disable
#define DHCP_MODE         1

//! If DHCP mode is disabled give IP statically
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

//! Server IP address. Should be in reverse long format
//! E.g: 0x640AA8C0 == 192.168.10.100
#define SERVER_IP_ADDRESS 0x640AA8C0


//! Server port number
#define SERVER_PORT       1883


//! Client port number
#define CLIENT_PORT       5001

//! Memory length for driver
#define GLOBAL_BUFF_LEN   8000

//! Wlan task priority
#define RSI_WLAN_TASK_PRIORITY   1

//! Wireless driver task priority
#define RSI_DRIVER_TASK_PRIORITY   1

//! Wlan task stack size
#define RSI_WLAN_TASK_STACK_SIZE  500

//! Wireless driver task stack size
#define RSI_DRIVER_TASK_STACK_SIZE  500

//! Keep alive period
#define RSI_KEEP_ALIVE_PERIOD     100

//!Memory to initialize MQTT client Info structure
#define MQTT_CLIENT_INIT_BUFF_LEN  3500

//! QOS of the message
#define QOS                        0  

//! Memory to initialize driver
uint8_t global_buf[GLOBAL_BUFF_LEN];


//! MQTT Related Macros and declarations
#define RSI_MQTT_TOPIC       "REDPINE"

//! Message to publish
uint8_t publish_message[] ="THIS IS MQTT CLIENT DEMO FROM REDPINE";

//! MQTT CLient ID
int8_t clientID[] = "MQTTCLIENT";

//! user name for login credentials
int8_t  username[] = "username";

//! password for login credentials
int8_t  password[] = "password";

volatile int halt = 0;


int8_t mqqt_client_buffer[MQTT_CLIENT_INIT_BUFF_LEN];

//! Call back when a message is received on the published topic
void rsi_message_received(MessageData* md)
{

  MQTTMessage* message = md->message;

  MQTTString* topic = md->topicName;
  //! process the received data
  halt = 1;
  return;
}


//! Remote socket terminate call back handler
void rsi_remote_socket_terminate_handler(uint16_t status, const uint8_t *buffer, const uint16_t length)
{
  rsi_rsp_socket_close_t *close = (rsi_rsp_socket_close_t *)buffer;
  //! Remote socket has been terminated
}


int32_t rsi_mqtt_client()
{

  struct      sockaddr_in server_addr;
  int32_t     status       = RSI_SUCCESS;
#if !(DHCP_MODE)
  uint32_t    ip_addr      = DEVICE_IP;
  uint32_t    network_mask = NETMASK;
  uint32_t    gateway      = GATEWAY;
#endif
  uint32_t    server_address =  SERVER_IP_ADDRESS;

  rsi_mqtt_client_info_t *rsi_mqtt_client = NULL;

  MQTTMessage publish_msg;

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
  
  //! Initialze remote terminate call back
  rsi_wlan_register_callbacks(RSI_REMOTE_SOCKET_TERMINATE_CB, rsi_remote_socket_terminate_handler);

  //! MQTT client initialisation
  rsi_mqtt_client = rsi_mqtt_client_init(mqqt_client_buffer, MQTT_CLIENT_INIT_BUFF_LEN,(int8_t *)&server_address,SERVER_PORT,CLIENT_PORT,0,RSI_KEEP_ALIVE_PERIOD);
  if(mqqt_client_buffer == NULL)
  {
    return status;
  }
  //! Connect to the MQTT brohker/server
  status = rsi_mqtt_connect(rsi_mqtt_client,0,clientID,NULL,NULL);
  if(status != RSI_SUCCESS)
  {
    return status;
  }

  //! Subscribe to the topic given
  rsi_mqtt_subscribe(rsi_mqtt_client,QOS,RSI_MQTT_TOPIC,rsi_message_received);
  if(status != RSI_SUCCESS)
  {
    return status;
  }

  //!The DUP flag MUST be set to 1 by the Client or Server when it attempts to re-deliver a PUBLISH Packet
  //!The DUP flag MUST be set to 0 for all QoS 0 messages
  publish_msg.dup = 0;

  //! This field indicates the level of assurance for delivery of an Application Message. The QoS levels are
  //! 0  - At most once delivery
  //! 1  - At least once delivery
  //! 2  - Exactly once delivery
  publish_msg.qos = QOS;

  //! If the RETAIN flag is set to 1, in a PUBLISH Packet sent by a Client to a Server, the Server MUST store
  //! the Application Message and its QoS, so that it can be delivered to future subscribers whose
  //! subscriptions match its topic name
  publish_msg.retained = 0;

  //! Attach paylaod 
  publish_msg.payload = publish_message;

  //! Fill paylaod length
  publish_msg.payloadlen = sizeof(publish_message);


  //! Publish message on the topic
  rsi_mqtt_publish(rsi_mqtt_client,RSI_MQTT_TOPIC,&publish_msg);

  while (!halt)
  {
    //! Recv data published on the subscribed topic 
    status = rsi_mqtt_poll_for_recv_data(rsi_mqtt_client,10);
    if(status != RSI_SUCCESS)
    {
      //! Error in receiving
      return status;
    }
  }

  //! UnSubscribe to the topic given
  rsi_mqtt_unsubscribe(rsi_mqtt_client,RSI_MQTT_TOPIC);
  if(status != RSI_SUCCESS)
  {
    //! Error in receiving
    return status;
  }
  //! Disconnect to the MQTT broker
  rsi_mqtt_disconnect(rsi_mqtt_client);
  if(status != RSI_SUCCESS)
  {
    //! Error in receiving
    return status;
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

  //! Unmask interrupts


#ifdef RSI_WITH_OS	
  //! OS case
  //! Task created for WLAN task
  rsi_task_create(rsi_mqtt_client, "wlan_task", RSI_WLAN_TASK_STACK_SIZE, NULL, RSI_WLAN_TASK_PRIORITY, &wlan_task_handle);

  //! Task created for Driver task
  rsi_task_create(rsi_wireless_driver_task, "driver_task",RSI_DRIVER_TASK_STACK_SIZE, NULL, RSI_DRIVER_TASK_PRIORITY, &driver_task_handle);

  //! OS TAsk Start the scheduler
  rsi_start_os_scheduler();

#else
  //! NON - OS case
  //! Call MQTT client application
  status = rsi_mqtt_client();

  //! Application main loop
  main_loop();
#endif
  return status;

}

