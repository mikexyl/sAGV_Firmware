#include <stdint.h>

#include  <ucos_ii.h>

#include "common/PosixError.h"
#include "common/log_print.h"
#include "common/tools.h"
#include "agv.h"

#include "app_cfg.h"
#include "can_dispatcher.h"
#include "bsp_io.h"
#include "dio.h"
#include "bms.h"

#define BMSERR_POWER_LOW                 1
#define BMSERR_POWER_UPDATE_OUTOFTM      2

static battery_info_t  s_battery_info;
static int can_user;

static uint32_t power_update_time = 0;
static uint32_t power_query_time = 0;

static uint32_t bms_error = 0;

static void query_battery_info_tmrcb(void* tmr, void*data);
static int  can_msg_process(int usr, CanRxMsg* msg);

int bms_init(void)
{
    s_battery_info.state = BATTERY_STATE_NORMAL;
    s_battery_info.voltage = 0;
    s_battery_info.current = 0;
    s_battery_info.power = 0;
    s_battery_info.temperature = 0;

    can_user = can_dispatcher_register_user("bms", can_msg_process);

    return 0;
}

#define BMS_TASK_STACKSIZE  256
static OS_STK bms_task_stack[BMS_TASK_STACKSIZE];

static void bms_task(void *param)
{
    while(1)
    {
        bms_query();
        OSTimeDly(MS_TO_TICKS(5*1000));
    }
}

int bms_run(void)
{
    uint8_t err;

    power_update_time = OSTimeGet();

#if 0
    OS_TMR  *query_tmr;

    query_tmr = OSTmrCreate(10, 10, OS_TMR_OPT_PERIODIC,
                          query_battery_info_tmrcb, NULL, "bms-query", &err);
    if(query_tmr == NULL)
    {
        APP_TRACE("BMS create timer failed.\r\n");
        return -PERR_ENOMEM;
    }

    OSTmrStart(query_tmr, &err);

    return 0;
#else
    err = OSTaskCreateExt(bms_task,	/* 启动任务函数指针 */
                          (void *)0,		/* 传递给任务的参数 */
                          (OS_STK *)&bms_task_stack[BMS_TASK_STACKSIZE - 1], /* 指向任务栈栈顶的指针 */
                          BMS_TASK_PRIOR        ,	/* 任务的优先级，必须唯一，数字越低优先级越高 */
                          BMS_TASK_PRIOR        ,	/* 任务ID，一般和任务优先级相同 */
                          (OS_STK *)&bms_task_stack[0],/* 指向任务栈栈底的指针。OS_STK_GROWTH 决定堆栈增长方向 */
                          BMS_TASK_STACKSIZE, /* 任务栈大小 */
                          (void *)0,	/* 一块用户内存区的指针，用于任务控制块TCB的扩展功能
                                         （如任务切换时保存CPU浮点寄存器的数据）。一般不用，填0即可 */
                          OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); /* 任务选项字 */
    if(err != OS_ERR_NONE)
    {
        APP_TRACE("bms create task failed\r\n");
        return -PERR_ENOMEM;
    }
    else
    {
        OSTaskNameSet(BMS_TASK_PRIOR, "bms", &err);
        return 0;
    }
#endif
}

const battery_info_t *bms_get_battery_info(void)
{
    return &s_battery_info;
}

int bms_charge(int on)
{
    if(on)
    {
        APP_TRACE("bms start charge.\r\n");
        bsp_DoSet(DO_BATTERY_CHARGE, DO_BATTERY_CHARGE_ON);
    }
    else
    {
        APP_TRACE("bms stop charge.\r\n");
        bsp_DoSet(DO_BATTERY_CHARGE, DO_BATTERY_CHARGE_OFF);
    }

    query_battery_info_tmrcb(NULL, NULL);

    return 0;
}

static int can_msg_process(int usr, CanRxMsg* msg)
{
    uint16_t tmp;

    if( (msg->IDE != CAN_Id_Standard) || (msg->RTR != CAN_RTR_Data) 
        || (msg->StdId != 0x10) )
        return -PERR_ENOTSUP;

    //OSSchedLock();

    power_update_time = OSTimeGet();
    if(bms_error & BMSERR_POWER_UPDATE_OUTOFTM)
    {
        uint8_t err = BMS_COM_OUTOFTM;
        motionctrl_event_callback(MOTIONCTRL_EVENT_ERROR_RECOVERY, 1, &err);

        bms_error &= ~BMSERR_POWER_UPDATE_OUTOFTM;
    }

    tmp = ((uint16_t)msg->Data[0])<<8 | msg->Data[1];
    s_battery_info.voltage = tmp/10.0;

    tmp = ((uint16_t)msg->Data[2])<<8 | msg->Data[3];
    s_battery_info.current = tmp/10.0;

    s_battery_info.power = msg->Data[4];
    s_battery_info.temperature= msg->Data[5]-40;
    s_battery_info.state = msg->Data[6];

    //APP_TRACE("BMS: power= %d%% volt=%f\r\n", s_battery_info.power,
     //         s_battery_info.voltage);

    //OSSchedUnlock();
    return PERR_EOK;
}

void bms_query(void)
{
    query_battery_info_tmrcb(NULL, NULL);
}

/*
 *  Query the general infomation of battery
 */
static void query_battery_info_tmrcb(void* tmr, void*data)
{
    CanTxMsg txmsg;
    uint32_t tm, difftm;

    //APP_TRACE("BMS query. \r\n");
    if((bms_error & BMSERR_POWER_UPDATE_OUTOFTM) == 0)
    {
        tm = OSTimeGet();
        difftm = TIME_DIFFERENCE(tm, power_update_time);

        if(difftm > MS_TO_TICKS(30*1000))
        {
          uint8_t err = BMS_COM_OUTOFTM;

          motionctrl_event_callback(MOTIONCTRL_EVENT_ERROR, 1, &err);

          bms_error |= BMSERR_POWER_UPDATE_OUTOFTM;

          APP_TRACE("BMS time out.tm=%d oldtm=%d difftm = %d. last query time=%d\r\n",
                    tm, power_update_time, difftm, power_query_time);

          APP_TRACE("cpu usage: %2d%%\r\n", OSCPUUsage);
        }
    }

    txmsg.StdId = 0x16;
    txmsg.IDE = CAN_Id_Standard;
    txmsg.RTR = CAN_RTR_Data;

    txmsg.DLC = 8;

    txmsg.Data[0] = 0x16;
    txmsg.Data[1] = 0xBB;
    memset(&txmsg.Data[2], 0, 5);
    txmsg.Data[7] = 0x7E;

    can_dispatcher_send_msg(can_user, &txmsg);

    power_query_time = OSTimeGet();
}
