#include <math.h>

static float angleMeasuredOld;
static float angleReal;
static int   count;

void ExtendAngleInit(float angleMeasured)
{
    angleMeasuredOld=angleMeasured;
    count=0;
    angleReal=angleMeasured;
}

// 角度制
float ExtendAngleExecute(float angleMeasured){


    float deltaAngle=angleMeasured-angleMeasuredOld;

    // 因为单次采样周期，转动角度不可能超过180，此数据意味着存在换向
    if(fabs(deltaAngle)>180){

        if (deltaAngle>0){

            count=count-1;

        }else{
            count=count+1;
        }

    }

    angleReal=count*360+angleMeasured;

    angleMeasuredOld=angleMeasured; // 更新


    return angleReal;

}

