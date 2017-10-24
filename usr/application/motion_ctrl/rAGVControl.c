#include <stdlib.h>

#include "ucos_ii.h"

#include "common/tools.h"

#include "rAGVControl.h"
#include "DataBank.h"
#include "Tracingzq.h"
#include "Rotating.h"
#include "user_function.h"
#include "app_cfg.h"
#include "agv.h"
#include "Navigation.h"
#include "encoder.h"
#include "motion_ctrl.h"
#include "TshapeVPlan.h"
#include "common/PosixError.h"
#include "MCTools.h"
#include "angle.h"
#include "MotionEstimate.h" 

Uint16 reverseMode = REVERSE_MODE_HORIZONTAL;
Uint16 reverseCnt = 0;
Uint16 nReverseFlag = 0;


//导航和脱轨
static Uint16 GSMissing = 0x00 ; //导航丢失， 默认为不丢失
static Uint16 OutOfTrack = 0x00 ; //脱轨， 默认为不脱轨

//速度相关
static Uint16   nEvent = NO_EVENT;        // 当前所响应的事件

float32  fFwdObsVel = 50.0f;   //前方有障碍物（远）时降到此速度
Uint16   uObClearCount = 0x00;     //远距离障碍物标志，及远距离障碍物减速次数

#if 0
Uint16 actionFinishSeq = 0x0;
Uint32 actionFinishReply = 0x0;
#endif
const float32 fOpenVel = 40.0f; 
const float32 wOpenVel = 1.0f; 


Uint16 	nAgvWorkMode = AGV_MODE_STANDBY;

float32 twoD_Progress = 0.0f;
float32 RealTwoDProgress = 0.0f;

float rotateR = 0.0f;

Uint16 twoDMissingCnt = 0x0;


static void error_event(int error);
//static void error_recovery(int error);

void AGV_Run(void)
{
   switch(nAgvWorkMode)
   {
      case AGV_MODE_STANDBY: AGV_Standby();
	                         break;
	  case AGV_MODE_RUNNING: AGV_Running();
	                         break;
	  case AGV_MODE_OP:      AGV_Operation();
							 break;
	  case AGV_MODE_SUSPENDED:AGV_SuspendRun();
	                          break;
      default: break;
   }
}

/*-----------------------------
   待机操作函数
-----------------------------*/
void AGV_Standby(void)
{
	if(1)
	{
		nAgvWorkMode = AGV_MODE_RUNNING;

		wCurrent = 0.0f;
		fCurVel = 0.0f;
		fCurVelL = 0.0f;
		fCurVelR = 0.0f;
		fTargetVel = 0.0f;
		twoD_Progress = 0.0f;
		RealTwoDProgress = 0.0f;
	}
}

uint8_t dflag[3] = {0};

void AGV_Running(void)
{
	float32 fVelInc, fVelDec;
	float32  wCurInc, wCurDec;
  static  float s_max_vel = 0;
  float     fActVel;  // 真实线速度
  float     wActVel;  // 真实角速度
  //uint32_t  difftm;
  //uint32_t  tm;
  uint8_t   data[16];
  float     l;
  int32 deltaVL = 0;
  int32 deltaVR = 0;  //50ms中 左右码盘的计数值。

#if 0
  static uint32_t old_tm = 0;
  tm = OSTimeGet();
  difftm = TIME_DIFFERENCE(tm, old_tm);
  old_tm = tm;

  //AGCRUN_CYCLE = difftm;
#endif
  //APP_TRACE("AGV run. difftm = %d\r\n", difftm);

	fVelInc = (AGCRUN_CYCLE/1000.0f) * fAccUp;              // AGCRUN_CYCLE周期加速步长
	fVelDec = (AGCRUN_CYCLE/1000.0f) * fAccDn;               // AGCRUN_CYCLE周期减速步长

	wCurInc = (AGCRUN_CYCLE/1000.0f) * wAccUp;
	wCurDec = (AGCRUN_CYCLE/1000.0f) * wAccUp;

  DISABLE_INT();
  deltaVL = get_count_and_clear(encoder_l);
  deltaVR = get_count_and_clear(encoder_r);
  ENABLE_INT();

  if( (Motionstyle == ACTION_MODE_GOAHEAD)
    || (Motionstyle == ACTION_MODE_GOBACK)
    || (Motionstyle == ACTION_MODE_TURNLEFT)
    || (Motionstyle == ACTION_MODE_TURNRIGHT) )
    APP_TRACE("encoder: left=%d, right=%d\r\n", deltaVL, deltaVR);

  //Check encoder error
  if(Motionstyle != ACTION_MODE_STOP)
  {
    int expect_cnt_l, expect_cnt_r;
    static int  encoder_errcnt_l=0, encoder_errcnt_r=0;

    expect_cnt_l = LENGTH_TO_ENCODERCNT(fCurVelL)*AGCRUN_CYCLE/1000;
    expect_cnt_r = LENGTH_TO_ENCODERCNT(fCurVelR)*AGCRUN_CYCLE/1000;

    if( (abs(abs(expect_cnt_l) - abs(deltaVL)) > 30)
        && ((abs(expect_cnt_l) > 2*abs(deltaVL))||((abs(deltaVL) > 2*abs(expect_cnt_l)))) )
    {
      ++encoder_errcnt_l;

      APP_TRACE("left wrong encoder cnt %d, %d is expected. err cnt = %d\r\n",
                deltaVL, expect_cnt_l, encoder_errcnt_l);

      deltaVL = expect_cnt_l;
    }
    else if(encoder_errcnt_l > 0)
      --encoder_errcnt_l;

    if( ((abs(abs(expect_cnt_r) - abs(deltaVR))) > 30)
        && ((abs(expect_cnt_r) > 2*abs(deltaVR))||((abs(deltaVR) > 2*abs(expect_cnt_r)))) )
    {
      ++encoder_errcnt_r;

      APP_TRACE("right wrong encoder cnt %d, %d is expected. err cnt = %d\r\n",
                deltaVR, expect_cnt_r, encoder_errcnt_r);

      deltaVR = expect_cnt_r;
    }
    else if(encoder_errcnt_r > 0)
      --encoder_errcnt_r;

    if(encoder_errcnt_l > 5)
    {
        APP_TRACE("left encoder error.\r\n");
        error_event(ENCODER_L_ERROR);
    }
    if(encoder_errcnt_r > 5)
    {
        APP_TRACE("right encoder error.\r\n");
        error_event(ENCODER_R_ERROR);
    }
  }

	if(NO_EVENT == nEvent)
  {
    if(ethCommBreakFlag == 0x1)   //可恢复停车标志位
		{
			nEvent = CONTROL_RESUMABLE_STOP_EVENT;
		}
		else if (OutOfTrack == 0x01)                              // 检查导航脱轨事件
    {
      APP_TRACE("Error event: out of track\r\n");
			nEvent = OUT_OF_TRACK_EVENT;
      error_event(nEvent);
    }
		else if (GSMissing == 0x01)                    // 检查CAN导航信号断事件, check 此处是否需要修改
    {
      APP_TRACE("Error event: navi out of time\r\n");
			nEvent = GS_COMM_BREAK_EVENT;
      error_event(nEvent);
    }
		else if(ServoLeftInvaild())
		{
      APP_TRACE("Error event: sevo invalid\r\n");
			nEvent = SERVOL_INVALID_EVENT;
      //error_event(nEvent);
		}
		else if(ServoRightInvalid())
		{
      APP_TRACE("Error event: sevo invalid\r\n");
			nEvent = SERVOR_INVALID_EVENT;
      //error_event(nEvent);
		}

		else
		{
/*------------------------------------------直行--------------------------------------------------*/
			if( (Motionstyle == ACTION_MODE_GOAHEAD) || (Motionstyle == ACTION_MODE_GOBACK) )
			{

        /* use encoder*/
#if 0
				RealTwoDProgress += (((abs(fCurVelL) + abs(fCurVelR))* 0.5f)* (AGCRUN_CYCLE/1000.0f));
				sRemain = sRemain - (((abs(fCurVelL) + abs(fCurVelR))* 0.5f)* (AGCRUN_CYCLE/1000.0f));
#else

        l = ENCODERCNT_TO_LENGTH((abs(deltaVL)+abs(deltaVR))*0.5f);

        RealTwoDProgress += l;
        sRemain -= l;
#endif

#if 0
        APP_TRACE("AGV go straight, iPath=%d, fCurVel = %f, remain=%f, fTargetVel=%f\r\n",
               iPath, fCurVel, sRemain,fTargetVel);
#endif
			// 处理路径跟踪	
				UpdateAgvHeadDirToNew();
				Get_TwoDDev();

        if(Motionstyle == ACTION_MODE_GOBACK)
        {
            GS_DevFB = -GS_DevFB;
            GS_DEV = -GS_DEV;
        }

        /*TODO: tobe checked */
#if 1
        twoD_Progress += l;
#else
        twoD_Progress += fCurVel * AGCRUN_CYCLE/1000.0f;  //计算所得单位为mm： mm/s * ms 每隔50ms才加一次？
#endif

        //fActVel = (ENCODERCNT_TO_LENGTH(abs(deltaVL+deltaVR)/2.0) * 1000)/AGCRUN_CYCLE;
        fActVel = l*1000.0/AGCRUN_CYCLE;

        if(fActVel > s_max_vel)
            s_max_vel = fActVel;

        float s_Open;

        s_Open = (s_max_vel*70.0)/1000.0;
        //if(s_Open < 40)
         //   s_Open = 40;

        APP_TRACE("ActVel = %.2f\r\n", fActVel);
        SmoothVelocityPlanning(sRemain-s_Open, fAccUp, fAccDn, fActVel, fOpenVel,
                               vPath, &fTargetVel);
#if 0
 				if ( sRemain < S_Vdown)
				{
					  if(fTargetVel > fOpenVel)   //由原来的不等于0改为了大于fOpenVel
            {
              fTargetVel = fOpenVel ;
            }
				}
				else
				{
					  VPlan(sRemain,fAccUp,fAccDn,fCurVel,vPath);
				}
#else
#if 0
        if((sRemain < S_Vdown) & (sRemain >= (S_Vdown*0.67)) )   
        {
          if(dflag[0] == 0)
            fTargetVel = fCurVel * 0.9f; 

          fVelDec = fVelDec * 1.5;
          dflag[0] = 1;
        }
				else if((sRemain < (S_Vdown*0.67)) & (sRemain >= (S_Vdown*0.3)) )
        {
          if(dflag[1] == 0)
            fTargetVel = fCurVel * 0.6f;  

          fVelDec = fVelDec * 1.2;
          dflag[1] = 1;
        }
				else if((sRemain < (S_Vdown*0.3)) )
        {
          if(dflag[2] == 0)
            fTargetVel = fCurVel * 0.05f;  //original 0.05
          if(fTargetVel < 40)
            fTargetVel = 40.0;

          fVelDec = fVelDec * 1.2;
          dflag[2] = 1;
        }
				else if(sRemain >= S_Vdown)
        {
          VPlan((sRemain-0.0f),fAccUp,fAccDn,fCurVel,vPath); //original sRemain-120.0
          dflag[0]=0;
          dflag[1]=0;
          dflag[2]=0;
        }
#endif
#endif
				if(fCurVel < 0.01f)
        {
					  //pInitzq(GS_DEV * 0.001,GS_Angle * 0.01745);	
					  pInitzq(GS_DEV * 0.001, ANGLE_TO_RADIAN(GS_Angle));
				}
				else if( NewTag != 0x00 )
				{
            //APP_TRACE("%d: tag: %d\r\n", OSTimeGet(), twoD_TagCode);
            //APP_TRACE("DEV_FB = %f, DEV =%f, GS_Angle=%f\r\n", GS_DevFB, GS_DEV, GS_Angle);
            //APP_TRACE("GS_DevFB = %f\r\n", GS_DevFB);

            RealTwoDProgress = GS_DevFB;  //两个二维码之间经过的路程  为了消除累计误差
            twoD_Progress = GS_DevFB;  //两个二维码之间经过的路程  为了消除累计误差

						if( sRemain < 300.0f)  //目的地
						{
              //APP_TRACE("sRemain = %f\r\n", sRemain);
							sRemain = -GS_DevFB ;
							if(((absVar(sRemain)) < sStopDistance )||(GS_DevFB >0.0001f))
							{
								fTargetVel = 0.0f;
							}
						}
					  else				// 过程站
            {
              iPath -- ;  //有新码则减少一个路段的计数
              sRemain = twoD_Distance * iPath - GS_DevFB;

              //pInitzq(GS_DEV * 0.001,GS_Angle * 0.01745);
              pInitzq(GS_DEV * 0.001, ANGLE_TO_RADIAN(GS_Angle));

              NewTag = 0x00;   //清零是为了两个二维码之间拟合仅一次
            }
				}
				else if (RealTwoDProgress > (twoD_Distance + twoD_Distance/2.0))
				{
					twoD_TagCodeOld = 0x0;  //将码值清零，用于强校验，必须推到二维码上再清脱轨
          APP_TRACE("%d: OutOfTrack \r\n", OSTimeGet());
          OutOfTrack = 0x01 ;  //通过按钮或者 命令 清
				}
				else if (twoD_Progress > (twoD_Distance * 3.0f))  //此处应该用几个Sopen？？
				{
            //APP_TRACE("Motors: power off");
            motors_power_onoff(0);
				}
#if 0
				if (fCurVel < fTargetVel)
				{
          FApproach(&fCurVel, fTargetVel, fVelInc); // 直接采用路段最高允许速度	
				}
				else
					FApproach(&fCurVel, fTargetVel, fVelDec); // 
#else
				fCurVel = fTargetVel;
#endif

				if(fCurVel < 0.01f)
				{
            Drive(0x0,0x0);
            fCurVelL = 0;
            fCurVelR = 0;

            NewTag = 0x00;
            iPath = 0x00;

            if(Motionstyle == ACTION_MODE_GOAHEAD)
              data[0] = ACTION_GOSTRAIGHT;
            else
              data[0] = ACTION_GOBACKWARD;

            motionctrl_event_callback(MOTIONCTRL_EVENT_ACTION_OVER, 1, data);

            s_max_vel = 0;
            RealTwoDProgress = 0;
            clear_encoder(encoder_l);
            clear_encoder(encoder_r);

            if(Motionstyle == ACTION_MODE_GOBACK)
              GlideReset();

            Motionstyle = ACTION_MODE_STOP ;
        }
        else
        {
          // When velocity is very low 或丢失 或与传感器通信断, don't follow guide tape。
          if (nEvent == OUT_OF_TRACK_EVENT || nEvent == GS_COMM_BREAK_EVENT)
          {
            fDiffVel = 0.0f;
          }
          else
          {
            if(Motionstyle == ACTION_MODE_GOBACK)
              fDiffVel = CalcPzq(deltaVR, deltaVL);	  
            else
              fDiffVel = CalcPzq(deltaVL, deltaVR);	  
          }

          if(fCurVel <= fOpenVel+0.01)
              fDiffVel = 0;

          if(fDiffVel != 0)
          {
            //APP_TRACE("fDiffVel = %f\r\n", fDiffVel);
          }

          VControl(fCurVel, fDiffVel);	// 计算输出给速度环的控制量

          if(Motionstyle == ACTION_MODE_GOBACK)
              Drive(fCurVelR, fCurVelL);
          else
              Drive(fCurVelL, fCurVelR);
        }
			}
/*---------------------------------------------左转-----------------------------------------------*/
			else if(Motionstyle == ACTION_MODE_TURNLEFT)
			{
        if(wCurrent>1.0f)
        {
          ++twoDMissingCnt;
          if(twoDMissingCnt >100)
          {
            GSMissing = 0x01;
            twoD_TagCodeOld = 0x0;
          }
        }
				UpdateAgvHeadDirToNew();
				Get_TwoDDev();

        /*TODO:  use encoder*/
#if 0
				angleProcess +=  wCurrent * AGCRUN_CYCLE/1000.0f;  //计算所得单位为 度
#else
        angleProcess += LEGHTH_TO_ANGLE(ENCODERCNT_TO_LENGTH((abs(deltaVL)+abs(deltaVR))*0.5f));
#endif
				angleRemain = (minusL(AGV_Head_Dir , AGV_Dir_Dest))* 90.0f - GS_Angle;
				if((turnWholeCycleCnt == 0x1)&&(AGV_Head_Dir == AGV_Dir_Dest))
				{
				   angleRemain += 360.0f;
				}
				else
				{
					turnWholeCycleCnt = 0x0;
				}
                /*
				if(angleRemain < angleVDown)
				{
					if(wTarget > 5.0f)
					{
						wTarget = 5.0f ;
					}
				}
                */

				if( angleRemain < angleStopDistance )
				{
					wCurrent = wOpenVel;
					//angleProcess = 0;
				}
				
				else
				{
					//CyclePlan(angleRemain,wAccUp,wAccDn,wCurrent,wCycle);

                    wActVel = (ENCODERCNT_TO_LENGTH(deltaVR-deltaVL)/deltaT)/(AXLE_LENGTH);
					float wMax = 360; // the max angle speed, (degree per second)	
					SmoothVelocityPlanning(angleRemain - angleStopDistance, wAccUp, wAccDn, wActVel, wOpenVel,
                               wMax, &wCurrent);
				}
                /*
				if( angleRemain < angleStopDistance )
				{
					wCurrent = 0.0f ;
					angleProcess = 0;
				}
				*/

                /*
				if ( wCurrent <  wTarget )
					FApproach(&wCurrent, wTarget, wCurInc); // 直接采用路段最高允许速度
				else
					FApproach(&wCurrent, wTarget, wCurDec); // 直接采用路段最高允许速度
				*/

				if(wCurrent < 0.01f)
				{
				    Motionstyle = ACTION_MODE_STOP ;

            fCurVelL = 0;
            fCurVelR = 0;

            Drive(0x0,0x0);
            GlideReset();

            angleRemain = 0.0f ;
            angleProcess = 0.0f;

            data[0] = ACTION_TRUNLEFT;
            motionctrl_event_callback(MOTIONCTRL_EVENT_ACTION_OVER, 1, data);
        }

				// 删除
				#if 0
			  if(((270.0f < twoDAngleOld ) && (twoDAngleOld< 361.0f))&&((0.001f< twoDAngleConver)&&(twoDAngleConver < 90.0f))&&(acrossXAxisFlag == 0))
			  {
			  	 acrossXAxisFlag = 1;
           twoDAngleNew = twoDAngleConver + 360.0f;
           twoDAngleOld = twoDAngleConver;  //Old 始终为采集上来的角度信息
			  }
			  else if((acrossXAxisFlag == 1)&&((twoDAngleConver - twoDAngleOld) > 180.0f))
			  {
			  	 //此时过滤已经到第一象限又抖动回第四象限，不做任何处理
			  }
			  else if(acrossXAxisFlag == 1)
			  {
			  	 //此时处于跨X正轴的后续，一直需要往上增加
			  	 twoDAngleNew = twoDAngleConver + 360.0f;
				 twoDAngleOld = twoDAngleConver;
			  }
			  else
			  {
			  	 //此时未跨轴
			  	 twoDAngleNew = twoDAngleConver;
           twoDAngleOld = twoDAngleConver;
			  }
#endif
			if(GSMissing == 0x01){
				
				POSE newPose= MotionEstimate(deltaVL, deltaVR);
				twoD_XP = newPose.x;
				twoD_YP = newPose.y;
				twoDAngleNew = newPose.theta;
		      
			}else{
                twoDAngleNew=ExtendAngleExecute(twoDAngleConver);
				MotionEstimateSet(twoD_XP, twoD_YP, twoDAngleNew);
			}
				
			  rotateR =  CalcRotateP(twoD_XP, twoD_YP, twoDAngleNew);
#if 0
        APP_TRACE("X=%f, Y=%f, anglenew=%f\r\n", twoD_XP, twoD_YP, twoDAngleNew);
        if(rotateR != 0)
          APP_TRACE("rotateR =%f \r\n", rotateR);
#endif
			  WControl(wCurrent, rotateR);	// 计算输出给速度环的控制量
        Drive(fCurVelL, fCurVelR);
			}
/*-------------------------------------------右转------------------------------------------------*/
			else if(Motionstyle == MOTIONSTATE_TRUNRIGHT)
      {
			   if(wCurrent>1.0f)
			   {
				   ++twoDMissingCnt;
				   if(twoDMissingCnt >100)
				   {
				   		GSMissing = 0x01;
              twoD_TagCodeOld = 0x0;
				   }
			   }
			   UpdateAgvHeadDirToNew();
			   Get_TwoDDev();

        /*TODO:  use encoder*/
#if 0
			   angleProcess -=  wCurrent * AGCRUN_CYCLE/1000.0f;  //计算所得单位为 度
#else
         angleProcess -= LEGHTH_TO_ANGLE(ENCODERCNT_TO_LENGTH((abs(deltaVL)+abs(deltaVR))*0.5f));
#endif
			   angleRemain = (minusR(AGV_Head_Dir , AGV_Dir_Dest))* 90.0f + GS_Angle;
			   if((turnWholeCycleCnt == 0x1)&&(AGV_Head_Dir == AGV_Dir_Dest))
			   {
			   		angleRemain += 360.0f;
			   }
			   else
			   {
				    turnWholeCycleCnt = 0x0;
			   }

			   if( angleRemain < angleVDown )
			   {
					if(wTarget > 5.0f)
					{
						wTarget = 5.0f;
					}
			   }
			   else
			   {
			   		CyclePlan(angleRemain,wAccUp,wAccDn,wCurrent,wCycle);
			   }

			   if(angleRemain < angleStopDistance)
			   {
			   		wTarget = 0.0f;
					angleProcess = 0.0f;
			   }

				if ( wCurrent <  wTarget )
					FApproach(&wCurrent, wTarget, wCurDec); // 直硬捎寐范巫罡咴市硭俣?
				else
					FApproach(&wCurrent, wTarget, wCurInc); // 直接采用路段最高允许速度

				if(absVar(wCurrent) < 0.01f)
				{
            Motionstyle = ACTION_MODE_STOP ;

            fCurVelL = 0;
            fCurVelR = 0;

            Drive(0x0,0x0);

            GlideReset();
            angleRemain = 0.0f ;
            angleProcess = 0.0f;

            data[0] = ACTION_TRUNRIGHT;
            motionctrl_event_callback(MOTIONCTRL_EVENT_ACTION_OVER, 1, data);
				}

				// 删除
				#if 0
				if(((0.001f < twoDAngleOld)&&( twoDAngleOld< 90.0f))&&((270.0f< twoDAngleConver)&&(twoDAngleConver < 361.0f))&&(acrossXAxisFlag == 0))
				{
				   acrossXAxisFlag = 1;
				   twoDAngleNew = twoDAngleConver - 360.0f;
				   twoDAngleOld = twoDAngleConver;
				}
				else if((acrossXAxisFlag == 1)&&((twoDAngleOld-twoDAngleConver) > 180.0f))
				{
					//此时为了滤除已经由第一象限到第四象限，又抖动回第一象限的情况
				}
				else if(acrossXAxisFlag == 1)
				{
				   twoDAngleNew = twoDAngleConver - 360.0f;
				   twoDAngleOld = twoDAngleConver;
				}
				else
        {
          twoDAngleNew = twoDAngleConver;
          twoDAngleOld = twoDAngleConver;
        }

				#endif
				
				
				
				twoDAngleNew=ExtendAngleExecute(twoDAngleConver);
				
				rotateR =  CalcRotateP(twoD_XP, twoD_YP, twoDAngleNew);
#if 0
        APP_TRACE("X=%f, Y=%f, anglenew=%f\r\n", twoD_XP, twoD_YP, twoDAngleNew);
        if(rotateR != 0)
          APP_TRACE("rotateR =%f \r\n", rotateR);
#endif
				WControl(wCurrent, rotateR);	// 计算输出给速度环的控制量 

        Drive(fCurVelL, fCurVelR);
/*------------------------------------------翻板------------------------------------------------*/
		   }
	  }
  }
	if(NO_EVENT != nEvent)
	{
		if((nEvent == SERVOL_INVALID_EVENT)||(nEvent == SERVOR_INVALID_EVENT))
		{
			sRemain = 0.0f;
			angleRemain = 0.0f;
      fCurVelL = 0;
      fCurVelR = 0;
			Motionstyle = ACTION_MODE_STOP;
			nAgvWorkMode = AGV_MODE_SUSPENDED;
		}

		if((nEvent == GS_COMM_BREAK_EVENT )&&((Motionstyle == ACTION_MODE_TURNLEFT)||(Motionstyle == ACTION_MODE_TURNRIGHT)))
		{
			angleRemain = 0.0f;
      fCurVelL = 0;
      fCurVelR = 0;
			nAgvWorkMode = AGV_MODE_SUSPENDED;
			Motionstyle = ACTION_MODE_STOP ;
		}

    if( (Motionstyle == ACTION_MODE_GOAHEAD) || (Motionstyle == ACTION_MODE_GOBACK) )
		{
			UpdateAgvHeadDirToNew();
			Get_TwoDDev();
#if 0
			twoD_Progress += fCurVel * AGCRUN_CYCLE/1000.0f;
#else
      l = ENCODERCNT_TO_LENGTH((abs(deltaVL)+abs(deltaVR))*0.5f);
      twoD_Progress += l;
#endif

			if( NewTag != 0x00 )
			{
				RealTwoDProgress = GS_DevFB;  //两个二维码之间经过的路程  为了消除累计误差
				twoD_Progress = GS_DevFB;  //两个二维码之间经过的路程  为了消除累计误差

				if( sRemain < 300.0f )  //目的地
				{
					  sRemain = GS_DevFB ;				
				}
				else				// 过程站  
				{
					  iPath--;
					  sRemain = twoD_Distance * iPath - GS_DevFB;

				    //pInitzq(GS_DEV * 0.001,GS_Angle * 0.01745);
					  pInitzq(GS_DEV * 0.001, ANGLE_TO_RADIAN(GS_Angle));

					NewTag = 0x00;   //清零是为了两个二维码之间拟合仅一次
				}
			}

			if (FApproach(&fCurVel, 0.0f, fVelDec))
			{
				if(nEvent == OUT_OF_TRACK_EVENT )//脱轨需人为干涉上线，需清距离值
				{
					twoD_Progress = 0.0f;
					RealTwoDProgress = 0.0f;
					sRemain = 0.0f;	
					iPath = 0x00;
          fCurVelL = 0;
          fCurVelR = 0;

          /*先减速再停止*/
					//Motionstyle = ACTION_MODE_STOP ;
				}
				else if (nEvent == CONTROL_RESUMABLE_STOP_EVENT)
				{
					nAgvWorkMode = AGV_MODE_SUSPENDED;
				}
			}

      /*modified by SunLiang   2017-04-12 Start*/
      if(fCurVel < 0.01f)
      {
          fCurVelL = 0;
          fCurVelR = 0;
					Drive(0x0,0x0);
          Motionstyle = ACTION_MODE_STOP ;
      }
      else
      {
        fDiffVel = CalcPzq(deltaVL, deltaVR);
        VControl(fCurVel, fDiffVel);	// 计算输出给速度环的控制量

        Drive(fCurVelL, fCurVelR);
      }
      /*modified by SunLiang   2017-04-12 End*/

			if(CONTROL_RESUMABLE_STOP_EVENT == nEvent)
			{
				if(ethCommBreakFlag == 0)
				{
					nAgvWorkMode = AGV_MODE_RUNNING;
					nEvent = NO_EVENT;
				}
			}
		}
		else if( Motionstyle == ACTION_MODE_STOP )
		{
			nAgvWorkMode = AGV_MODE_SUSPENDED;    // 一般情况，转换到SUSPENDED模式
		}
	}
}

void AGV_SuspendRun(void)
{
	//此处断掉PWM输出，可选择关闭定时器？
//	EvaRegs.T1CON.bit.TENABLE	= 0;  //禁止定时器
//	EvbRegs.T3CON.bit.TENABLE	= 0;

  /*TODO: 应该*/
	Drive(0x0,0x0);
  motors_power_onoff(0);
	//DO_SERVO_POWERUP = 0x01;	// not power up	

	switch(nEvent)
	{

		case OUT_OF_TRACK_EVENT:
		{
			if (OutOfTrack == 0x00)
			{
        APP_TRACE("Error: OutOfTrack \r\n");

				nAgvWorkMode = AGV_MODE_RUNNING;
				fCurVel = 0.0f;
				fDiffVel = 0.0f;
				wCurrent = 0;
				iPath = 0;
				NewTag = 0x00;
				fCurVelL = 0;
				fCurVelR = 0;
				nEvent = NO_EVENT;
        motors_power_onoff(1);
			//	DO_SERVO_POWERUP = 0x00;  //动力回路上电
//				EvaRegs.T1CON.bit.TENABLE	= 1;  //使能定时器
			//	EvbRegs.T3CON.bit.TENABLE	= 1;
#if 0
				actionFinishReply =  (((Uint32)actionFinishSeq)<<24);
				actionFinishReply |= 0x00010100;
				actionFinishSeq ++;
		//		CAN_SendPacket(13,actionFinishReply,ACK_OK);
#endif
			}
			break;
		}
		case GS_COMM_BREAK_EVENT:
		{
			if (GSMissing  == 0x00)
			{
				nAgvWorkMode = AGV_MODE_RUNNING;
				fCurVel = 0.0f;
				fDiffVel = 0.0f;
				wCurrent = 0;
				iPath = 0;
				NewTag = 0x00;
				fCurVelL = 0;
				fCurVelR = 0;
				nEvent = NO_EVENT;
		//		DO_SERVO_POWERUP = 0x00;  //动力回路上电
        motors_power_onoff(1);
				twoDMissingCnt = 0x0;
		//		EvaRegs.T1CON.bit.TENABLE	= 1;  //使能定时器
		//		EvbRegs.T3CON.bit.TENABLE	= 1;
#if 0
				actionFinishReply =  (((Uint32)actionFinishSeq)<<24);
				actionFinishReply |= 0x00010200;
				actionFinishSeq ++;
		//		CAN_SendPacket(13,actionFinishReply,ACK_OK);
#endif
			}
			break;
		}
		case CONTROL_RESUMABLE_STOP_EVENT:
		{
			if (ethCommBreakFlag==0x00)
			{
				nAgvWorkMode = AGV_MODE_RUNNING;
				fCurVel = 0.0f;
				fDiffVel = 0.0f;
				wCurrent = 0;
				fCurVelL = 0;
				fCurVelR = 0;
				nEvent = NO_EVENT;
        motors_power_onoff(1);
		//		DO_SERVO_POWERUP = 0x00;  //动力回路上电
		//		EvaRegs.T1CON.bit.TENABLE	= 1;  //使能定时器
		//		EvbRegs.T3CON.bit.TENABLE	= 1;
			}
			break;
		}
		case SERVOL_INVALID_EVENT:
		{
			if(ServoLeftInvaild() == 0x00)
			{
				nAgvWorkMode = AGV_MODE_RUNNING;
				fCurVel = 0.0f;
				fDiffVel = 0.0f;
				wCurrent = 0;
				iPath = 0;
				fCurVelL = 0;
				fCurVelR = 0;
				nEvent = NO_EVENT;
        motors_power_onoff(1);
			//	DO_SERVO_POWERUP = 0x00;  //动力回路上电
			//	EvaRegs.T1CON.bit.TENABLE	= 1;  //使能定时器
		//		EvbRegs.T3CON.bit.TENABLE	= 1;
			}
			break;
		}
		case SERVOR_INVALID_EVENT:
		{
			if(ServoRightInvalid() == 0x00)
			{
				nAgvWorkMode = AGV_MODE_RUNNING;
				fCurVel = 0.0f;
				fDiffVel = 0.0f;
				wCurrent = 0;
				iPath = 0;
				fCurVelL = 0;
				fCurVelR = 0;
				nEvent = NO_EVENT;
        motors_power_onoff(1);
			}
			break;
		}
		default:break;
	}
}

void AGV_Operation(void)
{

}

void reverseTask(void)
{
  static uint32_t holdtm = 0;
  uint32_t  tm_now;
  int       err;

  reverseMode = (uint16_t)dumper_get_state();

 	switch(reverseMode)
	{
		case REVERSE_MODE_HORIZONTAL:
#if 0
        if(!is_dumper_homing())
        {
          APP_TRACE("Dumper not in place. Reset dumper.\r\n");
          err = dumper_down(DEFAULT_DUMPER_W/2);
        }
        else 
#endif
        if(0x1 == nReverseFlag )
        {
          dumper_up(DEFAULT_DUMPER_W);
          holdtm = 0;
          nReverseFlag = 0;
        }
        break;

    case REVERSE_MODE_TOP:
        if(holdtm == 0)
        {
            APP_TRACE("Dumper up ok. hold_tm =%d\r\n", holdtm);
            holdtm = OSTimeGet();
        }
        else
        {
            tm_now = OSTimeGet();
            if(TIME_DIFFERENCE(tm_now, holdtm) > DEFAULT_DUMPER_HOLDTIME_MS)
            {
                uint8_t data = ACTION_UNLOAD;

                holdtm = 0;
                err = dumper_down(DEFAULT_DUMPER_W);
                if(err == -PERR_EFAULT)
                  dumper_stop();

                motionctrl_event_callback(MOTIONCTRL_EVENT_ACTION_OVER, 1, &data);
            }
        }
			  break;

		case REVERSE_MODE_REVERSEUP :
        if(dumper_angle()>=DEFAULT_DUMPER_REVERSE_ANGLE)
            dumper_stop();

        break;

		case REVERSE_MODE_REVERSEDN:
        if(is_dumper_homing())
            dumper_stop();
        break;
		default:
        break;
  }
}

static void error_event(int error)
{
    uint8_t err = error;

    motionctrl_event_callback(MOTIONCTRL_EVENT_ERROR, 1, &err);
}
#if 0
static void error_recovery(int error)
{
    uint8_t err = error;

    motionctrl_event_callback(MOTIONCTRL_EVENT_ERROR_RECOVERY, 1, &err);
}
#endif
/*--------------------end line------------------------*/
