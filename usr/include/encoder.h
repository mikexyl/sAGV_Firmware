#ifndef __ENCODER_H_20170405210121__
#define __ENCODER_H_20170405210121__
#include <stdint.h>

#include "bsp_encoder.h"

int open_encoder(uint8_t port);

int get_encoder_count(int hd);

int get_count_and_clear(int hd);

int clear_encoder(int hd);

int close_encoder(int hd);

#endif
