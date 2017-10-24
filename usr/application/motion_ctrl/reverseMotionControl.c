#include "ucos_ii.h"

#include "app_cfg.h"
#include "reverseMotionControl.h"
#include "common/tools.h"
#include "DataBank.h"
#include "rAGVControl.h"
#include  "user_function.h"
#include "motors.h"
#include "Navigation.h"

void agv_motion_ctrl(void)
{
  uint32_t  ticks_s, ticks_e, ticks_d;

    APP_TRACE("Agv motion ctrl run.\r\n");
    while(1)
    {
        ticks_s = OSTimeGet();

        AGV_Run();
        reverseTask();

        ticks_e = OSTimeGet();

        ticks_d = TIME_DIFFERENCE(ticks_e, ticks_s);

        if(ticks_d < MS_TO_TICKS(AGCRUN_CYCLE))
          OSTimeDlyHMSM(0,0,0,AGCRUN_CYCLE-TICKS_TO_MS(ticks_d));
        else
          APP_TRACE("ERROR: The bigloop overtime.\r\n");
    }
}

void InitAgvHeadDir(void)
{
//    while(NewTag != 1)
//    {
//        OSTimeDlyHMSM(0,0,0,50);
//    }

    NewTag = 0;

    UpdateAgvHeadDirToNew();
    Get_TwoDDev();

    APP_TRACE("Initialize position ok. at point:%d\r\n", twoD_TagCode);
    APP_TRACE("x = %f, y = %f, angle = %f\r\n", twoD_XP, twoD_YP, twoD_Angle);
    APP_TRACE("FB_Dev = %f, DEV = %f, GS_Angle = %f\r\n",
              GS_DevFB, GS_DEV, GS_Angle);
}
/*--------------------end line --------------------------*/
