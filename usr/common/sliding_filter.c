#include <common/sliding_filter.h>
#include <string.h>
#include "app_cfg.h"

void sliding_filter_init(sliding_filter_t *filter, float *buffer)
{
    filter->cnt = 0;
    filter->buffer = buffer;

    memset(filter->buffer, 0, filter->size*sizeof(float));
}

void sliding_filter_input(sliding_filter_t *filter, float data)
{
    int i;

    APP_TRACE("sliding_filter_input: %f\r\n", data);

    if(filter->cnt == filter->size)
    {
        //TODO： 改为循环队列
      for(i = 0; i < filter->cnt-1; i++)
      {
        filter->buffer[i] = filter->buffer[i + 1];
      }

      --filter->cnt;
    }

    filter->buffer[filter->cnt++] = data;
}
#if 0
int sliding_filter_cnt(void)
{
    return filter->cnt;
}
#endif
float sliding_filter_output(sliding_filter_t *filter)
{
    int i;
    double tmp=0;

    for(i=0; i<filter->cnt; i++)
    {
        tmp += ((double)(filter->buffer[i]))/filter->cnt;
    }

    return (float)tmp;
}
