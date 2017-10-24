#include "DataBank.h"

float32 fTargetVel = 0.0f;
float32 wTarget = 0.0f;

#if 0
const float32 fAccUp = 500.0f;   //mm/s2
const float32 fAccDn = 500.0f;
#else
const float32 fAccUp = 1000.0f;   //mm/s2
const float32 fAccDn = 1000.0f;
#endif

float32 fVelRatio =16.6881f ; //车轮速度转化因子
float32 wVelRatio = 206.3172f ; 

#if 1
const float32  twoD_Distance = 550.0f;
#else
const float32  twoD_Distance = 500.0f;
#endif

#if 0
const float32 wAccUp = 100.0f;
const float32 wAccDn = 100.0f;
#else
const float32 wAccUp = 200.0f;  
const float32 wAccDn = 200.0f;
#endif

float32 angleVDown = 0.0f; 
float32 angleOpen  = 4.0f;

float32 sStopDistance = 7.0f;

float32 angleStopDistance = 0.8f;
/*-----------------end line--------------------*/
