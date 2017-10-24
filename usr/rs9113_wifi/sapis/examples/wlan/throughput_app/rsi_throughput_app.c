/**
 * @file    rsi_throughput_app.c
 * @version 0.1
 * @date    4 Dec 2015
 *
 *  Copyright(C) Redpine Signals 2015
 *  All rights reserved by Redpine Signals.
 *
 *  @section License
 *  This program should be used on your own responsibility.
 *  Redpine Signals assumes no responsibility for any losses
 *  incurred by customers or third parties arising from the use of this file.
 *
 *  @brief : This file contains example application for throughput measurment
 *
 *  @section Description  This file contains example application for throughput measurment 
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



//! Access point SSID to connect
#define SSID              "REDPINE_AP"

//!Scan Channel number , 0 - to scan all channels
#define CHANNEL_NO              0

//! Security type
#define SECURITY_TYPE     RSI_OPEN

//! Password
#define PSK               NULL

//! DHCP mode 1- Enable 0- Disable
#define DHCP_MODE         1

//! If DHCP mode is disabled give IP statically
#if !(DHCP_MODE)

//! IP address of the module 
//! E.g: 0x650AA8C0 == 192.168.10.101
#define DEVICE_IP         0x6500A8C0

//! IP address of Gateway
//! E.g: 0x010AA8C0 == 192.168.10.1
#define GATEWAY           0x010AA8C0

//! IP address of netmask
//! E.g: 0x00FFFFFF == 255.255.255.0
#define NETMASK           0x00FFFFFF 

#endif

//! port number
#define PORT_NUM       5001

//! Server IP address. Should be in reverse long format
//! E.g: 0x640AA8C0 == 192.168.10.100
#define SERVER_IP_ADDRESS 0x640AA8C0

//! Memory length for driver
#define GLOBAL_BUFF_LEN   8000

//! Memory length for send buffer
#define BUF_SIZE   1400

//! Wlan task priority
#define RSI_WLAN_TASK_PRIORITY   1

//! Wireless driver task priority
#define RSI_DRIVER_TASK_PRIORITY   1

//! Wlan task stack size
#define RSI_WLAN_TASK_STACK_SIZE  500

//! Wireless driver task stack size
#define RSI_DRIVER_TASK_STACK_SIZE  500


//! Type of throughput
#define  UDP_TX                     0
#define  UDP_RX                     1
#define  TCP_TX                     2
#define  TCP_RX                     3

//! Power measurement type
#define THROUGHPUT_TYPE  UDP_TX

//! Memory to initialize driver
uint8_t global_buf[GLOBAL_BUFF_LEN];
uint32_t total_tx_bytes = 0;
uint32_t total_rx_bytes;
uint32_t secs;
static uint32_t last_tx_print_time = 0;
static uint32_t last_rx_print_time;

extern volatile uint32_t _dwTickCount;

void measure_throughput(uint32_t  pkt_length, uint32_t tx_rx)
{
	static uint32_t current_time;
	static uint32_t last_print_time;
	uint32_t total_bytes;
	struct timeval tv1;
	float through_put;
#ifdef LINUX_PLATFORM
	gettimeofday(&tv1, NULL);
	current_time = tv1.tv_sec * 1000000 + tv1.tv_usec;
#else
	current_time = _dwTickCount ;
#endif
	if(tx_rx == 0)
	{
		total_tx_bytes += pkt_length;
		total_bytes = total_tx_bytes;
		last_print_time = last_tx_print_time;
	}
	else
	{
		total_rx_bytes += pkt_length;
		total_bytes = total_rx_bytes;
		last_print_time = last_rx_print_time;
	}
#ifdef LINUX_PLATFORM
if((current_time-last_print_time)>=1000000)    //!for 1 sec
#else
	if((current_time-last_print_time)>=30000)    //!for 30 sec
#endif
	{
#ifdef LINUX_PLATFORM
		through_put = ((float)(total_bytes)/((current_time-last_print_time))) * 8;
#else
		through_put = ((float)(total_bytes)/((current_time-last_print_time))) * 8;
		through_put /= 1000;
#endif
		if(tx_rx == 0)
		{
#ifdef LINUX_PLATFORM
			printf("\nSecs: %d     Bytes Transmitted %d,Throughput for last %d seconds is = %3.1f Mbps\n",secs++,total_bytes,(current_time-last_print_time)/1000000, through_put);
#endif
			last_tx_print_time = current_time;
			total_tx_bytes = 0;
		}
		else
		{
#ifdef LINUX_PLATFORM
			printf("\nSecs: %d     Bytes Received %d,Throughput for last %d seconds is = %3.1f Mbps\n",secs++,total_bytes,(current_time-last_print_time)/1000000, through_put);
#endif
			last_rx_print_time = current_time;
			total_rx_bytes = 0;
		}
	}
return;
}

int32_t rsi_throughput_app()
{
	int32_t     client_socket;
	int32_t     server_socket, new_socket;
	struct      sockaddr_in server_addr, client_addr;
	int32_t     status       = RSI_SUCCESS;
	uint32_t    recv_size = 0;
	int32_t     addr_size;
#if !(DHCP_MODE)
	uint32_t    ip_addr      = DEVICE_IP;
	uint32_t    network_mask = NETMASK;
	uint32_t    gateway      = GATEWAY;
#endif
	int8_t     send_buf[BUF_SIZE];
	uint16_t    i = 0;

	//! buffer to receive data over TCP/UDP client socket
	int8_t recv_buffer[BUF_SIZE];


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
		printf("Error \n");
		return status;
	}

	for (i = 0 ; i < BUF_SIZE; i++)
	{
		send_buf[i] = i;
	}

	switch(THROUGHPUT_TYPE)
	{
	case UDP_TX:
	{
		//! Create socket
		client_socket = socket(AF_INET, SOCK_DGRAM, 0);
		if(client_socket < 0)
		{
			status = rsi_wlan_get_status();
			return status;
		}

		//! Set server structure
		memset(&server_addr, 0, sizeof(server_addr));

		//! Set server address family
		server_addr.sin_family = AF_INET;

		//! Set server port number, using htons function to use proper byte order
		server_addr.sin_port = htons(PORT_NUM);

		//! Set IP address to localhost
		server_addr.sin_addr.s_addr = SERVER_IP_ADDRESS;

		while(1)
		{
			//! Send data on socket
			status = sendto(client_socket, send_buf, BUF_SIZE, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
			if(status < 0)
			{
				status = rsi_wlan_get_status();
				shutdown(client_socket, 0);
				return status;
			}

			//! Measure throughput per second, 0 - Tx and 1 - Rx
			measure_throughput(BUF_SIZE, 0);

		}
	}
	break;
	case UDP_RX:
	{
		//! Create socket
		server_socket = socket(AF_INET, SOCK_DGRAM, 0);
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
		server_addr.sin_port = htons(PORT_NUM);

		//! Bind socket
		status = bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr));
		if(status != RSI_SUCCESS)
		{
			status = rsi_wlan_get_status();
			shutdown(server_socket, 0);
			return status;
		}

		addr_size = sizeof(server_addr);

		while(1)
		{
			recv_size = BUF_SIZE;

			do
			{
				//! Receive data on socket
				status = recvfrom(server_socket, recv_buffer, recv_size, 0, (struct sockaddr *)&client_addr, &addr_size);
				if(status < 0)
				{
					status = rsi_wlan_get_status();
					shutdown(server_socket, 0);
					return status;
				}

				//! subtract received bytes
				recv_size -= status;

				//! Measure throughput per second, 0 - Tx and 1 - Rx
				measure_throughput(status, 1);

			} while(recv_size > 0);

		}
	}
	break;
	case TCP_TX:
	{
		//! Create socket
		client_socket = socket(AF_INET, SOCK_STREAM, 0);
		if(client_socket < 0)
		{
			status = rsi_wlan_get_status();
			return status;
		}

		//! Memset client structrue
		memset(&client_addr, 0, sizeof(client_addr));

		//! Set family type
		client_addr.sin_family= AF_INET;

		//! Set local port number
		client_addr.sin_port = htons(PORT_NUM);

		//! Bind socket
		status = bind(client_socket, (struct sockaddr *) &client_addr, sizeof(client_addr));
		if(status != RSI_SUCCESS)
		{
			status = rsi_wlan_get_status();
			shutdown(client_socket, 0);
			return status;
		}

		//! Set server structure
		memset(&server_addr, 0, sizeof(server_addr));

		//! Set server address family
		server_addr.sin_family = AF_INET;

		//! Set server port number, using htons function to use proper byte order
		server_addr.sin_port = htons(PORT_NUM);

		//! Set IP address to localhost
		server_addr.sin_addr.s_addr = SERVER_IP_ADDRESS;

		//! Connect to server socket
		status = connect(client_socket, (struct sockaddr *) &server_addr, sizeof(server_addr));
		if(status != RSI_SUCCESS)
		{
			status = rsi_wlan_get_status();
			shutdown(client_socket, 0);
			return status;
		}

		while(1)
		{
			//! Send data on socket
			status = send(client_socket,send_buf, BUF_SIZE, 0);
			if(status < 0)
			{
				status = rsi_wlan_get_status();
				shutdown(client_socket, 0);
				return status;
			}

			//! Measure throughput per second, 0 - Tx and 1 - Rx
			measure_throughput(BUF_SIZE, 0);

		}
	}
	break;
	case TCP_RX:
	{
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
		server_addr.sin_port = htons(PORT_NUM);



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

		while(1)
		{
			recv_size = BUF_SIZE;

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

				//! Measure throughput per second, 0 - Tx and 1 - Rx
				measure_throughput(status, 1);

			} while(recv_size > 0);

		}
	}
	break;
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
	rsi_task_create(rsi_throughput_app, "wlan_task", RSI_WLAN_TASK_STACK_SIZE, NULL, RSI_WLAN_TASK_PRIORITY, &wlan_task_handle);

	//! Task created for Driver task
	rsi_task_create(rsi_wireless_driver_task, "driver_task",RSI_DRIVER_TASK_STACK_SIZE, NULL, RSI_DRIVER_TASK_PRIORITY, &driver_task_handle);

	//! OS TAsk Start the scheduler
	rsi_start_os_scheduler();

#else
	//! NON - OS case
	//! Call UDP client application
	status = rsi_throughput_app();

	//! Application main loop
	main_loop();
#endif
	return status;

}

