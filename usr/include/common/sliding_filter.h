#ifndef __SLIDING_FILTER_C_20170717145922__
#define __SLIDING_FILTER_C_20170717145922__

#include <stdint.h>

typedef struct
{
    uint32_t size;
    uint32_t cnt;

    float    *buffer;
}sliding_filter_t;

void sliding_filter_init(sliding_filter_t *filter, float *buffer);

void sliding_filter_input(sliding_filter_t *filter, float data);

float sliding_filter_output(sliding_filter_t *filter);

#endif
