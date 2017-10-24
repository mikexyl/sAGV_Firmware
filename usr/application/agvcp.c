#include <ucos_ii.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "common/tools.h"
#include "app_cfg.h"
#include "agvcp.h"


#include "rsi_data_types.h"
#include "rsi_common_apis.h"
#include "rsi_wlan_apis.h"
#include "rsi_socket.h"
#include "rsi_error.h"


#define AGVCP_MAX_ERRCNT    5

#define MAX_ACTION_DATALEN  AGVCP_MAXMSG_SIZE
#define MAX_ACTION_NUM  4

typedef enum
{
    AGV_ATTITUDE_READY = 0,
    AGV_ATTITUDE_RUNING,
    AGV_ATTITUDE_OPERATION,
    AGV_ATTITUDE_SUSPEND,
}agvcp_attitude_t;

typedef enum
{
    AGV_MOTSTATE_STOPING =2,
    AGV_MOTSTATE_GOSTRAIGHT,
    AGV_MOTSTATE_TURN_LEFT,
    AGV_MOTSTATE_TURN_RIGHT ,
    AGV_MOTSTATE_UNLOADING,

    AGV_MOTSTATE_CHARGING,
}agvcp_motion_state_t;

typedef enum
{
    AGV_STATE_IDLE = 0,
    AGV_STATE_LOADING,
    AGV_STATE_CHARGING,
}agvcp_agv_state_t;

typedef enum
{
    AGVCP_ERR_EMRGE_STOP=1,

    AGVCP_ERR_GS_MISSING=6,
    AGVCP_ERR_OFFTRACK,
    AGVCP_ERR_SERVOL_FAULT,
    AGVCP_ERR_SERVOR_FAULT,

    AGVCP_ERR_ENCODER_L=13,
    AGVCP_ERR_ENCODER_R,

    AGVCP_ERR_WRONG_TAG=3004,

    AGVCP_ERR_BMS_COM=4004,
}agvcp_err_t;

typedef struct agvcp_conn
{
    uint32_t    agvid;
    int         socket;
    uint32_t    txseq;
    uint32_t    rxseq;

    conn_state_t conn_state;
    uint32_t    err_cnt;

    agvcp_cb_t agv_callback;

    int (*msg_callback)(struct agvcp_conn *connection, uint32_t msg_type, uint32_t seq,
                        uint8_t *buf, uint32_t len);

    OS_MEM      *action_mem;
    OS_EVENT    *action_que;

    OS_EVENT    *mutex;
}agvcp_conn_t;

typedef enum
{
    AGVCP_MSGTYPE_REGISTER = 1,
    AGVCP_MSGTYPE_UNREGISTER,

    AGVCP_MSGTYPE_HEARTBEAT=7,

    AGVCP_MSGTYPE_ERRREPORT=13,
    AGVCP_MSGTYPE_ERRRECOVERY,

    AGVCP_MSGTYPE_ACTION_START=9,
    AGVCP_MSGTYPE_ACTION_OVER=20,

    AGVCP_MSGTYPE_REPORT_FLAGPOINT=23,

    AGVCP_MSGTYPE_MSG_CONFIRM = 30,

    AGVCP_MSGTYPE_ACTION = 1010,

    AGVCP_MSGTYPE_READ_PARAM = 2000,
    AGVCP_MSGTYPE_WRITE_PARAM,
}   AGVCP_MSGTYPE_T;

static  agvcp_conn_t    conn;
static  OS_FLAG_GRP *connection_flag;

#define CONNECTION_FLAG_CONNECT         1<<0
#define CONNECTION_FLAG_DISCONNECT      1<<1

#define HEARTBEAT_CYCLE_MS              500

static OS_STK agvcp_task_stack[AGVCP_TASK_STACKSIZE];

static void on_connection_error(agvcp_conn_t *connection);
static void on_connection_recovery(agvcp_conn_t *connection);

static agvcp_attitude_t get_agvcp_attitude(work_state_t wkst, motion_state_t motst);
static agvcp_motion_state_t get_agvcp_motion_state(work_state_t wkst, motion_state_t motst);
static agvcp_agv_state_t get_agvcp_agv_state(work_state_t wkst, motion_state_t motst);


//=================porting====================================
#define BUFFER_PACKET_NUM       4
typedef struct
{
    uint32_t    len;
    uint8_t     data[1];
} msg_t;

static uint8_t  rxdata_mempool[BUFFER_PACKET_NUM][AGVCP_MAXMSG_SIZE+4];
static OS_MEM   *rxmem;

static void     *msg_mem[BUFFER_PACKET_NUM];
static OS_EVENT *rxq;

static void sock_async_rxcallback(uint32_t sock, const struct sockaddr* addr, uint8_t *data, uint32_t len)
{
    msg_t* msg;
    uint8_t err;

    if(len > AGVCP_MAXMSG_SIZE)
    {
        APP_TRACE("agvcp msg too long.\r\n");
        return ;
    }

    msg = OSMemGet(rxmem, &err);
    if(msg == NULL)
    {
        APP_TRACE("agvcp rxmsg no mem.\r\n");
        return ;
    }

    msg->len = len;
    memcpy(msg->data, data, len);

    err = OSQPost(rxq, msg);
    if(err != OS_ERR_NONE)
    {
        APP_TRACE("AGVCP Error: post rxdata failed.\r\n");
        OSMemPut(rxmem, msg);
        return ;
    }
}

static int create_udp_conn(uint32_t ip, uint16_t port, uint16_t local_port)
{
    int     ret;
    uint8_t err;
    int32_t sock;

    APP_TRACE("AGVCP: connect to %u.%d\r\n", ip, port);

    rxq = OSQCreate(msg_mem, BUFFER_PACKET_NUM);
    if(rxq== NULL)
    {
        APP_TRACE("agvcp create rxq failed.\r\n");
        ret = -PERR_ENOMEM;

        goto ERR_1;
    }

    rxmem = OSMemCreate(rxdata_mempool, BUFFER_PACKET_NUM,
                AGVCP_MAXMSG_SIZE+4, &err);
    if(rxmem == NULL)
    {
        APP_TRACE("agvcp create rxmem failed. err = %d\r\n", err);
        ret = -PERR_ENOMEM;

        goto ERR_2;
    }

    sock = socket_async(AF_INET, SOCK_DGRAM, 0, sock_async_rxcallback);
    ret = sock;
    if(ret < 0)
        goto ERR_3;

    struct      sockaddr_in addr;// client_addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family= AF_INET;
    addr.sin_port = htons(local_port);
    ret = bind(sock, (struct sockaddr *) &addr, sizeof(addr));
    if(ret != RSI_SUCCESS)
    {
        ret = -1;
        APP_TRACE("agvcp socket bind failed\r\n");
        goto ERR_4;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ip;
    addr.sin_port = htons(port);

    ret = connect(sock, (struct sockaddr *) &addr, sizeof(addr));
    if(ret != RSI_SUCCESS)
    {
        ret = -1;
        APP_TRACE("agvcp socket connect failed\r\n");
        goto ERR_4;
    }

    ret = sock;
    goto EXIT;

ERR_4:
    shutdown(sock, 0);
ERR_3:
    rxmem = 0;
ERR_2:
    OSQDel(rxq, OS_DEL_NO_PEND, &err);
    rxq = 0;

ERR_1:
EXIT:
    return ret;
}

static int destroy_udp_conn(int sock)
{
    uint8_t err;

    shutdown(sock, 0);

    rxmem = 0;

    OSQDel(rxq, OS_DEL_NO_PEND, &err);
    rxq = 0;

    return 0;
}

static int sock_read(int sock, uint8_t *buf, uint32_t size)
{
    uint8_t err;
    msg_t   *msg;
    int     len;

    msg = OSQAccept(rxq, &err);
    if(msg == NULL)
        return 0;

    if(size < msg->len)
    {
        APP_TRACE("sock read buffer size is not enough 1 %d bytes msg will be droped.\r\n", 
                  msg->len);
        OSMemPut(rxmem, msg);
        return -PERR_ENOSPC;
    }

    len = msg->len;
    memcpy(buf, msg->data, len);
#if 1
    APP_TRACE("AGVCP Recv:");
    memdump(buf, len);
#endif
    OSMemPut(rxmem, msg);

    return len;
}

static int sock_write(int sock, const uint8_t *buf, uint32_t len)
{
    int ret;
#if 1
    if( ((uint32_t)buf[0]<<24 | (uint32_t)buf[1]<<16 |
        (uint32_t)buf[2]<<8 | (uint32_t)buf[3]) != AGVCP_MSGTYPE_HEARTBEAT)
    {
      APP_TRACE("AGVCP Send:");
      memdump(buf, len);
    }
#endif
    ret = send(sock, (int8_t*)buf, len, 0);
    if(ret != len)
      APP_TRACE("AGVCP send failed with %d.\r\n", ret);

    return ret;
}

//=========================================================================


static int agv_rx_msg(const agvcp_conn_t *connection, uint32_t *mtype, uint32_t *seq, uint8_t *data, uint32_t size)
{
    uint8_t rxbuf[AGVCP_MAXMSG_SIZE];
    int len;
    int count = 0;

    //我们使用串口wifi模块传输UDP数据包
    //因为应用层协议本身没有设置包头包尾。
    //此处需要通过间隔时间来确保一个UDP数据包被完整接收
#if 0
    while(count<size)
    {
        len = sock_read(connection->socket, rxbuf, AGVCP_MAXMSG_SIZE);
        if(len <= 0)
            break;
        if(len > 0)
        {
            count += len;
            OSTimeDlyHMSM(0, 0, 0, 5);
        }
    }
#else
    len = sock_read(connection->socket, rxbuf, AGVCP_MAXMSG_SIZE);
    if(len > 0)
      count=len;
#endif

    if(count < 8)
        return -PERR_ETIMEDOUT;

    NTOHL_COPY(&rxbuf[0], mtype);
    NTOHL_COPY(&rxbuf[4], seq);

    if(count-8 > size)
        len = size;
    else
        len = count-8;

    memcpy(data, &rxbuf[8], len);

    return len;
}

static int agv_tx_msg(const agvcp_conn_t *connection, uint32_t msg_type, uint32_t seq, const uint8_t *msg, uint32_t len)
{
    uint8_t txbuf[AGVCP_MAXMSG_SIZE];

    if((len + 8) >= AGVCP_MAXMSG_SIZE)
        return -PERR_ENOMEM;

    HTONL_COPY(&msg_type, &txbuf[0]);
    HTONL_COPY(&seq, &txbuf[4]);

    memcpy(&txbuf[8], msg, len);

    sock_write(connection->socket, txbuf, len+8);

    return 0;
}

static int  wait_ack(agvcp_conn_t *connection, uint32_t msg_type, uint32_t seq, uint32_t tmout_ms)
{
    int ret;
    int err = -PERR_ETIMEDOUT;
    uint8_t rxbuf[AGVCP_MAXMSG_SIZE];
    uint32_t tmcount = 0;
    uint32_t rxmtype, rxseq;
    uint32_t    id;

    while(tmcount < tmout_ms)
    {
        OSTimeDlyHMSM(0, 0, 0, 10);
        tmcount += 10;

        ret = agv_rx_msg(connection, &rxmtype, &rxseq, rxbuf, sizeof(rxbuf));
        if(ret >= 0)
        {
            if((rxmtype == msg_type) && (rxseq == seq) )
            {
                err = PERR_EOK;
                break;
            }
            else if(ret > 4)   //不是ACK，是一个消息祯来处理
            {
                APP_TRACE("Msg(%d) recved in waitack\r\n", seq);

                NTOHL_COPY(rxbuf, &id);
                if(id == connection->agvid)
                {
                    /*
                     * Not the expecting ACK, other message also 
                     * need to be properly processd here
                     * */
                    if (connection->msg_callback != NULL)
                    {
                        ret = connection->msg_callback(connection, rxmtype, rxseq, &rxbuf[4], ret-4);
                    }
                }
                else
                    APP_TRACE("Wrong AGV-name .\r\n.");
            }
            else    //既非当前的ACK,又非一个正常的消息祯，直接丢弃
            {
                APP_TRACE("Bad msg in waitack.\r\n.");
            }
        }
    }

    return err;
}

#define WAIT_ACK_TIMEOUT   1000

static int agv_send_msg_locked(agvcp_conn_t *connection, uint32_t msg_type,
                                   const uint8_t *msg, uint32_t len, int retry_times)
{
    uint32_t    seq;
    int         ret = -PERR_ETIMEDOUT;
    int         infinite=0;

    if(retry_times == 0)
        infinite = 1;

    //APP_TRACE("AGVCP send msg(%d)\r\n", msg_type);

    seq = connection->txseq;

RETRY:
    agv_tx_msg(connection, msg_type, seq, msg, len);
    //no ack for heartbeat
    if(msg_type == AGVCP_MSGTYPE_HEARTBEAT)
    {
        connection->txseq++;
        ret = PERR_EOK;
        goto EXIT;
    }

    ret = wait_ack(connection, msg_type, seq, WAIT_ACK_TIMEOUT);
    if(ret == PERR_EOK)
      connection->txseq++;
    else if(infinite)
      goto RETRY;
    else if(--retry_times > 0)
      goto RETRY;

    if(ret != PERR_EOK)
    {
        if( (++connection->err_cnt > AGVCP_MAX_ERRCNT) &&
            (connection->conn_state == CONNSTATE_ONLINE) )
        {
            on_connection_error(connection);
            connection->conn_state = CONNSTATE_ERROR;
        }
    }
    else if (connection->conn_state == CONNSTATE_ERROR)
    {
        on_connection_recovery(connection);
        connection->conn_state = CONNSTATE_ONLINE;
    }

EXIT:
    return (ret == PERR_EOK)? PERR_EOK:-PERR_ETIMEDOUT;
}

static int agv_trans_msg(agvcp_conn_t *connection, uint32_t msg_type,
                                   const uint8_t *msg, uint32_t len, uint8_t *response, int retry_times)
{
    int ret;
    uint8_t err;

    OSMutexPend(connection->mutex, 0, &err);
    ret = agv_send_msg_locked(connection, msg_type, msg, len, retry_times);
    OSMutexPost(connection->mutex);

    return ret;
}

static int agv_send_msg(agvcp_conn_t *connection, uint32_t msg_type,
                                   const uint8_t *msg, uint32_t len, int retry_times)
{
    int ret;
    uint8_t err;

    OSMutexPend(connection->mutex, 0, &err);
    ret = agv_send_msg_locked(connection, msg_type, msg, len, retry_times);
    OSMutexPost(connection->mutex);

    return ret;
}

//////////////////////////////////////
static int agv_register_device(agvcp_conn_t *connection)
{
    uint8_t txbuf[16];
    uint32_t    temp32;
    agv_status_t stat;

    connection->agv_callback.get_status_callback(&stat);

    HTONL_COPY(&connection->agvid, &txbuf[0]);
    HTONL_COPY(&stat.mtstate.position.point, &txbuf[4]);
    HTONL_COPY(&stat.mtstate.direction.direction, &txbuf[8]);

    temp32 = (uint32_t)get_agvcp_agv_state(stat.work_state, stat.mtstate.state);
    HTONL_COPY(&temp32, &txbuf[12]);

    return agv_send_msg(connection, AGVCP_MSGTYPE_REGISTER, txbuf, 16, 1);
}

static int agv_unregister_device(agvcp_conn_t *connection)
{
    uint8_t txbuf[16];
    uint32_t    temp32;
    agv_status_t stat;

    connection->agv_callback.get_status_callback(&stat);

    HTONL_COPY(&connection->agvid, &txbuf[0]);
    HTONL_COPY(&stat.mtstate.position.point, &txbuf[4]);
    temp32 = (uint32_t)get_agvcp_agv_state(stat.work_state, stat.mtstate.state);
    HTONL_COPY(&temp32, &txbuf[8]);

    return agv_send_msg(connection, AGVCP_MSGTYPE_UNREGISTER, txbuf, 12, 1);
}

static agvcp_attitude_t get_agvcp_attitude(work_state_t wkst, motion_state_t motst)
{
    if(wkst == AGV_WKSTATE_ERROR)
        return AGV_ATTITUDE_SUSPEND;
    else if(wkst == AGV_WKSTATE_CHARGING)
        return AGV_ATTITUDE_OPERATION;
    else if(wkst == AGV_WKSTATE_MOVING)
        return AGV_ATTITUDE_RUNING;
    else
        return AGV_ATTITUDE_READY;
}

static agvcp_motion_state_t get_agvcp_motion_state(work_state_t wkst, motion_state_t motst)
{
    if(wkst == AGV_WKSTATE_CHARGING)
      return AGV_MOTSTATE_CHARGING;
    else if(motst == MOTIONSTATE_GOSTRAIGHT)
      return AGV_MOTSTATE_GOSTRAIGHT;
    else if(motst == MOTIONSTATE_TRUNLEFT)
      return AGV_MOTSTATE_TURN_LEFT;
    else if(motst == MOTIONSTATE_TRUNRIGHT)
      return AGV_MOTSTATE_TURN_RIGHT;
    else if(motst == MOTIONSTATE_UNLOADING)
      return AGV_MOTSTATE_UNLOADING;
    else
      return AGV_MOTSTATE_STOPING;
}

static agvcp_agv_state_t get_agvcp_agv_state(work_state_t wkst, motion_state_t motst)
{
    if(wkst == AGV_WKSTATE_CHARGING)
      return AGV_STATE_CHARGING;
    else
      return AGV_STATE_IDLE;
}

static uint32_t get_agvcp_err(uint8_t agv_err)
{
    uint32_t agvcp_err;

    switch(agv_err)
    {
        case GS_COMM_BREAK_EVENT:
            agvcp_err = AGVCP_ERR_GS_MISSING;
            break;

        case OUT_OF_TRACK_EVENT:
            agvcp_err = AGVCP_ERR_OFFTRACK;
            break;

        case WAY_POINT_ERROR:
            agvcp_err = AGVCP_ERR_WRONG_TAG;
            break;

        case SERVOL_INVALID_EVENT:
            agvcp_err = AGVCP_ERR_SERVOL_FAULT;
            break;

        case SERVOR_INVALID_EVENT:
            agvcp_err = AGVCP_ERR_SERVOR_FAULT;
            break;

        case BMS_COM_OUTOFTM:
            agvcp_err = AGVCP_ERR_BMS_COM;
            break;

        case ENCODER_L_ERROR:
            agvcp_err = AGVCP_ERR_ENCODER_L;
            break;
        case ENCODER_R_ERROR:
            agvcp_err = AGVCP_ERR_ENCODER_R;
            break;
        default:
            agvcp_err = 0;
            break;
    }

    return agvcp_err;
}

static int agv_send_heartbeat(agvcp_conn_t *connection)
{
    uint8_t     txbuf[64];
    uint32_t    len=0;
    agv_status_t stat;
    uint32_t    temp32;

    connection->agv_callback.get_status_callback(&stat);

    HTONL_COPY(&connection->agvid, &txbuf[len]);
    len+=4;
    HTONL_COPY(&stat.battery, &txbuf[len]);
    len+=4;
    HTONL_COPY(&stat.mtstate.speed, &txbuf[len]);
    len+=4;
    HTONL_COPY(&stat.mtstate.direction.direction, &txbuf[len]);
    len+=4;
    HTONL_COPY(&stat.mtstate.position.point, &txbuf[len]);
    len+=4;

    temp32 = (uint32_t)get_agvcp_attitude(stat.work_state, stat.mtstate.state);
    HTONL_COPY(&temp32, &txbuf[len]);
    len+=4;
    HTONL_COPY(&stat.mtstate.position.x_deviation, &txbuf[len]);
    len+=4;
    HTONL_COPY(&stat.mtstate.position.y_deviation, &txbuf[len]);
    len+=4;
    HTONL_COPY(&stat.mtstate.direction.deviation, &txbuf[len]);
    len+=4;

    temp32 = (uint32_t)get_agvcp_motion_state(stat.work_state, stat.mtstate.state);
    HTONL_COPY(&temp32, &txbuf[len]);
    len+=4;

    return agv_send_msg(connection, AGVCP_MSGTYPE_HEARTBEAT, txbuf, len, 1);
}

static int agv_send_naviinfo(agvcp_conn_t *connection, uint32_t position, uint32_t dir)
{
    uint8_t txbuf[16];
    //agv_status_t stat;
    //connection->agv_callback.get_status_callback(&stat);

    APP_TRACE("Send naviinfo at: %d dir=%d\r\n", position, dir);

    HTONL_COPY(&connection->agvid, &txbuf[0]);
    HTONL_COPY(&position, &txbuf[4]);
    HTONL_COPY(&dir, &txbuf[8]);

    return agv_send_msg(connection, AGVCP_MSGTYPE_REPORT_FLAGPOINT, txbuf, 12, 0);
}

static int agv_send_action_over(agvcp_conn_t *connection, uint32_t action)
{
    uint8_t txbuf[16];
    agv_status_t stat;

    APP_TRACE("Send action over. action = %d\r\n", action);
    connection->agv_callback.get_status_callback(&stat);

    HTONL_COPY(&connection->agvid, &txbuf[0]);
    HTONL_COPY(&action, &txbuf[4]);
    HTONL_COPY(&stat.mtstate.position.point, &txbuf[8]);
    HTONL_COPY(&stat.mtstate.direction.direction, &txbuf[12]);

    return agv_send_msg(connection, AGVCP_MSGTYPE_ACTION_OVER, txbuf, 16, 0);
}

static int agv_send_error(agvcp_conn_t *connection, uint32_t err)
{
    uint8_t     txbuf[64];
    uint32_t    len=0;
    agv_status_t stat;
    uint32_t agvcp_err;

    connection->agv_callback.get_status_callback(&stat);

    HTONL_COPY(&connection->agvid, &txbuf[len]);
    len+=4;
    HTONL_COPY(&stat.mtstate.position.point, &txbuf[len]);
    len+=4;
    agvcp_err = get_agvcp_err(err);
    HTONL_COPY(&agvcp_err, &txbuf[len]);
    len+=4;

    APP_TRACE("AGVCP report error %d\r\n", agvcp_err);
    return agv_send_msg(connection, AGVCP_MSGTYPE_ERRREPORT, txbuf, len, 0);
}

static int agv_confirm_msg(agvcp_conn_t *connection, AGVCP_MSGTYPE_T mtype, uint32_t seq)
{
    uint8_t txbuf[16];

//  APP_TRACE("confirm msg %d\r\n", seq);
#if 1
    HTONL_COPY(&connection->agvid, &txbuf[0]);
    HTONL_COPY(&mtype,  &txbuf[4]);
    HTONL_COPY(&seq, &txbuf[8]);
#else
    HTONL_COPY(&mtype,  &txbuf[0]);
    HTONL_COPY(&seq, &txbuf[4]);
    HTONL_COPY(&connection->agvid, &txbuf[8]);
#endif

    return agv_send_msg(connection, AGVCP_MSGTYPE_MSG_CONFIRM, txbuf, 12, 0);
}

#if 0
static int agv_confirm_lastmsg(agvcp_conn_t *connection, AGVCP_MSGTYPE_T mtype)
{
    return agv_confirm_msg(connection, mtype, connection->rxseq);
}
#endif

//===================================================
static void on_connection_error(agvcp_conn_t *connection)
{
    APP_TRACE("AGVCP: connection error.\r\n");
    connection->agv_callback.agvcp_event_callback(AGVCPEVT_CONNECTION_LOST, 0, NULL);
}

static void on_connection_recovery(agvcp_conn_t *connection)
{
    APP_TRACE("AGVCP: connection recovery.\r\n");
    connection->agv_callback.agvcp_event_callback(AGVCPEVT_CONNECTION_RECOVERY, 0, NULL);
}

static int buffer_msg(agvcp_conn_t *connection, uint32_t msg_type, uint32_t seq, 
                             uint8_t *params, uint32_t len)
{
    uint8_t err;
    uint8_t *data;
    uint8_t ret;

    //APP_TRACE("buffer msg[%d]\r\n", seq);
    data = OSMemGet(connection->action_mem, &err);
    if(data == NULL)
    {
        return -PERR_ENOMEM;
    }

    if(len+16 > MAX_ACTION_DATALEN)
    {
        OSMemPut(connection->action_mem, data);
        return -PERR_ENOSPC;
    }

    memcpy(&data[0], &msg_type, 4);
    memcpy(&data[4], &seq, 4);
    memcpy(&data[8], &len, 4);
    memcpy(&data[12], params, len);

    ret = OSQPost(connection->action_que, data);
    //APP_TRACE("actoin post: %lu \r\n", (uint32_t)data);
    if(ret != OS_ERR_NONE)
    {
        APP_TRACE("AGVCP Error: enque action failed.\r\n");
        OSMemPut(connection->action_mem, data);
        return -PERR_ENOMEM;
    }

    return 0;
}

static int process_buffered_msg(agvcp_conn_t *connection)
{
    uint32_t    mtype, seq;
    uint32_t    action, paraml;
    const uint8_t    *params;
    uint8_t *data;
    uint8_t err;

    do{
        data = OSQAccept(connection->action_que, &err);
        if(data != NULL)
        {
            memcpy(&mtype,  &data[0], 4);
            memcpy(&seq,    &data[4], 4);
            if(mtype != AGVCP_MSGTYPE_ACTION)
            {
                APP_TRACE("AGVCP Error: buffered a msg(%d) not action.\r\n", mtype);
                goto FREEMEM;
            }

            NTOHL_COPY(&data[12], &action);
            NTOHL_COPY(&data[16], &paraml);
            params = &data[20];

            /*
             *  对于动作命令，需要一个恢复的命令向控制台发送命令确认，
             *  控制台回复确认后再执行
             * */
            agv_confirm_msg(connection, (AGVCP_MSGTYPE_T)mtype, seq);

            connection->agv_callback.agvcp_event_callback((agvcp_event_t)action, paraml, params);
FREEMEM:
            OSMemPut(connection->action_mem, data);
        }
    }while(data);

    return 0;
}

static int agvcp_msg_process(agvcp_conn_t *connection, uint32_t msg_type, uint32_t seq, 
                             uint8_t *buf, uint32_t len)
{
    uint32_t    tmp=0;
    uint32_t    paramlen;
    uint8_t     txmsg[64];
    int         ret;
    uint32_t    id;
    int         val_int;
    float       val_f;

    APP_TRACE("AGVCP process msg: %d.%d.\r\n", msg_type, seq);
    switch(msg_type)
    {
        case AGVCP_MSGTYPE_ACTION:
            NTOHL_COPY(&buf[0], &tmp);      //action
            NTOHL_COPY(&buf[4], &paramlen); //param-len
            if( (paramlen + 8) != len )
            {
                APP_TRACE("AGVCP Error: frame legth\r\n");
                return -PERR_EBADMSG;
            }
            HTONL_COPY(&connection->agvid, &txmsg[0]);

            //重复命令，回复但不重复处理
            if((seq == connection->rxseq) && (seq != 0) && (connection->rxseq != 0))
            {
                agv_tx_msg(connection, msg_type, seq, txmsg, 4);
                ret = -PERR_EEXIST;
            }
            else
            {
                //put action into a queue, and process it asynchronously
                ret = buffer_msg(connection, msg_type, seq, buf, len);
                if(ret == 0)
                {
                    ret = PERR_EOK;
                    agv_tx_msg(connection, msg_type, seq, txmsg, 4);
                }
                else    //buffer失败的情况下，不做回复，等待重发
                {
                    APP_TRACE("AGVCP Error: buffer action msg failed.\r\n");
                    ret = -PERR_EAGAIN;
                }
            }
            break;

        case AGVCP_MSGTYPE_WRITE_PARAM:
            HTONL_COPY(&connection->agvid, &txmsg[0]);
            memcpy(&txmsg[4], &buf[0], 8);  //id & value

            agv_tx_msg(connection, msg_type, seq, txmsg, 12);

            if((seq == connection->rxseq) && (seq != 0) && (connection->rxseq != 0))
                ret = -PERR_EEXIST;
            else
            {
                NTOHL_COPY(&buf[0], &id);
                NTOHL_COPY(&buf[4], &val_int);
                if(connection->agv_callback.write_param)
                    connection->agv_callback.write_param(id, val_int/1000.0);
                ret = PERR_EOK;
            }
            break;

        case AGVCP_MSGTYPE_READ_PARAM:
            NTOHL_COPY(&buf[0], &id);
            val_f = 0;
            if(connection->agv_callback.read_param)
                ret = connection->agv_callback.read_param(id, &val_f);

            HTONL_COPY(&connection->agvid, &txmsg[0]);
            memcpy(&txmsg[4], &buf[0], 4);  //id

            val_int = val_f*1000.0;
            HTONL_COPY(&val_int, &txmsg[8]);    //value
            agv_tx_msg(connection, msg_type, seq, txmsg, 12);
            ret = PERR_EOK;
            break;

        default:
            ret = -PERR_ENOTSUP;
            break;
    }

    if(ret == PERR_EOK)
        connection->rxseq = seq;

    return ret;
}

/*
 *TODO: 对于bufferd msg，有一个隐患，
 * 可能导致控制台后发的命令先被处理，这样在有命令缓冲的情况下
 * 是很危险的。
 * */
static void agvcp_connectoin_process(agvcp_conn_t *connection)
{
    uint8_t     data[256];
    uint32_t    msgtype ;
    uint32_t    seq;
    int         len;
    uint8_t     err;
    int         ret;
    static uint32_t    heartbeat_tm = 0;
    uint32_t    tm, difftm;
    uint32_t    id;

    while(true)
    {
        tm = OSTimeGet();
        difftm = TIME_DIFFERENCE(tm, heartbeat_tm);
        if(difftm >= MS_TO_TICKS(HEARTBEAT_CYCLE_MS))
        {
            heartbeat_tm = tm;
            agv_send_heartbeat(connection);
        }

        OSMutexPend(connection->mutex, 0, &err);
        len = agv_rx_msg(connection, &msgtype, &seq, data, sizeof(data));
        if(len >= 0)
        {
            APP_TRACE("Recv msg : %d\r\n", msgtype);

            NTOHL_COPY(&data[0], &id);      //agvname
            if(id!= connection->agvid)
                APP_TRACE("wrong agv-name\r\n");
            else
            {
                ret = agvcp_msg_process(connection, msgtype, seq, &data[4], len-4);
                if( (ret == PERR_EOK) && (connection->conn_state == CONNSTATE_ERROR) )
                {
                    on_connection_recovery(connection);
                    connection->conn_state = CONNSTATE_ONLINE;
                }
            }
        }
        OSMutexPost(connection->mutex);

        process_buffered_msg(connection);

        if(OSFlagPend(connection_flag, CONNECTION_FLAG_DISCONNECT,
                      OS_FLAG_WAIT_SET_ALL|OS_FLAG_CONSUME,
                      20, &err) == CONNECTION_FLAG_DISCONNECT)
        {
            break;
        }
    }
}

static void agvcp_task(void *params)
{
    uint8_t err;
    int     ret;

    APP_TRACE("Initialize UDP connection...\r\n");
    conn.socket = create_udp_conn(controller_ip, controller_port, local_port);
    if(conn.socket < 0)
    {
        APP_TRACE("AGVCP: Create connection failed.\r\n");
        return ;
    }

    while(true)
    {
        //等待连接命令
        OSFlagPend(connection_flag, CONNECTION_FLAG_CONNECT,
                   OS_FLAG_WAIT_SET_ALL|OS_FLAG_CONSUME, 0, &err);

        conn.conn_state = CONNSTATE_REGISTERING;
        conn.err_cnt = 0;

        ret = agv_register_device(&conn);
        while(ret != PERR_EOK)
        {
            OSTimeDlyHMSM(0, 0, 0, 500);
            ret = agv_register_device(&conn);
        }
        APP_TRACE("AGVCP register ok.\r\n");

        conn.conn_state = CONNSTATE_ONLINE;
        agvcp_connectoin_process(&conn);

        conn.conn_state = CONNSTATE_UNREGISTERING;
        do{ ret = agv_unregister_device(&conn); }  while(ret != PERR_EOK);
        conn.conn_state = CONNSTATE_OFFLINE;
    }
}

static uint8_t actionmem_pool[MAX_ACTION_NUM][MAX_ACTION_DATALEN] = {0};
static void* actionq_mem[MAX_ACTION_NUM];

int agvcp_init(uint32_t agv_name, const agvcp_cb_t *callback)
{
    uint8_t err;

    conn.agvid = agv_name;

    conn.agv_callback.agvcp_event_callback = callback->agvcp_event_callback;
    conn.agv_callback.get_status_callback = callback->get_status_callback;
    conn.agv_callback.read_param = callback->read_param;
    conn.agv_callback.write_param = callback->write_param;

    conn.txseq = 0;
    conn.rxseq = ~0;
    conn.conn_state = CONNSTATE_OFFLINE;
    conn.msg_callback = agvcp_msg_process;

    conn.mutex = OSMutexCreate(5, &err);
    if(conn.mutex == NULL)
    {
        APP_TRACE("Agvcp create mutex failed with err = %d\r\n", err);
        return -1;
    }

    APP_TRACE("actionmem_pool at: %lu \r\n", (uint32_t)actionmem_pool);
    conn.action_mem = OSMemCreate(actionmem_pool, MAX_ACTION_NUM,
                MAX_ACTION_DATALEN, &err);
    if(conn.action_mem == NULL)
    {
        APP_TRACE("AGVCP Error: create mempool failed. error=%d\r\n", err);
        return -PERR_ENOMEM;
    }

    conn.action_que = OSQCreate(actionq_mem, MAX_ACTION_NUM);
    if(conn.action_que== NULL)
    {
        APP_TRACE("AGVCP Error: create queue failed. error = %d\r\n", err);
        return -PERR_ENOMEM;
    }

    OSTaskCreateExt(agvcp_task,	/* 启动任务函数指针 */
                    (void *)0,		/* 传递给任务的参数 */
                    (OS_STK *)&agvcp_task_stack[AGVCP_TASK_STACKSIZE - 1], /* 指向任务栈栈顶的指针 */
                    AGVCP_TASK_PRIOR,	/* 任务的优先级，必须唯一，数字越低优先级越高 */
                    AGVCP_TASK_PRIOR,	/* 任务ID，一般和任务优先级相同 */
                    (OS_STK *)&agvcp_task_stack[0],/* 指向任务栈栈底的指针。OS_STK_GROWTH 决定堆栈增长方向 */
                    AGVCP_TASK_STACKSIZE, /* 任务栈大小 */
                    (void *)0,	/* 一块用户内存区的指针，用于任务控制块TCB的扩展功能
                       （如任务切换时保存CPU浮点寄存器的数据）。一般不用，填0即可 */
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); /* 任务选项字 */

    OSTaskNameSet(AGVCP_TASK_PRIOR, "agvcp", &err);

    connection_flag = OSFlagCreate(0, &err);

    return 0;
}

int agvcp_connect_controller(uint32_t host_ip, uint16_t host_port)
{
    uint8_t err;

    //Signal the connection task to run.
    OSFlagPost(connection_flag, CONNECTION_FLAG_CONNECT, OS_FLAG_SET, &err);

    return PERR_EOK;
}

int agvcp_disconnect_controller(void)
{
    uint8_t err;

    //Signal the connection task to stop.
    OSFlagPost(connection_flag, CONNECTION_FLAG_DISCONNECT, OS_FLAG_SET, &err);

    //wait the connection task to get this signal
    OSFlagPend(connection_flag, CONNECTION_FLAG_DISCONNECT, OS_FLAG_WAIT_CLR_ALL, 0, &err);

    while(conn.conn_state != CONNSTATE_OFFLINE)
        OSTimeDlyHMSM(0, 0 , 0, 10);

    destroy_udp_conn(conn.socket);
    return 0;
}

int agvcp_get_socket(void)
{
    return conn.socket;
}

conn_state_t agvcp_get_connenction_state(void)
{
    return conn.conn_state;
}

int agvcp_notify_event(uint32_t evt, uint32_t len, const uint8_t *params)
{
    int ret;
    uint8_t err;

    if( (conn.conn_state != CONNSTATE_ONLINE) &&
    (conn.conn_state != CONNSTATE_ERROR) )
        return 0;

    switch(evt)
    {
        case AGVEVT_ACTION_OVER:
            ret = agv_send_action_over(&conn, params[0]);
            break;

        case AGVEVT_POINT_ARRIVAL:
            uint32_t tag;
            uint32_t dir;

            NTOHL_COPY(&params[0], &tag);
            NTOHL_COPY(&params[4], &dir);

            ret = agv_send_naviinfo(&conn, tag, dir);
            break;

        case AGVEVT_ERROR:
            err = params[0];
            ret = agv_send_error(&conn, err);
            break;

        case AGVEVT_ERROR_RECOVERY:
            //ret = agv_send_error_recovery(&conn);
            break;

        default:
            ret = 0;
            break;
    }

    return ret;
}
