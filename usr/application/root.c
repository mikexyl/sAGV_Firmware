#include <ucos_ii.h>
#include <stdio.h>
#include <stdint.h>
#include <bsp.h>
#include <bsp_tim_pwm.h>
#include <ff.h>

#include "app_cfg.h"
#include "common/log_print.h"
#include "can_dispatcher.h"
#include "bsp_io.h"
#include "bsp_led.h"
#include "bsp_wifi.h"
#include "bsp_pwm.h"
#include "bsp_wifi.h"
#include "configuration.h"
#include "agv.h"
#include "bms.h"
#include "agvcp.h"
#include "motors.h"
#include "Navigation.h"
#include "reverseMotionControl.h"

#include "rsi_wlan_apis.h"

#include "algorithm.h"

extern void test(void);
extern int wifi_init(void);
extern uint32_t inet_addr(const char* ip_str);

extern int uart_output(uint32_t port, const char*data, uint32_t len);
extern int uart_console_run(USART_TypeDef *uart_port);
extern int udp_console_run(uint16_t udp_port);

static void led_tmr_callback(void *tmr, void*data);

FATFS   fs;

bool  spi_debug =false;

int32_t agv_id = AGV_ID;
int32_t controller_ip = CONTROLLER_IP;
int32_t controller_port = CONTROLLER_PORT;

int32_t local_ip = WIFI_LOCAL_IP;
int32_t subnet_mask = 0;
int32_t gateway = 0;

int32_t local_port = AGVCP_LOCAL_UDP_PORT;

char wifi_ssid[32] = WIFI_AP_NAME;
char wifi_password[32] = WIFI_PASSWORD;
char local_ip_str[16]={0};
char remote_ip_str[16]={0};

extern int32_t RSI_BAND;
extern int32_t RSI_CONFIG_CLIENT_BAND;
extern int32_t RSI_SCAN_CHANNEL_BIT_MAP_2_4;
extern int32_t RSI_SCAN_CHANNEL_BIT_MAP_5;
extern int32_t RSI_CONFIG_CLIENT_SCAN_CHAN_BITMAP_2_4_GHZ;
extern int32_t RSI_CONFIG_CLIENT_SCAN_CHAN_BITMAP_5_0_GHZ;

static void print_devinfo(void)
{
    APP_TRACE("+-----------------------------------------------------\r\n");
    APP_TRACE("+               Sorting AGV(ID: %08X)            +\r\n", agv_id);
    APP_TRACE("+               SW-version:"SW_VERSION"                       +\r\n");
    APP_TRACE("+----------------------------------------------------+\r\n");
}
void root_task(void *p_arg)
{
    (void)p_arg;
    OS_TMR    *led_tmr;
    uint8_t   err;
    int       ret;
    uint32_t    log_cbhd;

    bsp_Init();
    CPU_Init();
    BSP_Tick_Init();
    bsp_InitLed();

  /* 检测CPU能力，统计模块初始化。该函数将检测最低CPU占有率 */
#if (OS_TASK_STAT_EN > 0)
    OSStatInit();
#endif

    /*USART1 used to output debug info by printf*/
    bsp_InitUart(USART1, 115200, USART_StopBits_1, USART_Parity_No);
    //bsp_InitUart(USART1, 230400, USART_StopBits_1, USART_Parity_No);

#ifndef NO_SDCARD
    ret = f_mount(&fs, "0:/", 0);
    if(ret != FR_OK)
        APP_TRACE("fs mount failed with %d\r\n", ret);
#endif

    uart_console_run(USART1);

    log_init();
    log_cbhd = log_register_outputcb(uart_output, (uint32_t)USART1);

#ifndef NO_SDCARD
    load_configuration();
#endif

    RSI_CONFIG_CLIENT_BAND = RSI_BAND;
    RSI_CONFIG_CLIENT_SCAN_CHAN_BITMAP_2_4_GHZ = RSI_SCAN_CHANNEL_BIT_MAP_2_4;
    if(RSI_BAND == RSI_BAND_5GHZ)
    {
        RSI_SCAN_CHANNEL_BIT_MAP_5 = RSI_SCAN_CHANNEL_BIT_MAP_2_4;
        RSI_CONFIG_CLIENT_SCAN_CHAN_BITMAP_5_0_GHZ = RSI_SCAN_CHANNEL_BIT_MAP_2_4;
    }

    uint32_t ip;
    ip = inet_addr(remote_ip_str);
    if(ip != 0)
    {
        //ntohl
        controller_ip = (ip>>24)|(ip>>8)&0xFF00
          |(ip<<8)&0xFF0000|(ip<<24);
    }

    motors_init();
    MOTORS_ATTACH();

    print_devinfo();
    //wifi_init();

    udp_console_run(8040);

    OSTimeDly(MS_TO_TICKS(10));
    log_unregister_outputcb(log_cbhd);

    test();
    //OSTaskSuspend(OS_PRIO_SELF);

    can_dispatcher_init(CAN2, 500);

    /*导航摄像头初始化*/
    NavigationInit();
    bms_init();

    agv_init(agv_id);
    can_dispatcher_run();
    bms_run();
		
		canSendTest();
		
    //Wait for thr camera ready
    //OSTimeDly(MS_TO_TICKS(10*1000));
    /*
     * 先关闭，在打开。
     * 避免控制器断电而摄像头没断电的情况下，不上报位置信息。
     * */
    NavigationOnOff(0);
    OSTimeDly(MS_TO_TICKS(500));
    NavigationOnOff(1);

    //等待摄像头上报导航信息，初始化车头方向
    InitAgvHeadDir();
    motors_power_onoff(1);

    //延时500ms,防止电机锁定太快，没有推到位的情况
    OSTimeDly(MS_TO_TICKS(500));

    /*动力上电*/
    motors_power_onoff(1);

    //动力上电有延时
    OSTimeDly(MS_TO_TICKS(1500));

    //agv_run();

    /*
     * timer to drive the led indicator flickers
     * period = 500ms
     * */
    led_tmr = OSTmrCreate(500, 500, OS_TMR_OPT_PERIODIC,
                          led_tmr_callback, NULL, "led-tmr", &err);

    OSTmrStart(led_tmr, &err);

    APP_TRACE("Initialize over.\r\n"
              "root task suspending...\r\n");

    OSTaskSuspend(OS_PRIO_SELF);
}

static void led_tmr_callback(void *tmr, void*data)
{
    bsp_LedToggle();
}
