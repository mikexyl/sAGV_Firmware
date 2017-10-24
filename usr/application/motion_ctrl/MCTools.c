#include <stdlib.h>
#include <math.h>

#include "MCTools.h"
#include "user_function.h"
#include "rAGVControl.h"
#include "app_cfg.h"
#include "bsp_pwm.h"
#include "Navigation.h"

Uint16 uQEP1 = 0;  //码盘1的计数值
Uint16 uQEP1Old = 0;   //上一次码盘1的读数值
int16 RealV1 = 0;  //轴1的实际速度
int32 RealS1 = 0;  //轴1的实际位置

Uint16 uQEP2 = 0;  //码盘2的计数值
Uint16 uQEP2Old = 0; //上一次码盘2的读数值
int16 RealV2 = 0; //轴2的实际速度
int32 RealS2 = 0; //轴2的实际位置 

Uint16 AGV_Head_Dir = X_FORWARD;  //车头方向，默认与X轴正向同向 

//导航系统
float32 GS_DEV = 0 ; //最终参与计算的导航系统的左右偏差，以mm为单位,以左为正
float32 GS_DevFB = 0; //前后偏差，以mm为单位，以前为正
float32 GS_Angle = 0; //最终参与计算的导航系统的角度偏差，以度为单位 ，以左为正 

/*--------------获取DI状态------------------------*/

//判断左伺服是否失效   返回1表示左伺服失效 返回0则表示有效
Uint16  ServoLeftInvaild(void)
{
    return  ( 0x0  );
}

//判断右伺服是否失效   返回1表示右伺服失效 返回0则表示有效
Uint16  ServoRightInvalid(void)
{
    return   ( 0x0 );
}


//判断充电接触器是否闭合，返回1表示闭合了,返回0表示未闭合
Uint16 ChargeFeedback(void)
{
	return   (0x0 );
}

// 判断动力是否上电，返回1表示上电，返回0表示断电
Uint16 ServoPowerFeedback(void)
{
	return (0x1 );
}

// 判断是否到达下限位 返回1表示到达下限位，返回0表示不处于下限位
Uint16 DnLimitPos(void)
{
	return (0x0 );
}

void VControl(float32 fLinearVel, float32 fDiffVel)
{
     fDiffVel = LIMIT(fDiffVel,35);

     if(abs(fDiffVel) > abs(fLinearVel))
       fDiffVel = (fDiffVel>0)?abs(fLinearVel):-(abs(fLinearVel));

	 //计算左右轮应该有的速度
	 fCurVelL = fLinearVel + fDiffVel;  //左轮
	 fCurVelR = fLinearVel - fDiffVel;  //右轮

	 fCurVelL = LIMIT(fCurVelL,2500);
	 fCurVelR = LIMIT(fCurVelR,2500);
}

/*********************************************
    旋转时速度控制
    根据导航读到的偏差 更新左右轮的期望速度
	wCurVel 当前的转动角速度 (以角度记，非弧度，长度以mm记)
	wRadio 输入旋转半径，默认为0(mm)
********************************************/
void WControl(float32 wCurVel, float32 wRadio)
{
     
	 //计算左右轮应该有的速度(mm)
	 
	 //fCurVelL = RatioW2V((wCurVel + fDiffVel));  //左轮
	 //fCurVelR = RatioW2V((wCurVel - fDiffVel));  //右轮

	 wRadio=LIMIT(wRadio,90);

	 fCurVelL=-1*(wCurVel*PI/180)*(wRadio-VEHICLERADIUS);
	 fCurVelR=(wCurVel*PI/180)*(wRadio+VEHICLERADIUS);
	 
	 fCurVelL = LIMIT(fCurVelL,650);
	 fCurVelR = LIMIT(fCurVelR,650);
} 


/*********************************************
    使原变量按照指定的步长向目标变量变化(浮点数)
	返回：0 表示还没变为目标值
	      1 数值变为了目标值
**********************************************/
Uint16 FApproach(float32* pFrom, float32 fTo,float32 fStep)
{
	if(* pFrom + fStep < fTo)
	{
	   * pFrom += fStep;
	   return 0x0;
	}
	else if(* pFrom - fStep > fTo)
	{
	   * pFrom -= fStep;
	   return 0x0;
	}
	else if(* pFrom != fTo)
	{
		* pFrom = fTo;
		return 0x0;
	}
	else
	{
	    return 0x1;
	}
} 

void UpdateAgvHeadDirToNew(void)
{
	if((twoD_Angle< 45.0001f) || ((twoD_Angle > 315.0001f)&&(twoD_Angle< 360.1f)))
	{
		AGV_Head_Dir = X_FORWARD;
	}
	else if((twoD_Angle > 45.0001f)&&(twoD_Angle< 135.0001f))
	{
		AGV_Head_Dir =  Y_BACKWARD ;
	}
	else if((twoD_Angle > 135.0001f)&&(twoD_Angle< 225.0001f))
	{
		AGV_Head_Dir = X_BACKWARD;
	}
	else if((twoD_Angle > 225.0001f)&&(twoD_Angle< 315.0001f))
	{
		AGV_Head_Dir = Y_FORWARD;
	}
}

/*------------------------获取二维码偏差信息-----------------------------*/
void Get_TwoDDev(void)
{
	if( AGV_Head_Dir == X_FORWARD)
	{
		GS_DEV = twoD_YP;
		GS_DevFB = twoD_XP;
		GS_Angle = ( twoD_Angle > 180.0f) ? (360.0f - twoD_Angle) : ( - twoD_Angle);
	}
	else if ( AGV_Head_Dir == X_BACKWARD)
	{
		GS_DEV =  - twoD_YP;
		GS_DevFB = - twoD_XP;
		GS_Angle = 180.0f - twoD_Angle ;
	}
	else if ( AGV_Head_Dir == Y_FORWARD)
	{
		GS_DEV = - twoD_XP;
		GS_DevFB = twoD_YP;
		GS_Angle = 270.0f - twoD_Angle ;			
	}
	else if ( AGV_Head_Dir ==  Y_BACKWARD)
	{
		GS_DEV = twoD_XP;
		GS_DevFB = -twoD_YP;
		GS_Angle = 90.0f - twoD_Angle;	
	}
} 


//float转换为int16 含四舍五入
int16  floatToInt16(float32 data)
{
    int16 reverData = 0;
	if(data>0)
	   reverData = (data * 10+5)/10;
	else if(data<0)
	   reverData = (data * 10-5)/10;
	else
	   reverData = 0;
	return reverData;
} 

/*---------------------大小端转换函数----------------------*/
Uint32 swapUint32(Uint32 value)  
{
    Uint32 data = 0;
	data =  ((value & 0x000000FF) << 24);
	data |= ((value & 0x0000FF00) << 8);
	data |= ((value & 0x00FF0000) >> 8);
	data |= ((value & 0xFF000000) >> 24);
	return data;
} 
/*----------------end line--------------------*/
