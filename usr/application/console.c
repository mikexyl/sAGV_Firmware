#include    <string.h>
#include    <stdio.h>

#include    <ucos_ii.h>
#include    "bsp_uart.h"
#include    "app_cfg.h"
#include    "common/lwshell.h"
#include    "common/circ_buf.h"
#include    "common/PosixError.h"

#include "rsi_data_types.h"
#include "rsi_common_apis.h"
#include "rsi_wlan_apis.h"
#include "rsi_socket.h"
#include "rsi_error.h"

static void uart_console_task(void *param);
static int uart_get_char(uint32_t port, char* ch);
int uart_output(uint32_t port, const char*data, uint32_t len);

#define UART_CONSOLE_TASK_STACKSIZE    1024
static OS_STK uart_console_task_stack[UART_CONSOLE_TASK_STACKSIZE];

int uart_console_run(USART_TypeDef *uart_port)
{
    uint8_t err;

//    log_register_outputcb(uart_output, (uint32_t)USART1);

    err = OSTaskCreateExt(uart_console_task,	/* 启动任务函数指针 */
                          (void *)uart_port,		/* 传递给任务的参数 */
                          (OS_STK *)&uart_console_task_stack[UART_CONSOLE_TASK_STACKSIZE - 1], /* 指向任务栈栈顶的指针 */
                          UART_CONSOLE_TASK_PRIOR        ,	/* 任务的优先级，必须唯一，数字越低优先级越高 */
                          UART_CONSOLE_TASK_PRIOR        ,	/* 任务ID，一般和任务优先级相同 */
                          (OS_STK *)&uart_console_task_stack[0],/* 指向任务栈栈底的指针。OS_STK_GROWTH 决定堆栈增长方向 */
                          UART_CONSOLE_TASK_STACKSIZE, /* 任务栈大小 */
                          (void *)0,	/* 一块用户内存区的指针，用于任务控制块TCB的扩展功能
                                         （如任务切换时保存CPU浮点寄存器的数据）。一般不用，填0即可 */
                          OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); /* 任务选项字 */

    if(err != OS_ERR_NONE)
    {
      printf("uart_console: create task failed\r\n");
      return -PERR_ENOMEM;
    }
    else
    {
      OSTaskNameSet(UART_CONSOLE_TASK_PRIOR, "uart_console", &err);
      return 0;
    }
}


static void uart_console_task(void *param)
{
    console_run(uart_get_char, uart_output, (uint32_t)param);
}

static int uart_get_char(uint32_t port, char* ch)
{
    return bsp_UartReceive((USART_TypeDef*)port, (uint8_t*)ch, 1);
}

int uart_output(uint32_t port, const char*data, uint32_t len)
{
    if(len == 0)
      len = strlen((const char*)data);

    return bsp_UartSend((USART_TypeDef*)port, (const uint8_t*)data, (uint16_t)len);
}

/////////////////////////////////////////////////////////////////////////////////////////////
#if 1

/*
 *  UDP console, 同一时间只支持一个接入登录
 * */
static void udp_rx_callback(uint32_t sock, const struct sockaddr*addr, uint8_t *data, uint32_t len);
static void udp_console_task(void *param);
static int udp_get_char(uint32_t port, char* ch);
static int udp_output(uint32_t port, const char*data, uint32_t len);

#define UDP_CONSOLE_TASK_STACKSIZE     512
static struct sockaddr_in peer_addr;
static OS_STK udp_console_task_stack[UDP_CONSOLE_TASK_STACKSIZE];

#define     UDP_RXBUFFER_SIZE   64
static  char    buffer_mem[UDP_RXBUFFER_SIZE];
static  struct  circ_buf rxbuf = {buffer_mem, 0, 0};
static OS_EVENT *mutex=NULL;

int udp_console_run(uint16_t udp_port)
{
    uint8_t err;
    int32_t sock;
    struct sockaddr_in addr;
    int status;

    sock = socket_async(AF_INET, SOCK_DGRAM, 0, udp_rx_callback);
    if(sock < 0)
    {
        APP_TRACE("UDP console create socket failed.\r\n");
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family= AF_INET;
    addr.sin_port = htons(udp_port);

    status = bind(sock, (struct sockaddr *) &addr, sizeof(addr));
    if(status != RSI_SUCCESS)
    {
        APP_TRACE("UDP console bind port failed.\r\n");
        shutdown(sock, 0);
        return -1;
    }

    mutex = OSMutexCreate(OS_PRIO_MUTEX_CEIL_DIS, &err);
    if(mutex == NULL)
    {
        shutdown(sock, 0);
        printf("udp console create mutex failed with err = %d\r\n",
               err);
        return -PERR_ENOMEM;
    }

    err = OSTaskCreateExt(udp_console_task,	/* 启动任务函数指针 */
                          (void *)sock,		/* 传递给任务的参数 */
                          (OS_STK *)&udp_console_task_stack[UDP_CONSOLE_TASK_STACKSIZE - 1], /* 指向任务栈栈顶的指针 */
                          UDP_CONSOLE_TASK_PRIOR        ,	/* 任务的优先级，必须唯一，数字越低优先级越高 */
                          UDP_CONSOLE_TASK_PRIOR        ,	/* 任务ID，一般和任务优先级相同 */
                          (OS_STK *)&udp_console_task_stack[0],/* 指向任务栈栈底的指针。OS_STK_GROWTH 决定堆栈增长方向 */
                          UDP_CONSOLE_TASK_STACKSIZE, /* 任务栈大小 */
                          (void *)0,	/* 一块用户内存区的指针，用于任务控制块TCB的扩展功能
                                         （如任务切换时保存CPU浮点寄存器的数据）。一般不用，填0即可 */
                          OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); /* 任务选项字 */

    if(err != OS_ERR_NONE)
    {
        OSMutexDel(mutex, OS_DEL_NO_PEND, &err);
        shutdown(sock, 0);
        printf("udp_console: create task failed\r\n");
        return -PERR_ENOMEM;
    }
    else
    {
        OSTaskNameSet(UDP_CONSOLE_TASK_PRIOR, "udp_console", &err);
        return 0;
    }
}

static int write_circbuf(struct circ_buf* buf, uint32_t buf_size, const uint8_t *data, uint32_t len)
{
    uint32_t    cover_len = 0;
    uint32_t    space;
    uint32_t    cnt = 0;
    uint32_t    cplen;

    space = CIRC_SPACE(buf->head, buf->tail, buf_size);
    if(space < len)
        cover_len = len-space;

    for(int i=0; i<2; i++)
    {
        space = (buf_size - buf->head);
        //space = CIRC_SPACE_TO_END(buf->head, buf->tail, buf_size);
        cplen = ((len-cnt) >= space)? space:(len-cnt);

        memcpy(&buf->buf[buf->head], &data[cnt], cplen);

        buf->head += cplen;
        cnt += cplen;
        buf->head &= (buf_size-1);

        if(cnt == len)
            break;
    }

    if(cover_len > 0)
    {
        buf->tail += cover_len;     //(cover_len > cplen)?cplen:cover_len;
        buf->tail &= (buf_size-1);
    }

    return cnt;
}

static int read_circbuf(struct circ_buf* buf, uint32_t buf_size, uint8_t *data, uint32_t len)
{
    uint32_t    num;
    uint32_t    rdlen;
    uint32_t    cplen;
    uint32_t    cnt = 0;

    num = CIRC_CNT(buf->head, buf->tail, buf_size);
    rdlen = (num >= len)? len:num;

    for(int i=0; i<2; i++)
    {
        num = CIRC_CNT_TO_END(buf->head, buf->tail, buf_size);
        cplen = (num >= (rdlen-cnt))? (rdlen-cnt):num;

        memcpy(&data[cnt], &buf->buf[buf->tail], cplen);

        buf->tail += cplen;
        cnt += cplen;
        buf->tail &= (buf_size-1);

        if(cnt == rdlen)
          break;
    }

    return cnt;
}

static void udp_rx_callback(uint32_t sock, const struct sockaddr*addr, uint8_t *data, uint32_t len)
{
    uint8_t err;

    if(addr->sa_family != AF_INET)
        return;

    const struct sockaddr_in *src_addr = (const struct sockaddr_in *)addr;

    if( (peer_addr.sin_addr.s_addr != src_addr->sin_addr.s_addr)
      || (peer_addr.sin_port != src_addr->sin_port) )
    {
        peer_addr.sin_family = AF_INET;
        peer_addr.sin_addr.s_addr = src_addr->sin_addr.s_addr;
        peer_addr.sin_port = src_addr->sin_port;
    }

    OSMutexPend(mutex, 0, &err);
    write_circbuf(&rxbuf, UDP_RXBUFFER_SIZE, data, len);
    OSMutexPost(mutex);
}

static void udp_console_task(void *param)
{
    console_run(udp_get_char, udp_output, (uint32_t)param);
}

static int udp_get_char(uint32_t port, char* ch)
{
    int ret;
    uint8_t err;

    OSMutexPend(mutex, 0, &err);
    ret = read_circbuf(&rxbuf, UDP_RXBUFFER_SIZE, (uint8_t*)ch, 1);
    OSMutexPost(mutex);

    return ret;
}

int udp_output(uint32_t sock, const char*data, uint32_t len)
{
    if(len == 0)
      len = strlen((const char*)data);

    return sendto((int32_t)sock, (int8_t*)data, len, 0, (struct sockaddr*)&peer_addr, sizeof(peer_addr));
}
#endif
