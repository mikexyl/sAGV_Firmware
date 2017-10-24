#include <ucos_ii.h>
#include "bsp.h"
#include "bsp_timer.h"
#include "app_cfg.h"
//! include file to refer data types
#include "rsi_data_types.h"
//! COMMON include file to refer wlan APIs
#include "rsi_common_apis.h"

#include "rsi_bootup_config.h"

//! WLAN include file to refer wlan APIs
#include "rsi_wlan_apis.h"

//! socket include file to refer socket APIs
#include "rsi_socket.h"

//! Error include files
#include "rsi_error.h"

#if 0
int32_t RSI_BAND = RSI_BAND_5GHZ;
int32_t RSI_CONFIG_CLIENT_BAND = RSI_BAND_5GHZ;
#else
int32_t RSI_BAND = RSI_BAND_2P4GHZ;
int32_t RSI_CONFIG_CLIENT_BAND = RSI_BAND_2P4GHZ;
#endif

int32_t RSI_CONFIG_CLIENT_SCAN_CHAN_BITMAP_2_4_GHZ = 0;
int32_t RSI_CONFIG_CLIENT_SCAN_CHAN_BITMAP_5_0_GHZ = 0;

int32_t RSI_SCAN_CHANNEL_BIT_MAP_2_4 = 0;
int32_t RSI_SCAN_CHANNEL_BIT_MAP_5 = 0;

#define WIFI_SPI_PORT               SPI2
#define WIFI_SPI_PORT_RCC           RCC_APB1Periph_SPI2
#define WIFI_SPI_PIN_AF             GPIO_AF_SPI2

#define WIFI_SPI_CS_GPIO_PORT       GPIOD
#define WIFI_SPI_CS_GPIO_PIN        GPIO_Pin_10

#define WIFI_SPI_SCK_GPIO_PORT      GPIOB
#define WIFI_SPI_SCK_GPIO_PIN       GPIO_Pin_10
#define WIFI_SPI_SCK_GPIO_PINSRC    GPIO_PinSource10

#define WIFI_SPI_MISO_GPIO_PORT     GPIOB
#define WIFI_SPI_MISO_GPIO_PIN      GPIO_Pin_14
#define WIFI_SPI_MISO_GPIO_PINSRC   GPIO_PinSource14

#define WIFI_SPI_MOSI_GPIO_PORT     GPIOB
#define WIFI_SPI_MOSI_GPIO_PIN      GPIO_Pin_15
#define WIFI_SPI_MOSI_GPIO_PINSRC   GPIO_PinSource15

#define WIFIDRIVER_TASK_STACKSIZE  512

static OS_STK driver_task_stack[WIFIDRIVER_TASK_STACKSIZE];

//the rsi buffer size need to be changed according to the rsi configuration
#define RSI_BUFFER_SIZE     10*1024
uint8_t wifi_driver_buffer[RSI_BUFFER_SIZE];

static void wifi_spi_gpio_init(void)
{
	  GPIO_InitTypeDef GPIO_InitStructure;

    //CS pin as gpio out
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_InitStructure.GPIO_Pin = WIFI_SPI_CS_GPIO_PIN;
	  GPIO_Init(WIFI_SPI_CS_GPIO_PORT, &GPIO_InitStructure);
    WIFI_SPI_CS_GPIO_PORT->BSRRL = WIFI_SPI_CS_GPIO_PIN;

    //sck, miso, mosi as spi function pin
		GPIO_PinAFConfig(WIFI_SPI_SCK_GPIO_PORT, WIFI_SPI_SCK_GPIO_PINSRC, WIFI_SPI_PIN_AF);
		GPIO_PinAFConfig(WIFI_SPI_MISO_GPIO_PORT, WIFI_SPI_MISO_GPIO_PINSRC, WIFI_SPI_PIN_AF);
		GPIO_PinAFConfig(WIFI_SPI_MOSI_GPIO_PORT, WIFI_SPI_MOSI_GPIO_PINSRC, WIFI_SPI_PIN_AF);

		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

		GPIO_InitStructure.GPIO_Pin = WIFI_SPI_SCK_GPIO_PIN;
		GPIO_Init(WIFI_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Pin = WIFI_SPI_MISO_GPIO_PIN;
		GPIO_Init(WIFI_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Pin = WIFI_SPI_MOSI_GPIO_PIN;
		GPIO_Init(WIFI_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);
}

int wifi_spi_init(void)
{
    //config spi pins
    wifi_spi_gpio_init();

    //enable spi clock
    RCC_APB1PeriphClockCmd(WIFI_SPI_PORT_RCC, ENABLE);

    SPI_InitTypeDef  SPI_InitStructure;

    /* 配置SPI硬件参数 */
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	/* 数据方向：2线全双工 */
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		/* STM32的SPI工作模式 ：主机模式 */
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;	/* 数据位长度 ： 8位 */
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;			/* 时钟下降沿采样数据 */
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;		/* 时钟的第1个边沿采样数据 */
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;			/* 片选控制方式：软件控制 */

    /* 设置波特率预分频系数 SPI_BaudRatePrescaler_8 ，实测SCK周期 96ns, 10.4MHz */
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;

    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	/* 数据位传输次序：高位先传 */
    SPI_InitStructure.SPI_CRCPolynomial = 7;			/* CRC多项式寄存器，复位后为7。本例程不用 */
    SPI_Init(WIFI_SPI_PORT, &SPI_InitStructure);

    SPI_Cmd(WIFI_SPI_PORT, ENABLE);				/* 使能SPI  */

    return 0;
}


static void driver_task(void *param)
{
    APP_TRACE("wifi driver task run.\r\n");
    rsi_wireless_driver_task();
}

int wifi_init(void)
{
    int32_t status;
    uint8_t err;

    //configure the spi gpio and spi port
    wifi_spi_init();

    //! Driver initialization
    status = rsi_driver_init(wifi_driver_buffer, sizeof(wifi_driver_buffer));
    if((status < 0) || (status > sizeof(wifi_driver_buffer)))
    {
      APP_TRACE("wifi initialize driver failed\r\n");
      return status;
    }

    //! RS9113 intialisation
    status = rsi_device_init(RSI_LOAD_IMAGE_I_FW);
    if(status != RSI_SUCCESS)
    {
      APP_TRACE("WIFI device initialization failed.\r\n");
      return status;
    }
    else
      APP_TRACE("WIFI device initialization ok.\r\n");

    err = OSTaskCreateExt(driver_task,	/* 启动任务函数指针 */
                    (void *)0,		/* 传递给任务的参数 */
                    (OS_STK *)&driver_task_stack[WIFIDRIVER_TASK_STACKSIZE - 1], /* 指向任务栈栈顶的指针 */
                    WIFIDRIVER_TASK_PRIOR        ,	/* 任务的优先级，必须唯一，数字越低优先级越高 */
                    WIFIDRIVER_TASK_PRIOR        ,	/* 任务ID，一般和任务优先级相同 */
                    (OS_STK *)&driver_task_stack[0],/* 指向任务栈栈底的指针。OS_STK_GROWTH 决定堆栈增长方向 */
                    WIFIDRIVER_TASK_STACKSIZE, /* 任务栈大小 */
                    (void *)0,	/* 一块用户内存区的指针，用于任务控制块TCB的扩展功能
                       （如任务切换时保存CPU浮点寄存器的数据）。一般不用，填0即可 */
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); /* 任务选项字 */

    if(err != OS_ERR_NONE)
    {
        APP_TRACE("driver create task failed\r\n");
        return -ENOMEM;
    }
    else
    {
        OSTaskNameSet(WIFIDRIVER_TASK_PRIOR, "wifi_driver", &err);
    }

    status = rsi_wireless_init(0, 0);
    if(status != RSI_SUCCESS)
    {
        APP_TRACE("WIFI wireless initialization failed.\r\n");
        return status;
    }
    else
        APP_TRACE("WIFI wireless initialization ok.\r\n");

    status = rsi_wireless_antenna(1, 0, 0);
    if(status != RSI_SUCCESS)
        APP_TRACE("configure antenna failed.\r\n");

    while(1)
    {
      status = rsi_wlan_scan(wifi_ssid, 0, NULL, 0);
      if(status != RSI_SUCCESS)
      {
        APP_TRACE("Scan ssid %s failed\r\n.", wifi_ssid);
        OSTimeDly(MS_TO_TICKS(100));
      }
      else
      {
        APP_TRACE("wifi %s found!\r\n.", wifi_ssid);
        break;
      }
    }

    APP_TRACE("connecting to ap: %s\r\n", wifi_ssid);

    while(1)
    {
        //! Connect to an Access point
      status = rsi_wlan_connect((int8_t *)wifi_ssid, RSI_WPA_WPA2_MIXED, wifi_password);
      if(status != RSI_SUCCESS)
      {
        APP_TRACE("WIFI connect AP failed.\r\n");
        OSTimeDly(MS_TO_TICKS(100));
        //return status;
      }
      else
      {
        APP_TRACE("WIFI connect AP OK.\r\n");
        break;
      }
    }

    while(1)
    {
      //! Configure IP
      if(local_ip == 0)   //Use DHCP
        status = rsi_config_ipaddress(RSI_IP_VERSION_4, RSI_DHCP, 0, 0, 0, NULL, 0,0);
      else
        status = rsi_config_ipaddress(RSI_IP_VERSION_4, RSI_STATIC, (uint8_t *)&local_ip, (uint8_t *)&subnet_mask, (uint8_t *)&gateway, NULL, 0,0);
      if(status != RSI_SUCCESS)
      {
        APP_TRACE("wifi config ip address failed.\r\n");
        //return status;
      }
      else
        break;
    }

    rsi_rsp_wireless_info_t wlan_info;

    rsi_wlan_get(RSI_WLAN_INFO, (uint8_t*)&wlan_info, sizeof(wlan_info));

    APP_TRACE("MAC address: %02x-%02x-%02x-%02x-%02x-%02x\r\n",
              wlan_info.mac_address[0], wlan_info.mac_address[1],
              wlan_info.mac_address[2], wlan_info.mac_address[3],
              wlan_info.mac_address[4], wlan_info.mac_address[5]);

    APP_TRACE("IP address: %d.%d.%d.%d\r\n",
              wlan_info.ipv4_address[0], wlan_info.ipv4_address[1],
              wlan_info.ipv4_address[2], wlan_info.ipv4_address[3]);
#if 0
    controller_ip = MAKE_IP(wlan_info.ipv4_address[0], wlan_info.ipv4_address[1], wlan_info.ipv4_address[2], 79);
    APP_TRACE("Controller IP: %d.%d.%d.%d\r\n",
              wlan_info.ipv4_address[0], wlan_info.ipv4_address[1],
              wlan_info.ipv4_address[2], 79);
#endif

    return 0;
}

uint32_t inet_addr(const char* ip_str)
{
    uint32_t ip = 0;
    uint32_t tmp;
    const char *endptr = NULL;
    const char *pstr = ip_str;

    int i;

    for(i=0; i<4; i++)
    {
        while(*pstr == ' ')
          pstr++;

        ip <<= 8;
        tmp = strtoul(pstr, &endptr, 10);
        ip |= tmp;

        pstr = endptr;
        while(*pstr == ' ')
            pstr++;

        if(i == 3) continue;

        if(*pstr != '.')
            break;
        else
            pstr++;
    }

    if(i < 4)
        return 0;
    else
        return ip;
}
