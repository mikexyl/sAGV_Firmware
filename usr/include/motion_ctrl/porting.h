#ifndef __TYPES_H_20170322144427__
#define __TYPES_H_20170322144427__

#include <stdint.h>
#include "bsp.h"

typedef float   float32;
typedef double  float64;

typedef uint32_t    Uint32;
typedef uint16_t    Uint16;
typedef uint8_t     Uint8;

typedef int32_t     int32;
typedef int16_t     int16;
typedef int8_t      int8;

#if 0
extern Uint16  twoDMissingCnt;
extern  uint32_t    dummy;
#define LED0    dummy
#define LED1    dummy

#define DI_SERVOL_INVALID	0
#define DI_SERVOR_INVALID	0
#define DI_POWERUP_FEEDBACK 1
#define DI_CHARGE_FEEDBACK  0
#define DI_DN_LIMITPOS  	0
#define DI1                 dummy

#define   DO_rsvd0        		  dummy
#define   DO_rsvd1        		  dummy
#define   DO_rsvd2         		  dummy
#define   DO_rsvd3         		  dummy
#define   PWM_REVERSE		        dummy
#define   DO_rsvd4     			    dummy
#define   DO_CHARGE_CONTROL		  dummy
#define   DO_SERVO_POWERUP      dummy
#define   DO_SERVO3_OFFLINE     dummy
#define   DO_SERVOUP_DIR      	dummy
#define   DO_SERVOR_DIR      	  dummy
#define   DO_SERVOL_DIR         dummy
#define   DO_rsvd5  			      dummy
#define   DO_rsvd6  			      dummy
#define   DO_rsvd7  			      dummy
#define   DO_rsvd8			  	    dummy

#define StartCpuTimer2()  do{;} while(0)
#define DELAY_US(a)	    do{;} while(0)
void CAN_SendPacket(Uint16 uMailbox,Uint32 uDataH,Uint32 uDataL);
#endif

#define DINT    DISABLE_INT()
#define EINT    ENABLE_INT()


#endif
