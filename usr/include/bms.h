#ifndef __BMS_H_20170509154035__
#define __BMS_H_20170509154035__

#include <stdint.h>

#define BATTERY_STATE_NORMAL    0
#define BATTERY_STATE_CHARGING  1

typedef struct
{
    int     state;

    float   voltage;
    float   current;
    uint8_t power;          //0-100
    float   temperature;
}battery_info_t;

int bms_init(void);

int bms_run(void);

void bms_query(void);

const battery_info_t *bms_get_battery_info(void);

int bms_charge(int on);

#endif
