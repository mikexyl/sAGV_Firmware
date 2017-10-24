#include "MotionEstimate.h"

float PxOld;
float PyOld;
float PthetaOld;


//位置和航向初始化
void MotionEstimateSet(float pX0, float pY0, float pTheta0)
{
	PxOld= pX0; 
	PyOld= pY0; 
	PthetaOld = pTheta0 * 0.0174533; //弧度
}


POSE MotionEstimate(int32 coderVL, int32 coderVR){
		float deltaSL=0.0f;
	float deltaSR=0.0f;

	float vL=0.0f;
	float vR=0.0f;

	float v=0.0f;
	float wMeasure=0.0f;
	float vMeasure=0.0f;
	
	float k1x=0.0f;
    float k1y=0.0f;
    float ksit=0.0f;

    float k2x=0.0f;
    float k2y=0.0f;

    float k4x=0.0f;
    float k4y=0.0f;

    float PxEx= 0.0f;
    float PyEx= 0.0f;
		
		float Px=0.0f,Py=0.0f,Ptheta=0.0f;
		
	deltaSL = ENCODERCNT_TO_LENGTH(coderVL);     // mm, 此处需谨慎修正，包括 正负号及数值大小
	deltaSR = ENCODERCNT_TO_LENGTH(coderVR);


	vL=deltaSL/deltaT;  //mm/s
	vR=deltaSR/deltaT;  //mm/s
	
	
	vMeasure = 0.5*(vL+vR);               // mm/s  线速度
	wMeasure = (vR-vL)/AXLE_LENGTH;       // rad/s  角速度
	
	
    //运动学积分
    k1x = deltaT*cos(PthetaFreeze)*vMeasure;
    k1y = deltaT*sin(PthetaFreeze)*vMeasure;
    ksit = deltaT*wMeasure;

    k2x = deltaT*cos(PthetaFreeze+0.5*ksit)*vMeasure;
    k2y = deltaT*sin(PthetaFreeze+0.5*ksit)*vMeasure;

    k4x = deltaT*cos(PthetaFreeze+ksit)*vMeasure;
    k4y = deltaT*sin(PthetaFreeze+ksit)*vMeasure;


    Px = PxOld+ (1./6.)*(k1x + 4*k2x + k4x);
    Py = PyOld+ (1./6.)*(k1y + 4*k2y + k4y);
    Ptheta = PthetaOld+ ksit;
	
	POSE newPose;
	newPose.x=Px;
	newPose.y=Py;
	newPose.theta=Ptheta;
	
	PxOld= Px; 
	PyOld= Py; 
	PthetaOld = Ptheta; //弧度
		
		
	return newPose;
	
}