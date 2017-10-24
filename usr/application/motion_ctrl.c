#include <stdint.h>
#include <ucos_ii.h>

#include "common/PosixError.h"
#include "bsp_pwm.h"
#include "app_cfg.h"
#include "porting.h"
#include "reverseMotionControl.h"
#include "motion_ctrl.h"
#include "DataBank.h"
#include "rAGVControl.h"
#include "MCTools.h"
#include "Rotating.h"
#include "user_function.h"
#include "Navigation.h"
#include "motors.h"
#include "angle.h"

#include "bsp_io.h"
#include "encoder.h"

#define MOTIONCTRL_TASK_STACKSIZE    512

extern const float32 fOpenVel;

Uint16  Motionstyle = MOTIONSTATE_ONSTOPING;
Uint16  typeOfAction  = ACTION_STOP;   //当前动作
Uint16  typeOfActionNew = ACTION_STOP; //新动作

Uint16  iPath = 0;		// 
float32 vPath = 0;      //

float32 fCurVel = 0.0f;  
float32 fCurVelL = 0.0f;  
float32 fCurVelR = 0.0f;  
float32 fDiffVel = 0.0f;
float32 sRemain = 0; 
float32  S_Vdown=0.0f;
Uint16  ethCommBreakFlag = 0x00;

const float32 angleVDnRatio = 0.12f;
const float32 sVDnRatio = 0.08f;
Uint16 codeVersion = 0x1;


float32  angleRemain = 0;
float32  angleProcess = 0;
Uint16  AGV_Dir_Dest  = 0;
float32  wCurrent = 0;
float32  wCycle   = 0;
Uint16 turnWholeCycleCnt = 0x0;

float32 twoDAngleConver = 0.0f;
float32 twoDAngleDest = 0.0f;
float32 twoDAngleNew = 0.0f;

// 删除
//float32 twoDAngleOld = 0.0f;
//float32 twoDAngleNew = 0.0f;
//Uint16  acrossXAxisFlag = 0;



int encoder_l=0, encoder_r=0;

static OS_STK motionctrl_task_stack[MOTIONCTRL_TASK_STACKSIZE];

extern void InitAgvHeadDir(void);

static void motionctrl_task(void *param)
{
    APP_TRACE("motionctrl_task: task run.\r\n");
    agv_motion_ctrl();
}

void motionctrl_run(motevent_cb_t event_callback)
{
    uint8_t err;

    /*For debuging*/
    encoder_l = open_encoder(1);
    encoder_r = open_encoder(2);

    OSTaskCreateExt(motionctrl_task,	/* 启动任务函数指针 */
                    (void *)0,		/* 传递给任务的参数 */
                    (OS_STK *)&motionctrl_task_stack[MOTIONCTRL_TASK_STACKSIZE - 1], /* 指向任务栈栈顶的指针 */
                    MOTIONCTRL_TASK_PRIOR        ,	/* 任务的优先级，必须唯一，数字越低优先级越高 */
                    MOTIONCTRL_TASK_PRIOR        ,	/* 任务ID，一般和任务优先级相同 */
                    (OS_STK *)&motionctrl_task_stack[0],/* 指向任务栈栈底的指针。OS_STK_GROWTH 决定堆栈增长方向 */
                    MOTIONCTRL_TASK_STACKSIZE, /* 任务栈大小 */
                    (void *)0,	/* 一块用户内存区的指针，用于任务控制块TCB的扩展功能
                                （如任务切换时保存CPU浮点寄存器的数据）。一般不用，填0即可 */
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); /* 任务选项字 */

    OSTaskNameSet(MOTIONCTRL_TASK_PRIOR, "motion-ctrl", &err);
}

extern float32 detY0;
extern float32 detTheta0;
int moveTest(void){
	iPath=3;
	vPath=2;
}
int  motionctrl_cmd(agv_action_t cmd, uint32_t len, const uint8_t *param)
{
    Uint16  sPath = 0;      //
    int     dumperst;
    int     ret = -PERR_EBUSY;
		uint8_t test[8];
		test[0]=param[0];
		test[1]=param[1];
		test[2]=param[2];
		test[3]=param[3];
	
    APP_TRACE("motion-ctrl: agv cmd %d\r\n", cmd);
    APP_TRACE("detY = %f detTheta = %f\r\n", detY0, detTheta0);

    dumperst = dumper_get_state();
    if( (dumperst == DUMPER_STATE_MOVINGUP) || (dumperst == DUMPER_STATE_TOP) )
    {
        return -PERR_EBUSY;
    }

		switch(cmd)
		{
			 	 case ACTION_GOSTRAIGHT:
					    if((cmd == typeOfAction) || ((cmd != typeOfAction) && (Motionstyle == MOTIONSTATE_ONSTOPING)))
					    {
									if(Motionstyle == MOTIONSTATE_ONSTOPING)
									{
                      motor_set_direction(MOVE_FOREWARD_L, MOVE_FOREWARD_R);
                      clear_encoder(encoder_l);
                      clear_encoder(encoder_r);

                      OSTimeDly(MS_TO_TICKS(500));
									}
                  /*
                   * 由于motion-ctrl部分代码，对这些数据的访问比较零散。
                   * 通过互斥锁很难进行保护，故访问这些数据时采用禁止调度的方式对数据进行保护。
                   *  这样做的前提是：
                   *  Motion-ctrl 线程的优先级一定要高于can-dispatcher线程(即当前这一段程序所运行的上下文)
                   * */
                  OSSchedLock();

									typeOfAction = cmd;
									Motionstyle = MOTIONSTATE_GOSTRAIGHT;	

									sPath = (((uint32_t)param[0]<<24)|((uint32_t)param[1]<<16)|((uint32_t)param[2]<<8)|((uint32_t)param[3]));        //mm 
#if 0
                  sPath = (sPath/500)*twoD_Distance;
#endif
									sRemain += sPath ;
                  iPath   += ( sPath / twoD_Distance );
									vPath = (float32)((uint32_t)(((uint32_t)param[4]<<24)|((uint32_t)param[5]<<16)|((uint32_t)param[6]<<8)|((uint32_t)param[7])))/10.0f;

									//VPlan(sRemain,fAccUp,fAccDn,fCurVel,vPath);
									//S_Vdown = 0.5f * fTargetVel * fTargetVel / fAccDn +  sVDnRatio * fTargetVel;

                  OSSchedUnlock();

                  APP_TRACE("Motion cmd: go straight. legth = %d, vel = %f\r\n", sPath, vPath);

									ret = PERR_EOK;
					    }
					    else
					    {
									ret = -PERR_EBUSY;
              }
              break;

				   case ACTION_GOBACKWARD:
					  	if(Motionstyle == MOTIONSTATE_ONSTOPING)
              {
                  motor_set_direction(MOVE_ROLLBACK_L, MOVE_ROLLBACK_R);

                  clear_encoder(encoder_l);
                  clear_encoder(encoder_r);

                  OSTimeDly(MS_TO_TICKS(500));
                  /*
                   * 由于motion-ctrl部分代码，对这些数据的访问比较零散。
                   * 通过互斥锁很难进行保护，故访问这些数据时采用禁止调度的方式对数据进行保护。
                   *  这样做的前提是：
                   *  Motion-ctrl 线程的优先级一定要高于can-dispatcher线程(即当前这一段程序所运行的上下文)
                   * */
                  OSSchedLock();

									typeOfAction = cmd;
									Motionstyle = MOTIONSTATE_GOBACKWARD;

									sPath = (((uint32_t)param[0]<<24)|((uint32_t)param[1]<<16)|((uint32_t)param[2]<<8)|((uint32_t)param[3]));        //mm 
#if 0
                  sPath = (sPath/500)*twoD_Distance;
#endif
									sRemain += sPath ;
                  iPath   += ( sPath / twoD_Distance );
									vPath = (float32)((uint32_t)(((uint32_t)param[4]<<24)|((uint32_t)param[5]<<16)|((uint32_t)param[6]<<8)|((uint32_t)param[7])))/10.0f;
									//VPlan(sRemain,fAccUp,fAccDn,fCurVel,vPath);
									//S_Vdown = 0.5f * fTargetVel * fTargetVel / fAccDn +  sVDnRatio * fTargetVel;
                  OSSchedUnlock();

                  APP_TRACE("Motion cmd: go backward. legth = %d, vel = %f\r\n", sPath, vPath);

									ret = PERR_EOK;
              }
              break;

				   case ACTION_UNLOAD:
					  	if(Motionstyle == MOTIONSTATE_ONSTOPING)
							{
								nReverseFlag = 0x1;
                APP_TRACE("Motion cmd: unload\r\n");

								ret = PERR_EOK;
							}
							else
							{
								ret = -PERR_EBUSY;
							}

              break;
				   case ACTION_TRUNLEFT:
				 		  if(Motionstyle == MOTIONSTATE_ONSTOPING)
				 		  {
                  motor_set_direction(MOVE_ROLLBACK_L, MOVE_FOREWARD_R);
                  clear_encoder(encoder_l);
                  clear_encoder(encoder_r);

                  OSTimeDly(MS_TO_TICKS(500));

                  /*
                   * 由于motion-ctrl部分代码，对这些全局数据的访问比较零散。
                   * 通过互斥锁很难进行保护，故访问这些数据时采用禁止调度的方式对数据进行保护。
                   *  这样做的前提是：
                   *  Motion-ctrl 线程的优先级一定要高于can-dispatcher线程(即当前这一段程序所运行的上下文)
                   * */
                  OSSchedLock();

                  typeOfAction = cmd;
                  Motionstyle = MOTIONSTATE_TRUNLEFT;

                  Get_TwoDDev();
                  angleRemain  = (((uint32_t)param[0]<<24)|((uint32_t)param[1]<<16)|((uint32_t)param[2]<<8)|((uint32_t)param[3]))/10.0f - GS_Angle;      	  
                  AGV_Dir_Dest = (((((uint32_t)param[0]<<24)|((uint32_t)param[1]<<16)|((uint32_t)param[2]<<8)|((uint32_t)param[3]))/900 + AGV_Head_Dir)) % 4;
                  angleProcess = 0.0f;
                  turnWholeCycleCnt = ((((uint32_t)param[0]<<24)|((uint32_t)param[1]<<16)|((uint32_t)param[2]<<8)|((uint32_t)param[3])))/3600;	

                  wCycle       = ((((uint32_t)param[4]<<24)|((uint32_t)param[5]<<16)|((uint32_t)param[6]<<8)|((uint32_t)param[7])) & 0xFFFF)/10.0f;
                  CyclePlan(angleRemain,wAccUp,wAccDn,wCurrent,wCycle);
                  angleVDown = 	0.5f * wTarget * wTarget /wAccDn + angleVDnRatio * wTarget;

                  twoDAngleDest = twoDAngleConver+ angleRemain;
									
									ExtendAngleInit(twoDAngleConver);
                  RotateInit(twoD_XP, twoD_YP, twoDAngleConver, twoDAngleDest);
                  
									// 删除
									//twoDAngleOld  = twoDAngleConver ;
                  //twoDAngleNew  = twoDAngleConver ;
                  //acrossXAxisFlag = 0;





                  OSSchedUnlock();

                  APP_TRACE("Motion cmd: trunleft. angle = %f, w = %f\r\n", angleRemain, wCycle);
                  ret = PERR_EOK;
              }
              else
              {
                ret = -PERR_EBUSY;
              }
              break;
				   case ACTION_TRUNRIGHT:
					  	  if(Motionstyle == MOTIONSTATE_ONSTOPING)
				 		    {
                    motor_set_direction(MOVE_FOREWARD_L, MOVE_ROLLBACK_R);
                    clear_encoder(encoder_l);
                    clear_encoder(encoder_r);

                    OSTimeDly(MS_TO_TICKS(500));
                    /*
                   * 由于motion-ctrl部分代码，对这些数据的访问比较零散。
                   * 通过互斥锁很难进行保护，故访问这些数据时采用禁止调度的方式对数据进行保护。
                   *  这样做的前提是：
                   *  Motion-ctrl 线程的优先级一定要高于can-dispatcher线程(即当前这一段程序所运行的上下文)
                   * */
                    OSSchedLock();


                    typeOfAction = cmd;
                    Motionstyle = MOTIONSTATE_TRUNRIGHT;

                    Get_TwoDDev();
                    angleRemain  = (((uint32_t)param[0]<<24)|((uint32_t)param[1]<<16)|((uint32_t)param[2]<<8)|((uint32_t)param[3]))/10.0f + GS_Angle;      
                    AGV_Dir_Dest = (4 + AGV_Head_Dir - (((((uint32_t)param[0]<<24)|((uint32_t)param[1]<<16)|((uint32_t)param[2]<<8)|((uint32_t)param[3])))/900)) % 4;
                    angleProcess = 0.0f;
                    turnWholeCycleCnt = ((((uint32_t)param[0]<<24)|((uint32_t)param[1]<<16)|((uint32_t)param[2]<<8)|((uint32_t)param[3]))>>16)/3600;	
                    wCycle       = ((((uint32_t)param[4]<<24)|((uint32_t)param[5]<<16)|((uint32_t)param[6]<<8)|((uint32_t)param[7])) & 0xFFFF)/10.0f; 
                    CyclePlan(angleRemain,wAccUp,wAccDn,wCurrent,wCycle);
                    angleVDown = 	0.5f * wTarget * wTarget /wAccDn + angleVDnRatio * wTarget;

                    twoDAngleDest = twoDAngleConver - angleRemain;		

                    ExtendAngleInit(twoDAngleConver);
                    RotateInit(twoD_XP, twoD_YP, twoDAngleConver, twoDAngleDest);
                    
										// 删除
										//twoDAngleOld  = twoDAngleConver ;
                    //twoDAngleNew  = twoDAngleConver ;
                    //acrossXAxisFlag = 0;




                    OSSchedUnlock();

                    APP_TRACE("Motion cmd: trunright. angle = %f, w = %f\r\n", angleRemain, wCycle);
                    ret = PERR_EOK;
                }
                else
                {
                  ret = -PERR_EBUSY;
                }
                break;

				   case ACTION_CHARGE:
                typeOfAction = cmd;

                APP_TRACE("Motion cmd: charge\r\n");
                ret = PERR_EOK;
                break;
				   case ACTION_STOP_CHARGE:
				   	  typeOfAction = cmd;
              APP_TRACE("Motion cmd: stop charge\r\n");
              ret = PERR_EOK;
              break;

				   default:
              ret = -PERR_ENOTSUP;
              break;
    }

    return ret;
}

void get_motion_state(motion_info_t *state)
{
    state->position.point = twoD_TagCode;
    state->position.x_deviation = twoD_XP*10;
    state->position.y_deviation = twoD_YP*10;

    state->direction.direction = AGV_Head_Dir;//twoD_Angle*10;
    state->direction.deviation = GS_Angle;

    state->state = (motion_state_t)Motionstyle;
    state->speed = fCurVel*10;
    state->action = typeOfAction;

    return ;
}


