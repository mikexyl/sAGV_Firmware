#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#include "ucos_ii.h"

#include "app_cfg.h"
#include "common/list.h"
#include "ff.h"
#include "common/circ_buf.h"
#include "bsp_uart.h"
#include "common/PosixError.h"
#include "common/log_print.h"

#define     LOG_BUFFER_SIZE     2048
#define     MAX_CALLBACK_NUM    4

static  char   log_mem[LOG_BUFFER_SIZE];
static  struct  circ_buf buffer = {log_mem, 0, 0};
static char     tmp_logbuf[MAX_PRINT_LENGTH];

/*  in buffer mode:
 *      protect the circ buf 
 *  in nobuffer mode:
 *      protectand the output port
 */
static OS_EVENT             *mutex=NULL;
/*To protect the callback_list*/
static OS_EVENT             *list_lock=NULL;

static  OS_FLAG_GRP         *signal = NULL;

typedef struct
{
    list_head_t         list_node;

    sendout_callback_t  func;
    uint32_t            param;
}outputcb_desc_t;

static list_head_t callback_list = {&callback_list, &callback_list};
static outputcb_desc_t callback_mempool[MAX_CALLBACK_NUM];
static OS_MEM   *cbmem;

#define LOG_TASK_STACKSIZE  512
static OS_STK log_task_stack[LOG_TASK_STACKSIZE];

static void log_task(void *param);
static int  log_put(const char *str, uint32_t len);
static int  write_circbuf(struct circ_buf* buf, uint32_t buf_size,
                         const uint8_t *data, uint32_t len);
static int  read_circbuf(struct circ_buf* buf, uint32_t buf_size,
                        uint8_t *data, uint32_t len);
static int  out_put(const uint8_t *data, uint32_t len);
static void flush_tmr_callback(void *tmr, void*data);

static bool inited = false;

int log_init(void)
{
    uint8_t err;
    OS_TMR    *flush_tmr;

    mutex = OSMutexCreate(OS_PRIO_MUTEX_CEIL_DIS, &err);
    if(mutex == NULL)
    {
        printf("Log: create mutex failed with err = %d\r\n",
               err);
        return -PERR_ENOMEM;
    }

    list_lock = OSMutexCreate(OS_PRIO_MUTEX_CEIL_DIS, &err);
    if(list_lock == NULL)
    {
        printf("Log: create list lock failed with err = %d\r\n",
               err);
        return -PERR_ENOMEM;
    }

    list_lock = OSMutexCreate(OS_PRIO_MUTEX_CEIL_DIS, &err);
    if(list_lock == NULL)
    {
        printf("Log: create list lock failed with err = %d\r\n",
               err);
        return -PERR_ENOMEM;
    }

    signal = OSFlagCreate(0, &err);
    if(signal == NULL)
    {
        printf("Log: create flag failed.%d\r\n", err);
        return -PERR_ENOMEM;
    }

    cbmem = OSMemCreate(callback_mempool, MAX_CALLBACK_NUM, sizeof(outputcb_desc_t), &err);
    if(cbmem == NULL)
    {
        printf("Log: create memory partition failed with %d.", err);
        return -PERR_ENOMEM;
    }

    //定时将缓存的log输出
    flush_tmr = OSTmrCreate(10, 10, OS_TMR_OPT_PERIODIC,
                          flush_tmr_callback, NULL, "log-tmr", &err);
    OSTmrStart(flush_tmr, &err);

#ifndef NO_SDCARD
    DIR     d;
    FRESULT result;
    result = f_opendir(&d, LOGFILE_PATH);
    if(result == FR_NO_PATH)
    {
        result = f_mkdir(LOGFILE_PATH);
        if(result != FR_OK)
          printf("Create directory failed with %x.\r\n", result);

    }
#endif

#ifndef LOG_NOBUFFER
    err = OSTaskCreateExt(log_task,	/* 启动任务函数指针 */
                          (void *)0,		/* 传递给任务的参数 */
                          (OS_STK *)&log_task_stack[LOG_TASK_STACKSIZE - 1], /* 指向任务栈栈顶的指针 */
                          LOG_TASK_PRIOR        ,	/* 任务的优先级，必须唯一，数字越低优先级越高 */
                          LOG_TASK_PRIOR        ,	/* 任务ID，一般和任务优先级相同 */
                          (OS_STK *)&log_task_stack[0],/* 指向任务栈栈底的指针。OS_STK_GROWTH 决定堆栈增长方向 */
                          LOG_TASK_STACKSIZE, /* 任务栈大小 */
                          (void *)0,	/* 一块用户内存区的指针，用于任务控制块TCB的扩展功能
                                         （如任务切换时保存CPU浮点寄存器的数据）。一般不用，填0即可 */
                          OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); /* 任务选项字 */

    if(err != OS_ERR_NONE)
    {
      printf("Log: create task failed\r\n");
      return -PERR_ENOMEM;
    }
    else
    {
      inited = true;
      OSTaskNameSet(LOG_TASK_PRIOR, "log", &err);
      return 0;
    }
#endif
}

uint32_t log_register_outputcb(sendout_callback_t func, uint32_t param)
{
    outputcb_desc_t *cbdesc;
    uint8_t         err;

    if(func == NULL)
      return NULL;

    cbdesc = (outputcb_desc_t*)OSMemGet(cbmem, &err);
    if(cbdesc == NULL)
    {
        printf("Log: get memory failed with %d\r\n", err);
        return NULL;
    }

    cbdesc->func = func;
    cbdesc->param = param;

    OSMutexPend(list_lock, 0, &err);
    list_add_tail(&(cbdesc->list_node), &callback_list);
    OSMutexPost(list_lock);

    return (uint32_t)cbdesc;
}

int log_unregister_outputcb(uint32_t hd)
{
    uint8_t err;

    outputcb_desc_t *desc = (outputcb_desc_t*)hd;

    OSMutexPend(list_lock, 0, &err);
    list_del(&(desc->list_node));
    OSMutexPost(list_lock);

    OSMemPut(cbmem, desc);

    return 0;
}

int log_printf(const char* fmt, ...)
{
    va_list argptr;
    int     len;

    va_start(argptr, fmt);
    len = vsprintf(tmp_logbuf, fmt, argptr);
    va_end(argptr);

    log_put(tmp_logbuf, len);

    return len;
}

extern int cat(const TCHAR*fname);

#ifndef LOG_NOBUFFER
static void log_task(void *param)
{
    uint8_t err;
    uint8_t buf[128];
    int len;

#if 0
    TCHAR   file_name[64];
    sprintf(file_name, "%s/%s_%04d%02d%02d.log",
            LOGFILE_PATH,
            LOGFILE_NAME_PREFIX,
            2017,6,2);
    //cat(file_name);
    f_unlink(file_name);
#endif

    while(1)
    {
        OSFlagPend(signal, 1, OS_FLAG_WAIT_SET_ALL|OS_FLAG_CONSUME, 0, &err);
        while(1)
        {
            len = read_circbuf(&buffer, LOG_BUFFER_SIZE, buf, sizeof(buf));
            if(len > 0)
              out_put(buf, len);
            else
                break;
        }
    }
}
#endif

static int log_put(const char *str, uint32_t len)
{
    uint8_t err;

    if(!inited)
    {
        bsp_UartSend(USART1, (const uint8_t*)str, len);
        return len;
    }

#ifdef  LOG_NOBUFFER
    OSMutexPend(mutex, 0, &err);
    out_put(str, len);
    OSMutexPost(mutex);
#else
    write_circbuf(&buffer, LOG_BUFFER_SIZE, (const uint8_t*)str, len);

    if( (str[len-1] == '\r') || (str[len-1] == '\n') )
		{
        OSFlagPost(signal, 1, OS_FLAG_SET, &err);
    }
#endif

    return len;
}

static int write_circbuf(struct circ_buf* buf, uint32_t buf_size, const uint8_t *data, uint32_t len)
{
    uint32_t    cover_len = 0;
    uint32_t    space;
    bool        flush = false;
    uint32_t    cnt = 0;
    uint32_t    cplen;
    uint8_t err;

    OSMutexPend(mutex, 0, &err);
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

    //log数据已超过 buffer 1/3时通知输出
    if(CIRC_CNT(buf->head, buf->tail, buf_size) > buf_size/3)
        flush = true;

    OSMutexPost(mutex);

    if(flush)
        OSFlagPost(signal, 1, OS_FLAG_SET, &err);

    return cnt;
}

static int read_circbuf(struct circ_buf* buf, uint32_t buf_size, uint8_t *data, uint32_t len)
{
    uint32_t    num;
    uint32_t    rdlen;
    uint32_t    cplen;
    uint32_t    cnt = 0;
    uint8_t err;

    OSMutexPend(mutex, 0, &err);

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

    OSMutexPost(mutex);

    return cnt;
}

static int write_log_file(const uint8_t *data, uint32_t len)
{
    FIL     file;
    TCHAR   file_name[64];
    FRESULT result;
    UINT    bw;

    sprintf(file_name, "%s/%s.log",
            LOGFILE_PATH,
            LOGFILE_NAME_PREFIX);

    result = f_open(&file, file_name, FA_WRITE|FA_OPEN_ALWAYS);
    if(result != FR_OK)
    {
        printf("Log: open file %s faild with %d\r\n", file_name, result);
        return -PERR_ENODEV;
    }

    f_lseek(&file, f_size(&file));

    f_write(&file, data, len, &bw);

CLOSE_AGAIN:
    result = f_close(&file);
    if(result != FR_OK)
    {
      printf("log close file failed.\r\n");
      goto CLOSE_AGAIN;
    }

    return 0;
}

static int out_put(const uint8_t *data, uint32_t len)
{
    list_head_t     *node;
    outputcb_desc_t *desc;

#ifndef NO_SDCARD
    write_log_file(data, len);
#endif

    uint8_t         err;
    OSMutexPend(list_lock, 0, &err);
    if(err != OS_ERR_NONE)
        printf("error%d on list_lock pend\r\n", err);

    list_for_each(node, &callback_list)
    {
        desc = (outputcb_desc_t*)node;

        if(desc->func)
        {
            desc->func(desc->param, (const char*)data, len);
        }
    }

    OSMutexPost(list_lock);

    return len;
}

static void flush_tmr_callback(void *tmr, void*data)
{
    uint8_t err;

    OSFlagPost(signal, 1, OS_FLAG_SET, &err);
}
