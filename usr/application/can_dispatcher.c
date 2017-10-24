#include <string.h>
#include <stdbool.h>

#include <ucos_ii.h>

#include "app_cfg.h"
#include "common/list.h"
#include "common/PosixError.h"
#include "common/tools.h"
#include "can_dispatcher.h"

typedef struct
{
    list_head_t node;

    char name[16];
    msg_processor_t msg_processor;
}can_user_t;

#if 0
typedef struct
{
}
#endif

#define CAN_TASK_STACKSIZE    512
static OS_STK can_task_stack[CAN_TASK_STACKSIZE];

static CAN_TypeDef *port;
static list_head_t userlist_head = {&userlist_head, &userlist_head};
static OS_EVENT *bus_lock;

static can_user_t usermem_pool[CANDISPATCHER_MAX_USERNUM];
static OS_MEM *usermem;


static void can_task(void *param);

int can_dispatcher_init(CAN_TypeDef *canport, uint16_t baudrate_k)
{
    uint8_t err;

    port = canport;

    bsp_InitCan(port, baudrate_k);

    usermem = OSMemCreate(usermem_pool, CANDISPATCHER_MAX_USERNUM, 
                sizeof(can_user_t), &err);

    bus_lock = OSMutexCreate(2, &err);
    if(bus_lock == NULL)
    {
        printf("can dispatcher create mutex failed with err = %d\r\n",
               err);
        return -1;
    }

    return 0;
}

int can_dispatcher_register_user(const char*name, msg_processor_t proc)
{
    uint8_t err;
    can_user_t *newuser;

    newuser = OSMemGet(usermem, &err);
    if(newuser == NULL)
      return 0;

    if(name == NULL)
        newuser->name[0] = 0;
    else
    {
      if(strlen(name) > 15)
      {
        memcpy(newuser->name, name, 15);
        newuser->name[15] = 0;
      }
      else
        strcpy(newuser->name, name);
    }

    newuser->msg_processor = proc;

    list_add_tail(&newuser->node, &userlist_head);

    return (int)newuser;
}

int can_dispatcher_run(void)
{
    uint8_t err;

    OSTaskCreateExt(can_task,	/* 启动任务函数指针 */
                    (void *)0,		/* 传递给任务的参数 */
                    (OS_STK *)&can_task_stack[CAN_TASK_STACKSIZE - 1], /* 指向任务栈栈顶的指针 */
                    CAN_TASK_PRIOR        ,	/* 任务的优先级，必须唯一，数字越低优先级越高 */
                    CAN_TASK_PRIOR        ,	/* 任务ID，一般和任务优先级相同 */
                    (OS_STK *)&can_task_stack[0],/* 指向任务栈栈底的指针。OS_STK_GROWTH 决定堆栈增长方向 */
                    CAN_TASK_STACKSIZE, /* 任务栈大小 */
                    (void *)0,	/* 一块用户内存区的指针，用于任务控制块TCB的扩展功能
                       （如任务切换时保存CPU浮点寄存器的数据）。一般不用，填0即可 */
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); /* 任务选项字 */

    OSTaskNameSet(CAN_TASK_PRIOR, "can-dispatcher", &err);

    return 0;
}

int can_dispatcher_send_msg(int user, CanTxMsg *msg)
{
    uint8_t err;

    OSMutexPend(bus_lock, 0, &err);
    bsp_CanSend(port, msg);
    OSMutexPost(bus_lock);
#if 0
    APP_TRACE("Can send frame: \r\n");
    can_print_frame((CanRxMsg*)msg);
#endif
    return 0;
}

void can_print_frame(CanRxMsg *rxmsg)
{
    if(rxmsg->IDE == CAN_Id_Standard)
        APP_TRACE("STDID:%8x    Data(%d)", rxmsg->StdId, rxmsg->DLC);
    else if(rxmsg->IDE == CAN_Id_Extended)
        APP_TRACE("EXTID:%8x    Data(%d)", rxmsg->ExtId, rxmsg->DLC);

    memdump(rxmsg->Data, rxmsg->DLC);
}

static void can_task(void *param)
{
    CanRxMsg rxmsg;
    list_head_t *user;
    uint8_t err;
    int ret;

    APP_TRACE("can_task: task run.\r\n");

    while(true)
    {
        OSMutexPend(bus_lock, 0, &err);
        ret = bsp_CanReceive(port, &rxmsg);
        OSMutexPost(bus_lock);
        if(ret == 0)
        {
#if 0
            APP_TRACE("One frame received: ");
            can_print_frame(&rxmsg);
            bsp_CanSend(port, (CanTxMsg *)(&rxmsg));
#endif
            list_for_each(user, &userlist_head)
            {
    //            APP_TRACE("User: %s process frame.\r\n",
     //                     ((can_user_t*)user)->name);

                ret = ((can_user_t*)user)->msg_processor((int)user, &rxmsg);
                if(ret == PERR_EOK) break;
                else if(ret == -PERR_ENOTSUP) continue;
                else break;
            }
            continue;
        }

        OSTimeDlyHMSM(0,0,0,10);
    }
}
