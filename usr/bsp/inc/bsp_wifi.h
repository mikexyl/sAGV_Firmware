#ifndef _BSP_WIFI_H_
#define	_BSP_WIFI_H_

#include "stdarg.h"

#include <string.h>
#include <stdint.h>

#include <ucos_ii.h>
#include "bsp_uart.h"


/*Only support 1 connection at the present stage*/
typedef enum
{
    TCP_SERVER,
    TCP_CLIENT,
    UDP_BROADCAST,
    UDP_UNICAST,
}mico_protocol_t;

typedef enum
{
    WIFI_MODE_AP,
    WIFI_MODE_STATOIN,
    WIFI_MODE_AP_STATOIN,
}mico_wifi_mode_t;


#define IPCONF_DHCP 0
#define MICO_MAX_DEVICE_NUM     1
#define	MICO_MAX_SOCKET_NUM		1

int mico_create_device(USART_TypeDef * _Uartx, uint32_t wifi_mode);
int mico_init_driver(void);

void USART_OUT(USART_TypeDef* USARTx, uint8_t *Data,...);

int mico_init_station(int dev, char *ap_name, char*password, uint32_t ip, uint32_t mask, uint32_t gateway);
//Only support TCP_SERVER, UDP_UNICAST
//ip: for TCP_SERVER, it,s remote addr, for UDP_UNICAST it has no mean
//port: for TCP_SERVER, it,s remote port, for UDP_UNICAST it,s the local port

int mico_create_connection(int device, mico_protocol_t proto, uint32_t ip, uint16_t port_remote, u16 port_local);
int mico_read(int sock, uint8_t *buf, uint32_t size);

int mico_writeto(int sock, const uint8_t *buf, uint16_t len, uint32_t remote_ip, uint32_t remote_port);
int mico_write(int sock, const uint8_t *buf, uint16_t len);

void mico_SendCmdFun(u8 * _pcmd, u8 * _pre);


#endif

