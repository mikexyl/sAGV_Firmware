#ifndef __TOOLS_H_20160218105048__ 
#define  __TOOLS_H_20160218105048__ 

#include <stdint.h>

#define FLOAT_DEFAULT_DELTA         0.001
#define MEMDUMP_MAXCHARS_ONELINE    31  //必须是二的整数次幂

#define ARRAY_SIZE(array)   (sizeof(array)/sizeof(array[0]))

#define ISEQUAL_WITHDELTA(a, b, d)   ( ((a-b)<delta) && ((b-a)<delta) )

#define FLOAT_ISEQUAL(a, b)     IsEqualWithDelta(a, b, FLOAT_DEFAULT_DELTA)

#define TIME_DIFFERENCE(newtm, oldtm)   ((newtm>=oldtm)?(newtm-oldtm):(~0-oldtm+newtm+1))

#define NTOHL_COPY(src, dest)  reverse_copy(src, dest, 4)
#define HTONL_COPY(src, dest)  reverse_copy(src, dest, 4)

extern void reverse_copy(const void*src, void*dest, uint32_t len);

extern int Uint2BCD(uint32_t uint, uint8_t *bcd, uint8_t size);

extern int BCD2Uint(uint32_t *uint, const uint8_t*bcd,uint8_t len);

extern int BCD2Ascii(const uint8_t *bcd, uint16_t len, char* ascii);

extern int Ascii2BCD(const char* ascii, uint16_t len, uint8_t *bcd);

extern uint32_t Ascii2Uint(const char* ascii, uint16_t *len);

extern void memdump(const void*mem, uint32_t nbytes);

extern int strnicmp(const char* str1, const char* str2, uint32_t maxlen);
#endif
