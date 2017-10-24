#include <bsp.h>
#include "bsp_can.h"
#include "algorithm.h"
#include "stm32f4xx_can.h"
#include "can_dispatcher.h"
#include "app_cfg.h"
#include "motion_ctrl.h"
#include "motors.h"
#include "PosixError.h"
#include "encoder.h"

#define CANSEND_TASK_STACKSIZE  256
static OS_STK cansend_task_stack[CANSEND_TASK_STACKSIZE];
#define CANSEND_TASK_PRIOR 20

#define ALGO_TASK_STACKSIZE  256
static OS_STK algo_task_stack[ALGO_TASK_STACKSIZE];
#define ALGO_TASK_PRIOR 21

#define ENC_TASK_STACKSIZE  256
static OS_STK enc_task_stack[ENC_TASK_STACKSIZE];
#define ENC_TASK_PRIOR 30

#define SAGV_CANID 0x07
#define ENCODER_CANID 0x110
#define PC_CANID 0x09

#define AGV_CMD_TYPE_VEL 0x220
#define AGV_CMD_TYPE_ENC 0x230

static int  can_msg_process(int usr, CanRxMsg* msg);
static void canSendTask(void *param);
static int can_user;
static void sAGV_algo_run(void* param);
static void send_encoder_task(void* param);
static void send_encoder_unpolling(void);
static void encoder_tmr_callback(void *tmr, void*data);

int encoder_period=10;

typedef struct
{
    int32_t vel_l;
    int32_t vel_r;
}vel_data_t;
vel_data_t vel_data_algo;
vel_data_t vel_data_real;

typedef struct
{
	int32_t encoder_l;
	int32_t encoder_r;
}encoder_data_t;
encoder_data_t encoder_data;

uint8_t err;

int canSendTest(void)
{
	encoder_l = open_encoder(1);
  encoder_r = open_encoder(2);
	
  OSTimeDly(MS_TO_TICKS(1000));
	
	get_count_and_clear(encoder_l);
	get_count_and_clear(encoder_r);
	
	uint8_t err;

	can_user=can_dispatcher_register_user("pc",can_msg_process);
	
	err = OSTaskCreateExt(canSendTask,	/* ???????? */
                      (void *)0,		/* ???????? */
                      (OS_STK *)&cansend_task_stack[CANSEND_TASK_STACKSIZE - 1], /* ?????????? */
                      CANSEND_TASK_PRIOR        ,	/* ??????,????,????????? */
                      CANSEND_TASK_PRIOR        ,	/* ??ID,?????????? */
                      (OS_STK *)&cansend_task_stack[0],/* ???????????OS_STK_GROWTH ???????? */
                      CANSEND_TASK_STACKSIZE, /* ????? */
                      (void *)0,	/* ??????????,???????TCB?????
                                     (????????CPU????????)?????,?0?? */
                      OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); /* ????? */
											
 	err = OSTaskCreateExt(sAGV_algo_run,	/* ???????? */
                      (void *)0,		/* ???????? */
                      (OS_STK *)&algo_task_stack[ALGO_TASK_STACKSIZE - 1], /* ?????????? */
                      ALGO_TASK_PRIOR        ,	/* ??????,????,????????? */
                      ALGO_TASK_PRIOR        ,	/* ??ID,?????????? */
                      (OS_STK *)&algo_task_stack[0],/* ???????????OS_STK_GROWTH ???????? */
                      ALGO_TASK_STACKSIZE, /* ????? */
                      (void *)0,	/* ??????????,???????TCB?????
                                     (????????CPU????????)?????,?0?? */
                      OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); /* ????? */
	
//	OS_TMR    *encoder_tmr;
//	
//	encoder_tmr = OSTmrCreate(0, encoder_period, OS_TMR_OPT_PERIODIC,
//                          encoder_tmr_callback, NULL, "encoder-tmr", &err);

//	OSTmrStart(encoder_tmr, &err);								
	return 0;
}

static void send_encoder_unpolling(void)
{
	err = OSTaskCreateExt(send_encoder_task,	/* ???????? */
								(void *)0,		/* ???????? */
								(OS_STK *)&enc_task_stack[ENC_TASK_STACKSIZE - 1], /* ?????????? */
								ENC_TASK_PRIOR        ,	/* ??????,????,????????? */
								ENC_TASK_PRIOR        ,	/* ??ID,?????????? */
								(OS_STK *)&enc_task_stack[0],/* ???????????OS_STK_GROWTH ???????? */
								ENC_TASK_STACKSIZE, /* ????? */
								(void *)0,	/* ??????????,???????TCB?????
															 (????????CPU????????)?????,?0?? */
								OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); /* ????? */
}

static int can_msg_process(int usr,CanRxMsg* msg)
{
	uint32_t tmp[2];



	if( (msg->IDE != CAN_Id_Standard) || (msg->RTR != CAN_RTR_Data) )
			return -PERR_ENOTSUP;

	memcpy(tmp,msg->Data,16);
	switch(msg->StdId)
	{
	case AGV_CMD_TYPE_VEL|PC_CANID:
		vel_data_algo.vel_l=tmp[0];
		vel_data_algo.vel_r=tmp[1];
		break;
	case AGV_CMD_TYPE_ENC|PC_CANID:
		send_encoder_unpolling();
		break;
	default:
		break;
	}

	return PERR_EOK;
}

int _encCntL=0;
int _encCntR=0;
static void canSendTask(void *param)
{
	while(1)
	{
	CanTxMsg testMsg;

	encoder_data.encoder_l=get_count_and_clear(encoder_l);
	encoder_data.encoder_r=get_count_and_clear(encoder_r);
				
//	encoder_vel[0]=(float)encoder_data.encoder_l/encoder_period;
//	encoder_vel[1]=(float)encoder_data.encoder_r/encoder_period;
//		_encCntL=get_encoder_count(encoder_l);
//		_encCntR=get_encoder_count(encoder_r);
		
	testMsg.StdId=SAGV_CANID|ENCODER_CANID;
	testMsg.IDE=CAN_Id_Standard;
	testMsg.RTR=CAN_RTR_Data;
	testMsg.DLC=8;


	memcpy(testMsg.Data,&encoder_data,16);
	
	can_dispatcher_send_msg(0,&testMsg);
  OSTimeDly(MS_TO_TICKS(10));
	}
}

static void sAGV_algo_run(void* param)
{

	vel_data_algo.vel_l=0;
	vel_data_algo.vel_r=0;
	
	while(1)
	{
		uint32_t vel_l=0,vel_r=0;
		
		int dir_l=0,dir_r=1;
		if(vel_data_algo.vel_l>0)
		{
			dir_l=MOVE_FOREWARD_L;
			vel_l=vel_data_algo.vel_l;
		}
		else if(vel_data_algo.vel_l<0)
		{
			dir_l=MOVE_ROLLBACK_L;
			vel_l=-1*vel_data_algo.vel_l;
		}
			
		if(vel_data_algo.vel_r>0)
		{
			dir_r=MOVE_FOREWARD_R;
			vel_r=vel_data_algo.vel_r;
		}
		else if(vel_data_algo.vel_l<0)
		{
			dir_r=MOVE_ROLLBACK_R;
			vel_r=-1*vel_data_algo.vel_r;
		}
		
		motor_set_direction(dir_l, dir_r);
		Drive(vel_l,vel_r);
		
		OSTimeDly(MS_TO_TICKS(10));
	}
}
	
static void send_encoder_task(void* param)
{
		CanTxMsg testMsg;

		encoder_data.encoder_l=get_count_and_clear(encoder_l);
		encoder_data.encoder_r=get_count_and_clear(encoder_r);
				
	float encoder_vel[2];
	encoder_vel[0]=(float)encoder_data.encoder_l/encoder_period;
	encoder_vel[1]=(float)encoder_data.encoder_r/encoder_period;
//		_encCntL=get_encoder_count(encoder_l);
//		_encCntR=get_encoder_count(encoder_r);
		
		testMsg.StdId=SAGV_CANID|ENCODER_CANID;
		testMsg.IDE=CAN_Id_Standard;
		testMsg.RTR=CAN_RTR_Data;
		testMsg.DLC=8;
	
	
		memcpy(testMsg.Data,&encoder_vel,16);
		
		can_dispatcher_send_msg(0,&testMsg);
	
	OSTaskDel(ENC_TASK_PRIOR);
}

	float encoder_vel[2];

static void encoder_tmr_callback(void *tmr, void*data)
{
	CanTxMsg testMsg;

	encoder_data.encoder_l=get_count_and_clear(encoder_l);
	encoder_data.encoder_r=get_count_and_clear(encoder_r);
				
//	encoder_vel[0]=(float)encoder_data.encoder_l/encoder_period;
//	encoder_vel[1]=(float)encoder_data.encoder_r/encoder_period;
//		_encCntL=get_encoder_count(encoder_l);
//		_encCntR=get_encoder_count(encoder_r);
		
	testMsg.StdId=SAGV_CANID|ENCODER_CANID;
	testMsg.IDE=CAN_Id_Standard;
	testMsg.RTR=CAN_RTR_Data;
	testMsg.DLC=8;


	memcpy(testMsg.Data,&encoder_data,16);
	
	can_dispatcher_send_msg(0,&testMsg);
}
