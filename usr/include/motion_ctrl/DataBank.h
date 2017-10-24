#ifndef _DATABANK_H
#define _DATABANK_H

#include "porting.h"

extern void DB_WriteUint16(Uint16 uAddr, Uint16 data);
extern void DB_WriteUint32(Uint16 uAddr, Uint32 data);
extern Uint16 DB_ReadUint16(Uint16 uAddr);
extern Uint32 DB_ReadUint32(Uint16 uAddr);

extern float32 fTargetVel;
extern float32 wTarget;   

extern const float32 fAccUp; 
extern const float32 fAccDn;
extern float32 fVelRatio; 
extern float32 wVelRatio;

extern const float32  twoD_Distance;

extern const float32 wAccUp;  
extern const float32 wAccDn;
extern float32 angleVDown; 
extern float32 angleOpen;

extern float32 sStopDistance;
extern float32 angleStopDistance;

extern Uint16 pidFilterNum;

extern float32 vCtrlLowVelRatioL;	
extern float32 vCtrlHighVelRatioL;
extern float32 wCtrlCycleLeftRatioL;
extern float32 wCtrlCycleRightRatioL;

//系统参数区地址定义
#define ADDR_SYS_PARM_BASE    		(0x0000)
#define ADDR_VAR_SOPEN		  		(ADDR_SYS_PARM_BASE+0)  
#define ADDR_VAR_SSTOP_DISTANCE		(ADDR_SYS_PARM_BASE+4)
#define ADDR_VAR_SVDN_RATIO			(ADDR_SYS_PARM_BASE+8)
#define ADDR_ANGLESTOP_DISTANCE		(ADDR_SYS_PARM_BASE+12)
#define ADDR_ANGLEVDN_RATIO			(ADDR_SYS_PARM_BASE+16) 
#define ADDR_VCTRL_LOWVELRATIOL 	(ADDR_SYS_PARM_BASE+20)
#define ADDR_VCTRL_HIGHVELRATIOL 	(ADDR_SYS_PARM_BASE+24)
#define ADDR_WCTRL_CLEFTRATIOL 		(ADDR_SYS_PARM_BASE+28)
#define ADDR_WCTRL_CRIGHTRATIOL 	(ADDR_SYS_PARM_BASE+32)



#endif
/*-------------------end line-----------------------*/


