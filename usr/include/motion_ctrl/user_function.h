/*
  user_function.h							  
  用于声明各种与外围模块相关的驱动函数	  

*/


#ifndef __USER_FUNCTION_H_
#define __USER_FUNCTION_H_

#include "porting.h"
#include "mc_config.h"

/*--------------------Tools.c var define-----------------------*/
typedef union
{
   float32 f;
   Uint32  u;
}CFloatData;

typedef union
{
   int32  s;
   Uint32 u;
}CInt32Data;

typedef union
{
   int16  s;
   Uint16 u;
}CInt16Data;

extern Uint16 uQEP1;  //码盘1的计数值
extern Uint16 uQEP1Old ;   //上一次码盘1的读数值
extern int16 RealV1;  //轴1的实际速度
extern int32 RealS1;  //轴1的实际位置

extern Uint16 uQEP2;  //码盘2的计数值
extern Uint16 uQEP2Old; //上一次码盘2的读数值
extern int16 RealV2; //轴2的实际速度
extern int32 RealS2; //轴2的实际位置 


extern int32 deltaVL;  
extern int32 deltaVR;  


extern Uint16 AGV_Head_Dir;


//导航系统
extern float32 GS_DEV; //以mm为单位
extern float32 GS_DevFB;
extern float32 GS_Angle; //以度为单位  

#define  X_FORWARD    0x00
#define  Y_FORWARD    0x01
#define  X_BACKWARD   0x02
#define  Y_BACKWARD   0x03 


#define   LIMIT(x,lmt)      ((x > lmt)? lmt : ((x > -lmt)? x : (-lmt)))  //限制x在 -lmt ~ lmt
#define   minusL(src,dest)  ((dest>=src) ? (dest-src) : (dest+4-src))
#define   minusR(src,dest)  ((dest>src) ? (4+src-dest) : (src-dest))
#define   absVar(x)			((x>0)?(x):(-x))

#define   radianToAngle(x)	(x * 180/PI)
#define		angleToRadian(x)	(x * PI/180)

#define    RatioW2V(x)    (x* 2* PI* VEHICLERADIUS/360)
#define    int16ToF(x)    (x * 1.0f) 


extern void  CalculateRealPosition(void);
extern void VControl(float32 fLinearVel, float32 fDiffVel);
extern void WControl(float32 wCurVel, float32 wRadio);
extern Uint16 FApproach(float32* pFrom, float32 fTo,float32 fStep);
extern void UpdateAgvHeadDirToNew(void);

extern Uint16 ServoLeftInvaild(void);
extern Uint16 ServoRightInvalid(void);
extern Uint16 ChargeFeedback(void);


extern void Get_TwoDDev(void);
extern int16  floatToInt16(float32 data);
extern Uint32 swapUint32(Uint32 value) ;

/*-------------------CanProcess.c var define------------------------------------*/
extern Uint16  iPath;		// 剩余路段数
extern float32 vPath;
extern Uint16  ethCommBreakFlag; 
extern Uint16  Motionstyle;

extern float32 fCurVel;  //当前线速度
extern float32 fCurVelL;  //当前左轮线速度
extern float32 fCurVelR;  //当前右轮线速度
extern float32 fDiffVel;
extern float32 sRemain; 

extern float32  angleRemain;  //主控下发的需要旋转的角度
extern float32  angleProcess; //转90之间经过的角度值
extern Uint16  AGV_Dir_Dest;  //下发旋转指令后的目标方向
extern float32  wCurrent;
extern float32  wCycle;
extern  Uint16 turnWholeCycleCnt;

extern float32 twoDAngleConver;  //将二维码传上来的角度值转换为逆时针的0~360度
extern float32 twoDAngleOld;     //记录上一时刻的二维码角度值
extern float32 twoDAngleNew ;	 //记录当前时刻的二维码角度值，为了进行跨X轴的角度变换
extern Uint16  acrossXAxisFlag ;     //跨x轴后此标志位为1，表明后续的值依序递增或者递减
extern float32 twoDAngleDest;

extern Uint16   encoderError;

extern float32  S_Vdown;

#define  ENCODER_ERROR  0x00455252  //0ERR  代表编码器出错  

extern void CanProc(void);
extern void MotionReport(void);
extern void TwoDCodeControl(Uint16 bOnOff);

/*-------------rAGVControl.c var define--------------------*/
extern Uint16 reverseMode;
extern Uint16 reverseCnt;
extern Uint16 nReverseFlag;


//速度相关
extern Uint16  uServoDelayClaspCount;// 抱闸计数器 

extern Uint16 	nAgvWorkMode;
extern float32 RealTwoDProgress;


extern void AGV_Run(void);

/*------------------isr.c var define-------------------------------*/
extern Uint16 twoDMissingCnt;



/*----------------------VPlan.c var define---------------------------------------*/

extern void VPlan(float32 sTotal,float32 accUp,float32 accDown,float32 vCurrent,float32 VPath);
extern void CyclePlan(float32 Angle,float32 wAccUp, float32 wAccDn,float32 wCurrent, float32 wAngle);

#endif

/*****************************End line*************************************/
