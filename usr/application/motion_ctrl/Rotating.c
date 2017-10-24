#include <math.h>

#include "Rotating.h"
#include "user_function.h"
#include "MCTools.h"
//#include "EXT_RAM.h"

float32 rotateRadius = 0.0f;

//车体实时位姿

float PxDesired= 0.0f; 
float PyDesired= 0.0f; 
float PthetaDesired= 0.0f; 

CFloatData unionCFRotatoOrignal;

float para_a=0.0f;
float para_b=0.0f;

float PxFreeze= 0.0f; 
float PyFreeze= 0.0f; 
float PthetaFreeze=0.0f;

//位置和航向初始化
void RotateInit(float pX0, float pY0, float pTheta0, float pThetaEnd)
{

        PxDesired=0;
        PyDesired=0;
        //PthetaDesired = pThetaEnd * 0.0174533; //弧度
        PthetaDesired = ANGLE_TO_RADIAN(pThetaEnd);


		PxFreeze= pX0; 
		PyFreeze= pY0; 
		//PthetaFreeze = pTheta0 * 0.0174533; //弧度
		PthetaFreeze = ANGLE_TO_RADIAN(pTheta0);
}

float32 lowBound(float32 fValue)
{

	float32 recData = 0.0f;

	if((fValue > 0)&&(fValue <NUM_ERROR))
		recData = NUM_ERROR;
	else if((fValue <0)&&(fValue > -NUM_ERROR))
		recData = -NUM_ERROR;	
	else 
		recData = fValue;

	return recData;
}

//每50ms更新一次车体位姿 (mm)
float32 CalcRotateP(float Px0, float Py0, float Ptheta0)
{
	
	// 曲率半径
    float32 RadioCurve=0.0f;

	float deltaY=0.0f;
	float deltaX=0.0f;
	
	float Kxa=0.0f;
	float Kxb=0.0f;
	
	float Kya=0.0f;
	float Kyb=0.0f;

	float Px = Px0; //detX0
	float Py = Py0; //detY0
	//float Ptheta = Ptheta0 * 0.0174533; 
	float Ptheta = ANGLE_TO_RADIAN(Ptheta0);

	float PthetaEnd=0.0f;

	float deltaY_CCD=0.0f;
    float deltaX_CCD=0.0f;

	float cosPthetaEnd=0.0f;
    float sinPthetaEnd=0.0f;

	float cosPthetaFreeze=0.0f;
    float sinPthetaFreeze=0.0f;

	float Offset=0.0f;

	if(fabs(PthetaFreeze-PthetaDesired)>M_PINEW)
	{
		 if(PthetaFreeze>PthetaDesired)
		 {
			PthetaEnd=PthetaFreeze-M_PINEW;  
		 }
		 else
		 {
			PthetaEnd=PthetaFreeze+M_PINEW;
		 }
	 }
	 else
	 {
		 PthetaEnd=PthetaDesired;
	 }


	
	//如果还有足够空间，则进行参数更新，否则固定曲线参数
	if(fabs(PthetaFreeze-PthetaEnd)>0.3491)
	{
	
        cosPthetaEnd=cos(PthetaEnd);
		sinPthetaEnd=sin(PthetaEnd);


		cosPthetaFreeze=cos(PthetaFreeze);
		sinPthetaFreeze=sin(PthetaFreeze);
		
		
	    // 将车体旋转至中心
        //deltaX_CCD=CCD_X_ERROR*cosPthetaFreeze-CCD_Y_ERROR*sinPthetaFreeze;
        //deltaY_CCD=CCD_X_ERROR*sinPthetaFreeze+CCD_Y_ERROR*cosPthetaFreeze;	
		

        //将CCD摄像头旋转至中心
        //deltaX_CCD=-CCD_X_ERROR*(cosPthetaEnd-cosPthetaFreeze)+CCD_Y_ERROR*(sinPthetaEnd-sinPthetaFreeze);
        //deltaY_CCD=-1*(CCD_X_ERROR*(sinPthetaEnd-sinPthetaFreeze)+CCD_Y_ERROR*(cosPthetaEnd-cosPthetaFreeze));

		deltaX_CCD=deltaX_CCD-cos(PthetaDesired)*Offset;
        deltaY_CCD=deltaY_CCD-sin(PthetaDesired)*Offset;

        deltaX=PxDesired-PxFreeze+deltaX_CCD;
        deltaY=PyDesired-PyFreeze+deltaY_CCD;

	
		Kxa=cosPthetaEnd-cosPthetaFreeze+PthetaEnd*sinPthetaEnd-PthetaFreeze*sinPthetaFreeze;
		Kxb=sinPthetaEnd-sinPthetaFreeze;
			
		Kya=sinPthetaEnd-sinPthetaFreeze-(PthetaEnd*cosPthetaEnd-PthetaFreeze*cosPthetaFreeze);
		Kyb=cosPthetaEnd-cosPthetaFreeze;

	    
	    Kxa = lowBound(Kxa);
		Kxb = lowBound(Kxb);
		Kya = lowBound(Kya);
		Kyb = lowBound(Kyb);
		
	
		para_a=(deltaY/Kyb+deltaX/Kxb)/(Kya/Kyb+Kxa/Kxb);
		para_b=(deltaX/Kxa-deltaY/Kya)/(Kxb/Kxa+Kyb/Kya);
	

	}

    PxFreeze=Px;
    PyFreeze=Py;
    PthetaFreeze=Ptheta;
	
	
	RadioCurve=para_a * Ptheta+para_b;


	return RadioCurve ; //毫米
    

} 

/*--------------------end line---------------------------------*/




