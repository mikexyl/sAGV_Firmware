#ifndef _ROTATING_H
#define _ROTATING_H

#define M_PINEW 3.1415926	// 圆周率
#define NUM_ERROR (1e-6)   //数值误差限

#include "porting.h"

// #4 车
#define CCD_X_ERROR (0.005) //CCD摄像头在车头方向的安装误差
#define CCD_Y_ERROR (0.0000) //CCD摄像头在Y方向的安装误差


extern float32 rotateRadius;

extern void RotateInit(float pX0, float pY0, float pTheta0, float pThetaEnd);
extern float32 CalcRotateP(float Px, float Py, float Ptheta);

extern float PxFreeze; 
extern float PyFreeze; 
extern float PthetaFreeze;

#endif
/***********************END Line************************/
