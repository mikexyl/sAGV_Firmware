
#include "common/PosixError.h"
#include "bsp_io.h"
#include "bsp_pwm.h"
#include "app_cfg.h"
#include "motors.h"
#include "common/tools.h"

typedef struct
{
    E_DUMPER_STATE_T state;
    float       position;    //angle
    float       w;          //角速度

    uint32_t    tm;         //start moveing time ms
} dumper_state_t;

static uint32_t left_pps;
static uint32_t right_pps;

static dumper_state_t dumper_st =
{
    DUMPER_STATE_HORIZONTAL,
    0,
    0,
    0,
};

#if 0
void motors_powerup(void)
{
    bsp_DoSet(DO_MOTORS_POWERUP, 0);
    OSTimeDly(1);
}
#endif

void motors_init(void)
{
    BSP_INITPWM(1); //pwm used for left wheel
    BSP_INITPWM(2); //pwm used for right wheel
    BSP_INITPWM(3); //pwm used for unload

    bsp_InitIO();

    bsp_DoSet(DO_MOTORS_POWERUP, 1);
    bsp_DoSet(DO_MOTORS_DETACH, 0);
    bsp_DoSet(DO_BATTERY_CHARGE, DO_BATTERY_CHARGE_OFF);

    OSTimeDly(1);
//    APP_TRACE("DI status: %d-%d-%d\r\n", bsp_DiGet(1), bsp_DiGet(2), bsp_DiGet(6));
}

int motors_power_onoff(int on)
{
    if(on)
      APP_TRACE("Motors power on.\r\n");
    //else
      //APP_TRACE("Motors power off.\r\n");

    bsp_DoSet(DO_MOTORS_POWERUP, (on)?0:1);
    OSTimeDly(1);
    return bsp_DiGet(DI_MOTORS_POWERUP_FB)^0x01;
}

static void motor_ctrl(int pwm, uint32_t pps)
{
    if(pps == 0)
        bsp_PwmStop(pwm);
    else
    {
        if(!bsp_PwmIsRunning(pwm))
          bsp_PwmStart(pwm);
        bsp_PwmFreDutySet(pwm, pps, 5000);
    }
}

void DriveWithPluse(uint32_t pps_l, uint32_t pps_r)
{
    /*
     * 关中断是为了避免由于中断或系统调度引起两个轮子不同步
     * 而导致偏航甚至脱轨
     * */
    DISABLE_INT();
    motor_ctrl(MOTOR_LEFT_PWMNUM,   pps_l);
    motor_ctrl(MOTOR_RIGHT_PWMNUM,  pps_r);
    ENABLE_INT();

    printf("pps_l = %d, pps_r = %d.\r\n", pps_l, pps_r);
}

void Drive(float vel_l, float vel_r)
{
    uint32_t pps_l, pps_r;

    pps_l = VEL_TO_PPS(vel_l)+0.5;  //四舍五入
    pps_r = VEL_TO_PPS(vel_r)+0.5;  //四舍五入

    if( (pps_l == left_pps) && (pps_r == right_pps) )
        return ;

    left_pps = pps_l;
    right_pps = pps_r;

    if( (pps_l == 0) && (pps_r == 0) )
        APP_TRACE("Stop move.\r\n");
    else
        APP_TRACE("Drive: left=%dpps, right=%dpps \r\n", pps_l, pps_r);

    /*
     * 关中断是为了避免由于中断或系统调度引起两个轮子不同步
     * 而导致偏航甚至脱轨
     * */
    DISABLE_INT();
    motor_ctrl(MOTOR_LEFT_PWMNUM,   pps_l);
    motor_ctrl(MOTOR_RIGHT_PWMNUM,  pps_r);
    ENABLE_INT();
}

//Set two wheels run direction
void motor_set_direction(int left, int right)
{
    APP_TRACE("Motors set direction: left=%d, right=%d \r\n", left, right);

    bsp_DoSet(DO_LEFTMOTOR_DIR, left);
    bsp_DoSet(DO_RIGHTMOTOR_DIR, right);

    OSTimeDly(1);
}

/*  * *******************************
 *  Interfaces for dumper
 * *****************************************/
#define REVERSE_SERVO_MODE      SERVO_MODE

#define DUMPER_REDUCTION_RATIO  30
#define DUMPER_W_PPS(w) (w*REVERSE_SERVO_MODE*30/360.0)

void dumper_set_direction(int dir)
{
    bsp_DoSet(DO_DUMPER_DIR, dir);
}

void dumper_drive_with_pluse(int pps)
{
    motor_ctrl(MOTOR_UNLOAD_PWMNUM, pps);
}

bool is_dumper_homing(void)
{
    return (bsp_DiGet(DI_DUMPER_INPOSITION) == 0);
}

float dumper_angle(void)
{
    float roll_angle;
    uint32_t tm;
    float   difftm;


    if(dumper_st.state == DUMPER_STATE_MOVINGUP)
    {
        tm = OSTimeGet();

//        APP_TRACE("Dumper angle: position = %f, tm =%d\r\n", 
 //             dumper_st.position, dumper_st.tm);
    
        //APP_TRACE("Curtm = %d\r\n", tm);

        difftm = TIME_DIFFERENCE(tm, dumper_st.tm);
        roll_angle = (difftm/OS_TICKS_PER_SEC)*dumper_st.w;
        return dumper_st.position + roll_angle;
    }
    if(dumper_st.state == DUMPER_STATE_MOVINGDOWN)
    {
        tm = OSTimeGet();

        difftm = TIME_DIFFERENCE(tm, dumper_st.tm);
        roll_angle = (difftm/OS_TICKS_PER_SEC)*dumper_st.w;
        return dumper_st.position - roll_angle;
    }
    else
        return dumper_st.position;
}

void dumper_stop(void)
{
    uint32_t roll_angle;
    uint32_t tm;
    float   difftm;

    bsp_PwmStop(MOTOR_UNLOAD_PWMNUM);

    if(dumper_st.state == DUMPER_STATE_MOVINGDOWN)
    {
        if(is_dumper_homing())
        {
          dumper_st.position = 0;
          dumper_st.state = DUMPER_STATE_HORIZONTAL;
        }
        else
        {
       // APP_TRACE("ERROR: dumper stoped halfway.\r\n");
            tm = OSTimeGet();

            difftm = TIME_DIFFERENCE(tm, dumper_st.tm);
            roll_angle = (difftm/OS_TICKS_PER_SEC)*dumper_st.w;
            dumper_st.position -= roll_angle;

            dumper_st.state = DUMPER_STATE_TOP;
        }
    }
    else if(dumper_st.state == DUMPER_STATE_MOVINGUP)
    {
        tm = OSTimeGet();
        difftm = TIME_DIFFERENCE(tm, dumper_st.tm);
        roll_angle = (difftm/OS_TICKS_PER_SEC)*dumper_st.w;
        dumper_st.position += roll_angle;

        dumper_st.state = DUMPER_STATE_TOP;
    }

    dumper_st.tm = 0;

    APP_TRACE("Dumper stop at position %f\r\n", dumper_st.position);
}

int dumper_up(float w)
{
    uint32_t    pps;

#if 0
    if(dumper_st.state != DUMPER_STATE_HORIZONTAL)
        return -PERR_EBUSY;
#endif

    pps = DUMPER_W_PPS(w);
    APP_TRACE("Dumper up. pwm_pps = %d\r\n", pps);

    //设置电机方向
    bsp_DoSet(DO_DUMPER_DIR, DUMPER_MOTOR_DIR_UP);
    OSTimeDly(1);
    dumper_st.w = w;
    dumper_st.state = DUMPER_STATE_MOVINGUP;

    DISABLE_INT();
    bsp_PwmStart(MOTOR_UNLOAD_PWMNUM);
    bsp_PwmFreDutySet(MOTOR_UNLOAD_PWMNUM, pps, 5000);
    dumper_st.tm = OSTimeGet();
    ENABLE_INT();

    return 0;
}

int dumper_down(float w)
{
    uint32_t    pps;

    if(dumper_st.state == DUMPER_STATE_MOVINGDOWN)
        return -PERR_EBUSY;

    if(is_dumper_homing())
    {
        dumper_st.state = DUMPER_STATE_HORIZONTAL;
        return -PERR_EFAULT;
    }

    pps = DUMPER_W_PPS(w);
    APP_TRACE("Dumper down. pwm_pps = %d\r\n", pps);

    //设置电机方向
    bsp_DoSet(DO_DUMPER_DIR, DUMPER_MOTOR_DIR_DOWN);
    OSTimeDly(1);
    dumper_st.w = w;
    dumper_st.state = DUMPER_STATE_MOVINGDOWN;

    DISABLE_INT();
    bsp_PwmStart(MOTOR_UNLOAD_PWMNUM);
    bsp_PwmFreDutySet(MOTOR_UNLOAD_PWMNUM, pps, 5000);
    dumper_st.tm = OSTimeGet();
    ENABLE_INT();

    return 0;
}

E_DUMPER_STATE_T dumper_get_state(void)
{
    return dumper_st.state;
}
