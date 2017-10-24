#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "Tracingzq.h"
#include "fuzzyhuo.h"
#include "motion_ctrl.h"
#include "app_cfg.h"

//#define TRACEING_DEBUG
#define   LIMIT(x,lmt)      ((x > lmt)? lmt : ((x > -lmt)? x : (-lmt)))  //限制x在 -lmt ~ lmt //huo
//车体实时位姿
float Py = 0.0f; 
float Ptheta = 0.0f; 
float Px = 0.0f; 

float32 detY0 = 0;
float32 detTheta0 = 0;

static float32 GlideFilter(float32 input);  // float32 *glide_buff)

void pInitzq(float pY0, float pTheta0)
{
    Px = 0.0f;

//    if(Motionstyle == MOTIONSTATE_GOBACKWARD)
 //       Py = pY0 - detY0;
  //  else
        Py = pY0 + detY0;

		Ptheta = pTheta0 + detTheta0;
}

//float CalcPhfs(int32 coderVL, int32 coderVR) //huo for fuzzy compute 2017-4-7
float CalcPzq(int32 coderVL, int32 coderVR) //huo for fuzzy compute 2017-4-7
{
	float inputs[3]={0},outputs[2]={0}; //2017-4-25
	float deltaSL=0.0f;
	float deltaSR=0.0f;

	float vL=0.0f;
	float vR=0.0f;

	float v=0.0f;
	float wMeasure=0.0f;

	float k1x=0.0f;
    float k1y=0.0f;
    float ksit=0.0f;

    float k2x=0.0f;
    float k2y=0.0f;

    float k4x=0.0f;
    float k4y=0.0f;

    float PxEx= 0.0f;
    float PyEx= 0.0f;
    //float PthetaEx= 0.0f;

    deltaSL = ENCODERCNT_TO_LENGTH(abs(coderVL))/1000.0;
    deltaSR = ENCODERCNT_TO_LENGTH(abs(coderVR))/1000.0;

    vL=deltaSL/dT;  //m/s
    vR=deltaSR/dT;  //m/s

    v=0.5*(vL+vR);

#ifdef TRACEING_DEBUG
    APP_TRACE("AV: %f\r\n", v);
#endif

    wMeasure=(vR-vL)/B;

    //运动学积分
    k1x=dT*cos(Ptheta)*v;
    k1y=dT*sin(Ptheta)*v;
    ksit=dT*wMeasure;

    k2x=dT*cos(Ptheta+0.5*ksit)*v;
    k2y=dT*sin(Ptheta+0.5*ksit)*v;

    k4x=dT*cos(Ptheta+ksit)*v;
    k4y=dT*sin(Ptheta+ksit)*v;


    Px=Px+ (1./6.)*(k1x + 4*k2x + k4x);
    Py=Py+ (1./6.)*(k1y + 4*k2y + k4y);
    Ptheta=Ptheta+ ksit;


    // 估算当前的实际位置
    PxEx=Px;
    PyEx=Py;
    //PthetaEx=Ptheta;

	inputs[0]= PyEx*1000;  //mm
	inputs[1]= Ptheta*57.29578;   // degree
	inputs[2]= PxEx*1000; //mm

#ifdef  TRACEING_DEBUG
    APP_TRACE("input[0] = %f  input[1] = %f input[2] = %f  ",
            inputs[0], inputs[1], inputs[2]);
#endif
    fuzzy_step(inputs, outputs); //huo output the delta Z
#ifdef  TRACEING_DEBUG
    APP_TRACE("outputs[0] = %f\r\n", outputs[0]);
#endif
    outputs[0] = GlideFilter(outputs[0]);
#ifdef  TRACEING_DEBUG
    APP_TRACE("After filter: outputs[0] = %f\r\n", outputs[0]);
#endif

  return (-1.0*outputs[0]);
}

#define FILTER_N 1  //可调

static float32 glide_buff[FILTER_N]={0};
static float32 GlideFilter(float32 input)  // float32 *glide_buff)
{
  int32 i;

  double filter_sum = input;
 //glide_buff[FILTER_N-1] = input;

 for(i = 0; i < FILTER_N-1; i++) {
    glide_buff[i] = glide_buff[i + 1]; // 所有数据左移，低位拿掉
    filter_sum += glide_buff[i];
    }

 return (filter_sum / FILTER_N);
}

void GlideReset(void)
{
    memset(glide_buff, 0, sizeof(glide_buff));
}
/*--------------------end line---------------------------------*/




