#ifndef _TSHAPEVPLAN_H
#define _TSHAPEVPLAN_H 
#include "mc_config.h"

#define DELTA_T  		( AGCRUN_CYCLE * 0.001 )
#define EmergencyAccRatio 	(1.5)
#define PathError   	(0.003)
#define maximumAB(a,b) ((a)>(b)?(a):(b))
#define minimumAB(a,b) ((a)<(b)?(a):(b))

//extern float tShapeVTarget;
//extern float tShapeWTarget;

extern float SmoothVelocityPlanning(float pathLength,float aB_acc,float aB_dec,float Vcurr,float Vend,float Vmax,float * vShapeTarget);


#endif


/****************End line****************/ 
