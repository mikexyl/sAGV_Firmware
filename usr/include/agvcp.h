#ifndef __AGVCP_H_20170315164235__
#define __AGVCP_H_20170315164235__

#include <stdint.h>

#include <agv.h>
#include "common/PosixError.h"

#define AGVCP_MAXMSG_SIZE       512
#define AGVCP_TASK_STACKSIZE    1024

typedef enum
{
    CONNSTATE_OFFLINE = 0,
    CONNSTATE_ONLINE,
    CONNSTATE_REGISTERING,
    CONNSTATE_UNREGISTERING,
    CONNSTATE_ERROR,
}conn_state_t;

typedef enum
{
    AGVCPEVT_ACTION_GOSTRAIGHT = 1,
    AGVCPEVT_ACTION_TRUNLEFT ,
    AGVCPEVT_ACTION_TRUNRIGHT ,

    AGVCPEVT_ACTION_STARTCHARGE = 6,
    AGVCPEVT_ACTION_ENDCHARGE,

    AGVCPEVT_ACTION_STOP ,
    AGVCPEVT_ACTION_UNLOAD,

    AGVCPEVT_CONNECTION_LOST=0x11,
    AGVCPEVT_CONNECTION_RECOVERY,

} agvcp_event_t;

typedef struct
{
    //int (*controller_cmd_callback)(uint32_t action, uint32_t paramlen, const uint8_t *params);
    int (*agvcp_event_callback)(agvcp_event_t event, uint32_t paramlen, const uint8_t *params);
    int (*get_status_callback)(agv_status_t *stat);

    int ((*read_param)(uint32_t id, float* val));
    int ((*write_param)(uint32_t id, float val));
}agvcp_cb_t;

int agvcp_init(uint32_t agv_name, const agvcp_cb_t *callback);

int agvcp_connect_controller(uint32_t host_ip, uint16_t host_port);

int agvcp_get_socket(void);

int agvcp_disconnect_controller(void);

conn_state_t agvcp_get_connenction_state(void);

int agvcp_notify_event(uint32_t evt, uint32_t len, const uint8_t *params);

#if 0
int notify_new_positoin(const agv_positon_t *pos);
int notify_action_over(uint32_t action);
#endif

#endif
