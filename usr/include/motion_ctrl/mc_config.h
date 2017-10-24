#ifndef __MC_CONFIG_H_20170613145331__
#define __MC_CONFIG_H_20170613145331__

#define AGCRUN_CYCLE    20

#ifndef PI
#define PI  3.14159265358979323846
#endif
//
#define     WHEEL_PERIMETER (2.0* PI * 45) //ËΩ¶ËΩÆÁöÑÂë®ÈïøÔºåÂçï‰Ωç‰∏∫mm
#define		  VEHICLERADIUS  170.0   //ËΩ¶ËΩÆÈó¥Ë∑ùÁöÑÂçäÂæÑ

#define     ENCODER_PLUSE_PER_ROUND 4000.0

#define     ENCODERCNT_TO_LENGTH(cnt)   (((cnt)*WHEEL_PERIMETER)/(ENCODER_PLUSE_PER_ROUND))
#define     LENGTH_TO_ENCODERCNT(l)     ((l)*ENCODER_PLUSE_PER_ROUND/WHEEL_PERIMETER)

#define     deltaT              (AGCRUN_CYCLE * 0.001f)  //øÿ÷∆÷‹∆⁄, µ•Œªs
#define     AXLE_LENGTH         (VEHICLERADIUS*2)   // ¬÷æ‡£¨µ•Œªmm

typedef struct _POSE{

float x;
float y;
float theta;

} POSE;

#endif
