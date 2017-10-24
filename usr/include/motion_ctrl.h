#ifndef __MOTION_CTRL_H_20170322204502__
#define __MOTION_CTRL_H_20170322204502__

#include <stdint.h>

#include "porting.h"
#define FLAG_ACTION_START   0
#define FLAG_ACTOIN_END     1

typedef struct
{
    uint32_t point;
    uint32_t x_deviation;
    uint32_t y_deviation;
}agv_positon_t;

typedef struct
{
    uint32_t direction;
    uint32_t deviation; //
}agv_direction_t;

typedef enum
{
    MOTIONSTATE_ONSTOPING = 0,
    MOTIONSTATE_GOSTRAIGHT = 1,
    MOTIONSTATE_TRUNLEFT =2,
    MOTIONSTATE_TRUNRIGHT =3,
    MOTIONSTATE_UNLOADING =4,
    MOTIONSTATE_GOBACKWARD=5,
} motion_state_t;

typedef struct
{
    agv_positon_t   position;
    agv_direction_t direction;

    uint32_t        speed;
    motion_state_t  state;
    uint32_t        action;
} motion_info_t ;

typedef enum
{
    ACTION_GOSTRAIGHT = 1,
    ACTION_TRUNLEFT   = 2,
    ACTION_TRUNRIGHT  = 3,

    ACTION_CHARGE     = 6,
    ACTION_STOP_CHARGE = 7,

    ACTION_STOP       = 8,
    ACTION_UNLOAD     ,
    ACTION_GOBACKWARD ,

    ACTION_SYS_RESET = 21,
} agv_action_t;

/*----------------action mode define---------------------------*/
#define ACTION_MODE_STOP		    MOTIONSTATE_ONSTOPING
#define ACTION_MODE_GOAHEAD		  MOTIONSTATE_GOSTRAIGHT
#define ACTION_MODE_GOBACK		  MOTIONSTATE_GOBACKWARD
#define ACTION_MODE_TURNLEFT	  MOTIONSTATE_TRUNLEFT
#define ACTION_MODE_TURNRIGHT	  MOTIONSTATE_TRUNRIGHT

typedef enum
{
    MOTIONCTRL_EVENT_ACTION_START=0,
    MOTIONCTRL_EVENT_ACTION_OVER=1,

    MOTIONCTRL_EVENT_POINT_ARRIVAL=2,   //经过二维码
    MOTIONCTRL_EVENT_ERROR,
    MOTIONCTRL_EVENT_ERROR_RECOVERY,
}motion_event_t;

#define     FLAG_ACTION_START   0
#define     FLAG_ACTION_OVER    1

extern int encoder_l;
extern int encoder_r;
extern Uint16  Motionstyle;

typedef void (*motevent_cb_t)(motion_event_t evt, uint32_t len, const uint8_t *params);

void motionctrl_run(motevent_cb_t event_callback);

int  motionctrl_cmd(agv_action_t cmd, uint32_t len, const uint8_t *param);

void get_motion_state(motion_info_t *state);

extern void  CalculateRealPosition(void);
extern void VControl(float32 fLinearVel, float32 fDiffVel);
extern void WControl(float32 wCurVel, float32 wRadio);
extern void Drive(float vel_l, float vel_r);
extern Uint16 FApproach(float32* pFrom, float32 fTo,float32 fStep);
extern void UpdateAgvHeadDirToNew(void);

extern Uint16 ServoLeftInvaild(void);
extern Uint16 ServoRightInvalid(void);
extern Uint16 ChargeFeedback(void);

extern void Get_TwoDDev(void);
extern int16  floatToInt16(float32 data);
extern Uint32 swapUint32(Uint32 value) ;

int moveTest(void);
#endif
