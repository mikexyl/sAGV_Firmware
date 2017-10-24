#ifndef __MOTORS_H_20170331100925__
#define __MOTORS_H_20170331100925__
#include <stdint.h>
#include <stdbool.h>

#include "dio.h"
#include "mc_config.h"
#include "user_function.h"

#define USE_MOONS

//motors  pwm definition
enum motors_pwm
{
    MOTOR_LEFT_PWMNUM = 1,
    MOTOR_RIGHT_PWMNUM = 8,
    MOTOR_UNLOAD_PWMNUM = 2
};

#ifndef USE_MOONS

#define MOVE_FOREWARD_L 1
#define MOVE_ROLLBACK_L 0
#define MOVE_FOREWARD_R 0
#define MOVE_ROLLBACK_R 1

#define DUMPER_MOTOR_DIR_UP     0
#define DUMPER_MOTOR_DIR_DOWN   1

#define SERVO_MODE		2000.0f  //表示所选择的伺服模式为20000步/转

#else

#define MOVE_FOREWARD_L 0
#define MOVE_ROLLBACK_L 1
#define MOVE_FOREWARD_R 1
#define MOVE_ROLLBACK_R 0

#define DUMPER_MOTOR_DIR_UP     1
#define DUMPER_MOTOR_DIR_DOWN   0

#define SERVO_MODE		3200.0f  //表示所选择的伺服模式为20000步/转

#endif

typedef enum
{
    DUMPER_STATE_HORIZONTAL,
    DUMPER_STATE_MOVINGUP,
    DUMPER_STATE_TOP,
    DUMPER_STATE_MOVINGDOWN,

}E_DUMPER_STATE_T;

#define MOTORS_ATTACH()   bsp_DoSet(DO_MOTORS_DETACH, 1)
#define MOTORS_DETACH()   bsp_DoSet(DO_MOTORS_DETACH, 0)

#define LEGHTH_TO_PULSE(l)  (((l)*SERVO_MODE)/WHEEL_PERIMETER)
#define PULSE_TO_LENGTH(p)  (((p)*WHEEL_PERIMETER)/SERVO_MODE)

#define LEGHTH_TO_ANGLE(l)  (((l)*180)/(PI*VEHICLERADIUS))

//pps means pulse per second
#define VEL_TO_PPS(v)       LEGHTH_TO_PULSE(v)

void motors_init(void);

int motors_power_onoff(int on);

void motor_set_direction(int left, int right);

void DriveWithPluse(uint32_t pps_l, uint32_t pps_r);
void Drive(float vel_l, float vel_r);

void dumper_set_direction(int dir);
void dumper_drive_with_pluse(int pps);
bool is_dumper_homing(void);
float dumper_angle(void);
int dumper_up(float w);
int dumper_down(float w);
void dumper_stop(void);

E_DUMPER_STATE_T dumper_get_state(void);

#endif
