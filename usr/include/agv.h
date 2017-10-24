#ifndef __AGV_H_20170315165812__
#define __AGV_H_20170315165812__

#include <stdint.h>
#include "motion_ctrl.h"

//错误事件定义
#define NO_EVENT           		    	    0   // 没有事件
#define GS_COMM_BREAK_EVENT         	  1   // 导航传感器通讯断事件
#define OUT_OF_TRACK_EVENT          	  2   // 脱轨
#define SERVOL_INVALID_EVENT			      3   //左伺服失效事件
#define SERVOR_INVALID_EVENT			      4   //右伺服失效事件
#define CONTROL_RESUMABLE_STOP_EVENT 	  5   //网络通信断停车事件
#define WAY_POINT_ERROR                 6   //路线点校验错误
#define BMS_COM_OUTOFTM                 7
#define ENCODER_L_ERROR                 8
#define ENCODER_R_ERROR                 9

typedef enum
{
    AGV_WKSTATE_READY,
    AGV_WKSTATE_STOP,
    AGV_WKSTATE_MOVING,
    AGV_WKSTATE_CHARGING,
    AGV_WKSTATE_ERROR,
}work_state_t;

typedef struct
{
    motion_info_t  mtstate;

    uint32_t        battery;
    work_state_t    work_state;
    uint32_t        error;
}agv_status_t;

typedef enum
{
    AGVEVT_ACTION_START = MOTIONCTRL_EVENT_ACTION_START,
    AGVEVT_ACTION_OVER = MOTIONCTRL_EVENT_ACTION_OVER,

    AGVEVT_POINT_ARRIVAL = MOTIONCTRL_EVENT_POINT_ARRIVAL,

    AGVEVT_ERROR = MOTIONCTRL_EVENT_ERROR,
    AGVEVT_ERROR_RECOVERY = MOTIONCTRL_EVENT_ERROR_RECOVERY,
}avg_evt_t;


int agv_init(uint32_t agvid);

int agv_run(void);

void motionctrl_event_callback(motion_event_t evt, uint32_t len, const uint8_t *params);
#endif
