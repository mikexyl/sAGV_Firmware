#ifndef _MOTIONESTIMATE_H
#define _MOTIONESTIMATE_H

#include "mc_config.h"
#include "Rotating.h"
#define int32 int

extern void MotionEstimateSet(float pX0, float pY0, float pTheta0);
extern POSE MotionEstimate(int32 coderVL, int32 coderVR);


#endif
