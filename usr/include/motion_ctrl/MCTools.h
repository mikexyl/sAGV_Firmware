#ifndef _MCTOOLS_H_
#define _MCTOOLS_H_

#include "porting.h"
#include "user_function.h"

extern float32 GS_DEV;
extern float32 GS_DevFB;
extern float32 GS_Angle;
extern Uint16 AGV_Head_Dir;

#define ANGLE_TO_RADIAN(angle)  (((angle)*PI)/180)
#define RADIAN_TO_ANGLE(radian) ((radian)*180/PI)

#endif

/*-----------------end line----------------------*/



