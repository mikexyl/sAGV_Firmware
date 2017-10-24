#include <stdlib.h>
#include <string.h>

#include  <ucos_ii.h>
#include  <common/PosixError.h>
#include  <ff.h>
#include "minIni.h"
#include "app_cfg.h"
#include "log_print.h"
//#include "PosixError.h"

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

#define USE_ASYNC   1

void ls(const TCHAR* path)
{
    DIR dir;
    FRESULT ret;
    FILINFO finfo;

    ret = f_opendir(&dir, path);
    if(ret != FR_OK)
    {
        log_printf("open dir (%s) failed %d.\r\n", path, ret);
        return ;
    }

    log_printf("directory: %s\r\n", path);

    while(1)
    {
        ret = f_readdir(&dir, &finfo);
        if( (ret != FR_OK) || (finfo.fname[0] == 0) )
            break;

        if(finfo.fattrib & AM_DIR)
            log_printf("\td: %s\r\n", finfo.fname);
        else
            log_printf("\tf: %s\r\n", finfo.fname);
    }
}

#if 1
int create_test_file(const TCHAR*name)
{
    FIL file;
    FRESULT ret;

    ret = f_open(&file, name, FA_WRITE|FA_CREATE_ALWAYS);

    if(ret != FR_OK)
    {
        APP_TRACE("Open file failed.\r\n");
        return -PERR_ENODEV;
    }

    f_puts("test fatfs and sd.\r\n", &file);
    f_puts("you can see these infomation means the fatfs and sd card is working well.\r\n", &file);

    f_close(&file);

    APP_TRACE("Create file \"%s\" ok\r\n", name);

    return 0;
}

int cat(const TCHAR*fname)
{
    FIL file;
    FRESULT ret;
    char    buf[256];

    ret = f_open(&file, fname, FA_READ|FA_OPEN_EXISTING);
    if(ret != FR_OK)
    {
        printf("Open file \"%s\"failed with errcode = %d\r\n", fname, ret);
        return -PERR_ENODEV;
    }

    printf("cat file: %s (%d bytes)\r\n", fname, f_size(&file));

    printf("-----------------------\r\n");
    while(!f_eof(&file))
    {
        if(f_gets(buf, sizeof(buf), &file) == NULL)
            break;

        printf("%s", buf);
    }
    printf("-----------------------\r\n");

    f_close(&file);

    return 0;
}

#endif

#if USE_ASYNC
static uint8_t     rxbuf_1[512];
static uint32_t    rxlen_1 = 0;
static struct sockaddr_in addr1;

static uint8_t     rxbuf_2[512];
static uint32_t    rxlen_2 = 0;
static struct sockaddr_in addr2;

static OS_EVENT *mutex[2];

static void rx_callback_1(uint32_t sock, const struct sockaddr*addr, uint8_t *data, uint32_t len)
{
    uint8_t err;

    if(rxlen_1 != 0)
      APP_TRACE("*");

    if(len > sizeof(rxbuf_1))
      return ;

    OSMutexPend(mutex[0], 0,  &err);

    memcpy(rxbuf_1, data, len);
    rxlen_1 = len;

    memcpy(&addr1, addr, sizeof(struct sockaddr_in));
    //addr1.sin_addr.s_addr = addr->sin_addr.s_addr;
    //addr1.sin_port = addr->sin_port;

    OSMutexPost(mutex[0]);

    if(addr->sa_family != AF_INET)
      APP_TRACE("sa_family error.\r\n");
    else
    {
        const struct sockaddr_in *src = (const struct sockaddr_in *)addr;
        APP_TRACE("Packet from: %d.%d.%d.%d:%d\r\n",
                  src->sin_addr.s_addr&0xFF, (src->sin_addr.s_addr>>8)&0xFF,
                  (src->sin_addr.s_addr>>16)&0xFF, (src->sin_addr.s_addr>>24)&0xFF,
                  src->sin_port);
    }
    //APP_TRACE("%d bytes received from link1\r\n", len);
}

static void rx_callback_2(uint32_t sock, const struct sockaddr*addr, uint8_t *data, uint32_t len)
{
    uint8_t err;

    //APP_TRACE("%d bytes received from link2\r\n", len);

    if(len > sizeof(rxbuf_2))
      return ;

    OSMutexPend(mutex[1], 0, &err);

    memcpy(rxbuf_2, data, len);
    rxlen_2 = len;

    memcpy(&addr2, addr, sizeof(struct sockaddr_in));
    OSMutexPost(mutex[1]);

    if(addr->sa_family != AF_INET)
      APP_TRACE("sa_family error.\r\n");
    else
    {
        const struct sockaddr_in *src = (const struct sockaddr_in *)addr;
        APP_TRACE("Packet from: %d.%d.%d.%d:%d\r\n",
                  src->sin_addr.s_addr&0xFF, (src->sin_addr.s_addr>>8)&0xFF,
                  (src->sin_addr.s_addr>>16)&0xFF, (src->sin_addr.s_addr>>24)&0xFF,
                  src->sin_port);
    }
}

#endif

#if 0
#define DUMPER_W        80
static void down_tohome()
{
    APP_TRACE("dumper down to home.\r\n");
    dumper_down(DUMPER_W);

    while(!is_dumper_homing())
        ;

    dumper_stop();
}

static void up_to_angle(int angle)
{
    APP_TRACE("dumper up to angle.\r\n");

    dumper_up(DUMPER_W);
    while(dumper_angle()<angle)
        ;
    //OSTimeDly(MS_TO_TICKS(2000));

    dumper_stop();
}

#endif

int show_config_item(const char* sec, const char*key, const char* val, const void*data)
{
    printf("\t%s->%s = %s\r\n",sec, key, val);

		return 1;
}

#define CONFIG_FILENAME     "config.ini"

static void udp_echo(void*param)
{
  int32_t     server_socket;
  struct      sockaddr_in server_addr, client_addr;
  int32_t     status = RSI_SUCCESS;
  int32_t       addr_size;
  uint8_t       recv_buffer[256];
  uint16_t      port = (int32_t)param;

  APP_TRACE("udp echo run on port: %d\r\n", port);

  //! Create socket
#if !(USE_ASYNC)
  server_socket = socket(AF_INET, SOCK_DGRAM, 0);
#else
  if(port == 8003)
    server_socket = socket_async(AF_INET, SOCK_DGRAM, 0, rx_callback_1);
  else
    server_socket = socket_async(AF_INET, SOCK_DGRAM, 0, rx_callback_2);
#endif

  if(server_socket < 0)
  {
    APP_TRACE("udp_echo create socket error\r\n");
    status = rsi_wlan_get_status();
    return ;
  }

  //! Set server structure
  memset(&server_addr, 0, sizeof(server_addr));  

  //! Set family type
  server_addr.sin_family= AF_INET;

  //! Set local port number
  server_addr.sin_port = htons(port);

  //! Bind socket
  status = bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr));
  if(status != RSI_SUCCESS)
  {
    status = rsi_wlan_get_status();
    shutdown(server_socket, 0);
    return;
  }

  addr_size = sizeof(server_addr);

#if 0
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = MAKE_IP(3,1,168,192);
    client_addr.sin_port = htons(port);

    status = connect(server_socket, (struct sockaddr *) &client_addr, sizeof(client_addr));
    if(status != RSI_SUCCESS)
    {
        APP_TRACE("socket connect failed\r\n");
        return ;
    }
#endif

    APP_TRACE("create connect on port %d ok.\r\n", port);
#if 0
    for(int i=0; i<36;i++)
      recv_buffer[i] = i;
    while(1)
    {
        APP_TRACE("Send data.%d\r\n", port);
        sendto(server_socket, (int8_t*)recv_buffer, 36, 0,
               (struct sockaddr *)&client_addr, addr_size);
        APP_TRACE("Send data over%d\r\n", port);

        OSTimeDly(10);
    }
#endif

  while(1)
  {
#if !(USE_ASYNC)
      status = recvfrom(server_socket, (int8_t*)recv_buffer, sizeof(recv_buffer), 0,
                        (struct sockaddr *)&client_addr, &addr_size);
#else
      uint8_t err;
      if(port == 8003)
      {
            if(rxlen_1 != 0)
            {
              OSMutexPend(mutex[0], 0,  &err);
              status = rxlen_1;
              memcpy(recv_buffer, rxbuf_1, status);
              rxlen_1 = 0;
              memcpy(&client_addr, &addr1, sizeof(struct sockaddr_in));
              OSMutexPost(mutex[0]);
            }
            else
              status = 0 ;
      }
      else
      {
            if(rxlen_2 != 0)
            {
              OSMutexPend(mutex[1], 0,  &err);
              status = rxlen_2;
              memcpy(recv_buffer, rxbuf_2, status);
              rxlen_2 = 0;
              memcpy(&client_addr, &addr2, sizeof(struct sockaddr_in));
              OSMutexPost(mutex[1]);
            }
            else
              status = 0 ;
      }
#endif
      if(status < 0)
      {
        APP_TRACE("recv error.\r\n");
        status = rsi_wlan_get_status();
        shutdown(server_socket, 0);
        return;
      }
      else if(status > 0)
      {
        //APP_TRACE("%d bytes sendding port %d\r\n", status, port);
        sendto(server_socket, (int8_t*)recv_buffer, status, 0,
               (struct sockaddr *)&client_addr, addr_size);
        //APP_TRACE("send over %d\r\n", port);
      }
      else
        OSTimeDly(5);
  }

  APP_TRACE("Recv over\t\n");
  OSTaskSuspend(OS_PRIO_SELF);
}

#define UDPECHO_TASK_STACKSIZE  512
static OS_STK udpecho_task_stack[1][UDPECHO_TASK_STACKSIZE];

static void test_wifi(void)
{
#if USE_ASYNC
    uint8_t err;

    mutex[0] = OSMutexCreate(OS_PRIO_MUTEX_CEIL_DIS, &err);
    mutex[1] = OSMutexCreate(OS_PRIO_MUTEX_CEIL_DIS, &err);
#endif

#if USE_ASYNC
    err = OSTaskCreateExt(udp_echo,	/* 启动任务函数指针 */
                    (void *)8003,		/* 传递给任务的参数 */
                    (OS_STK *)&udpecho_task_stack[0][UDPECHO_TASK_STACKSIZE- 1], /* 指向任务栈栈顶的指针 */
                    30,	/* 任务的优先级，必须唯一，数字越低优先级越高 */
                    30,	/* 任务ID，一般和任务优先级相同 */
                    (OS_STK *)&udpecho_task_stack[0][0],/* 指向任务栈栈底的指针。OS_STK_GROWTH 决定堆栈增长方向 */
                    UDPECHO_TASK_STACKSIZE, /* 任务栈大小 */
                    (void *)0,	/* 一块用户内存区的指针，用于任务控制块TCB的扩展功能
                       （如任务切换时保存CPU浮点寄存器的数据）。一般不用，填0即可 */
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); /* 任务选项字 */

    if(err != OS_ERR_NONE)
    {
        APP_TRACE("wifi-test create task failed\r\n");
        return ;
    }

#if 0
    err = OSTaskCreateExt(udp_echo,	/* 启动任务函数指针 */
                    (void *)8004,		/* 传递给任务的参数 */
                    (OS_STK *)&udpecho_task_stack[1][UDPECHO_TASK_STACKSIZE- 1], /* 指向任务栈栈顶的指针 */
                    31,	/* 任务的优先级，必须唯一，数字越低优先级越高 */
                    31,	/* 任务ID，一般和任务优先级相同 */
                    (OS_STK *)&udpecho_task_stack[1][0],/* 指向任务栈栈底的指针。OS_STK_GROWTH 决定堆栈增长方向 */
                    UDPECHO_TASK_STACKSIZE, /* 任务栈大小 */
                    (void *)0,	/* 一块用户内存区的指针，用于任务控制块TCB的扩展功能
                       （如任务切换时保存CPU浮点寄存器的数据）。一般不用，填0即可 */
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); /* 任务选项字 */

    if(err != OS_ERR_NONE)
    {
        APP_TRACE("wifi test create task-2 failed\r\n");
        return ;
    }
#endif

#else
    udp_echo((void*)8003);
#endif

}

void test(void)
{
    test_wifi();
#if 0
    ini_putl("DEVICE", "agvname", agv_id, CONFIG_FILENAME);
    ini_puts("CONTROLER", "ip", "192.168.0.243", CONFIG_FILENAME);
    ini_putl("CONTROLER", "port", 3000, CONFIG_FILENAME);
    ini_browse(show_config_item, 0, CONFIG_FILENAME);

    while(1)
    {
        APP_TRACE(__FILE__"Log test. \r\n");
        OSTimeDly(200);
    }
#endif
#if 0
    motors_power_onoff(1);
    MOTORS_ATTACH();

    OSTimeDly(MS_TO_TICKS(2000));

    //dumper_up(50);
#if 1
    if(is_dumper_homing())
        up_to_angle(60);
    else
        down_tohome();
#endif
    OSTaskSuspend(OS_PRIO_SELF);
#else

    //f_chdir(LOGFILE_PATH);
    //create_test_file("test.txt");
    //cat("test.txt");
#endif
}

