#include <math.h>

#include "user_function.h"
#include "DataBank.h"
#include "VPlan.h"

//#include "DataBank.h"



/**********************************************  
   梯形速度规划算法
   sTotal   为需要走完的距离
   accUp    为加速度
   accDown  为减速度
   vPath    为主控下发的路段最高限速
   vCurrent 为读码盘所获取的当前速度
**********************************************/
void VPlan(float32 sTotal,float32 accUp,float32 accDown,float32 vCurrent,float32 VPath)
{
	float32 s10,s11,s3;
	
	s10 = ((VPath-vCurrent)* (VPath-vCurrent))/ (2 * accUp)  ;
  s11 =  vCurrent * (VPath-vCurrent)/ accUp ;  
	s3  =  VPath * VPath /(2 * accDown); 

	if(sTotal>0.0f)
	{
		if(s10 + s11+s3 < sTotal)  //此时有匀速段，速度最大值为主控下发的
		{

			fTargetVel = VPath;
		}
		else  //此时无匀速段，需求出加速的最大速度
		{
			fTargetVel = sqrt((( 2 * sTotal * accUp * accDown) + (accDown * vCurrent * vCurrent ))/ (accUp + accDown));
		}
	}
}

void CyclePlan(float32 Angle,float32 wAccUp, float32 wAccDn,float32 wCurrent, float32 wAngle)
{
	float32 angle_10,angle_11,angle_3;


	angle_10 = ((wAngle - wCurrent) * (wAngle - wCurrent)) / (2 * wAccUp);
	angle_11 = wCurrent * (wAngle - wCurrent) / wAccUp;
	angle_3  = wAngle * wAngle /  ( 2 * wAccDn );

	if( angle_10 + angle_11 + angle_3 < Angle)
	{

		wTarget = wAngle;
	}
	else
	{
		wTarget = sqrt((( 2 * Angle * wAccUp * wAccDn) + (wAccDn * wCurrent * wCurrent ))/ (wAccUp + wAccDn) );
	}

}
/*--------------------------end line-------------------*/


