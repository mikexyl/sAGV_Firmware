#ifndef __DIO_H_20170512155823__
#define __DIO_H_20170512155823__
#include "bsp_io.h"

/*
 *  DIO definition
 * */
enum dout
{
    DO_LEFTMOTOR_DIR = 1,   //  1,
    DO_RIGHTMOTOR_DIR,      //
    DO_DUMPER_DIR,          //

    DO_MOTORS_DETACH,        //1: detach
    DO_MOTORS_POWERUP,      //0: powerup
    DO_BATTERY_CHARGE,      //0: charge
};

#define DO_BATTERY_CHARGE_ON    0
#define DO_BATTERY_CHARGE_OFF   1

enum din
{
    DI_MOTORS_POWERUP_FB=1, //0: powerup
    DI_BATTRRY_CHARGE_FB=2, //0: charging
    DI_DUMPER_INPOSITION=3, //0: horizontal

    DI_MOTORT_FAULT_LEFT= 5,
    DI_MOTORT_FAULT_RIGHT=6,
    DI_MOTORT_FAULT_DUMPER=7,
};

#endif
