
#include "bsp.h"
#include "encoder.h"
#include "common/PosixError.h"

typedef struct
{
    uint32_t port;
    int      count;

    uint32_t lastval;
}encoder_t;

encoder_t encoder[2];

static encoder_t *get_encoder_from_port(uint8_t port)
{
    switch(port)
    {
        case 1:
            return &encoder[0];
        case 2:
            return &encoder[1];

        default:
            return NULL;
    }
}
#if 0
static void encoder_overflow_callback(uint8_t encoder_num,
                                      uint16_t cnt, uint8_t dir)
{
    encoder_t *dev;
    int16_t *intcnt=(int16_t*)&cnt;

    dev = get_encoder_from_port(encoder_num);
    if(dev == NULL)
      return;

#if 1
    if(dir == 1)
      dev->count += *intcnt;
    else
      dev->count += cnt;
#else
    dev->count += cnt;
#endif
}
#endif
int open_encoder(uint8_t port)
{
    encoder_t *dev;

    dev = get_encoder_from_port(port);
    if(dev == NULL)
      return (int)dev;

    dev->port = port;
    dev->lastval = 0;
    dev->count = 0;

    BSP_INITENCODER(port, NULL);
    //encoder_overflow_callback);

    return (int)dev;
}

int get_encoder_count(int hd)
{
    encoder_t *dev=(encoder_t*)hd;
    uint16_t    cnt;
    short       *intcnt=(short*)&cnt;
    uint8_t     dir;
    int         c;

    bsp_EncoderGetPulseNum(dev->port, &cnt, &dir);
    c = *intcnt+dev->count;

    //步进电机两个脉冲,编码器计一个数
    return c;
}

//Need the intrrupt to be disabled 
int get_count_and_clear(int hd)
{
    encoder_t   *dev=(encoder_t*)hd;
    uint16_t    cnt;
    short       *intcnt=(short*)&cnt;
    uint8_t     dir;
    int         c;

    bsp_EncoderGetPulseNum(dev->port, &cnt, &dir);
    bsp_EncoderSetZero(dev->port);
    c = *intcnt+dev->count;
    dev->count = 0;

    dev->lastval = 0;

    return c;
}

int clear_encoder(int hd)
{
    encoder_t *dev=(encoder_t*)hd;

    bsp_EncoderSetZero(dev->port);
    dev->lastval = 0;
    dev->count = 0;

    return 0;
}

int close_encoder(int hd)
{
    return clear_encoder(hd);
}
