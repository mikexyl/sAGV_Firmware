#ifndef __CAN_DISPATCHER_H_20170323113223__
#define __CAN_DISPATCHER_H_20170323113223__

#include "bsp_can.h"

#if 0
typedef struct
{
    uint8_t  ide;
    uint8_t  rtr;

    uint32_t id;
    uint32_t dlc;
    uint8_t  data[8];
} can_frame_t;
#endif

#define CANDISPATCHER_MAX_USERNUM   3

//return value must be among
//  EOK
//  -ENOTSUP
//  -EBADMSG
typedef int (*msg_processor_t)(int user, CanRxMsg *msg);

int can_dispatcher_init(CAN_TypeDef *canport, uint16_t baudrate_k);

int can_dispatcher_register_user(const char*name, msg_processor_t proc);

int can_dispatcher_run(void);

int can_dispatcher_send_msg(int user, CanTxMsg*msg);

void can_print_frame(CanRxMsg *rxmsg);

#endif
