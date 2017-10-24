#ifndef _TRACINGZQ_H
#define _TRACINGZQ_H
#include <stdint.h>
#include "porting.h"

#include "mc_config.h"

#define M_PI 3.14159265358979323846	// åœ†å‘¨çŽ‡

#define MAX(x,y) (x>y?x:y)
#define MIN(x,y) (x<y?x:y)

#define B   ((VEHICLERADIUS*2.0f)/1000.0f)

#define dT (AGCRUN_CYCLE * 0.001)  //¿ØÖÆÖÜÆÚ50ms

extern float pY; 
extern float pTheta ; 
extern 	float  coderTheta;
extern void pInitzq(float pY0, float pTheta0); 
//extern float CalcPhfs(int32 coderVL, int32 coderVR); //for fuzzy compute 2017-4-7
extern float CalcPzq(int32 coderVL, int32 coderVR); //for fuzzy compute 2017-4-7

extern void GlideReset(void);

#endif
/***********************END Line************************/
