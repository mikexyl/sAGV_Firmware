#ifndef __NAVIGATION_H_20170401111742__
#define __NAVIGATION_H_20170401111742__

#include "porting.h"

extern float32  twoD_YP ;      //Y轴上的偏移，单位为mm
extern float32  twoD_XP;      //X轴上的偏移，单位为mm
extern float32  twoD_Angle ;   //角度偏移，为二维码X轴正向与二维码灯正向的顺时针夹角，总范围为0~359
extern Uint16 NewTag;
extern Uint32 twoD_TagCode ; //标签号码
extern Uint32 twoD_TagCodeOld ; //上一次的标签号码


int NavigationInit(void);

int NavigationOnOff(int on);

#endif
