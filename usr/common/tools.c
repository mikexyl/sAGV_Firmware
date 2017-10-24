#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

#include "app_cfg.h"
#include "common/PosixError.h"
#include "common/tools.h"

void reverse_copy(const void*src, void*dest, uint32_t len)
{
    int   i;
    for(i=0; i<len; i++)
        ((uint8_t*)dest)[i] = ((uint8_t*)src)[len-1-i];
}

int Uint2BCD(uint32_t uint, uint8_t *bcd, uint8_t size)
{
    int i;
    uint32_t data = uint;

    for(i=0; i<size; i+=1)
    {
        if(data == 0)
        {
            break;
        }

        bcd[size-1-i] = data%10;
        data = data/10;
        bcd[size-1-i] |= ((data%10)<<4);
        data = data/10;
    }

    for(; i<size; i++)
    {
        bcd[size-1-i] = 0;
    }

    if(data != 0)
        return -PERR_ENOMEM;
    else
        return PERR_EOK;
}

int BCD2Uint(uint32_t *uint, const uint8_t*bcd,uint8_t len)
{
   uint32_t result = 0;
   uint32_t bcd_value;
   int      i=0;

   for(i=0; i<len; i++)
   {
       if((bcd[i]>>4)>10)
           break;
       bcd_value = bcd[i]>>4;
       result = result*10+bcd_value;
       if((bcd[i] & 0x0F) > 10)
           break;
       bcd_value = (bcd[i] & 0xF);
   }

   *uint = result;

   return i; 
}

int BCD2Ascii(const uint8_t *bcd, uint16_t len, char* ascii)
{
    uint8_t data = 0;
    int i=0;

    for(i=0; i<len; i++)
    {
        data = (bcd[i] >> 4) & 0x0F;
        if(data > 0x09)
           return  2*i;

        ascii[2*i] = data+'0';
        
        data = bcd[i] & 0x0F;
        if(data > 0x09)
           return  2*i+1;
        
        ascii[2*i+1] = data+'0';
    }

    return 2*i;
}

int Ascii2BCD(const char* ascii, uint16_t len, uint8_t *bcd)
{
    uint8_t     data = 0;
    uint16_t    i = 0;
    uint16_t    count = 0;
    
    //ÆæÊý³¤¶È£¬×ó²¹'0'
    if((len & 0x01) == 0x01)
    {
        if(!isdigit(ascii[i]))
            return count;

        data = ascii[i] - '0';     
        i++;
        bcd[count++] = data;
    }

    while(i<len)
    {
        if((!isdigit(ascii[i])) || (!isdigit(ascii[i+1])))
            return count;
       
        data = (ascii[i] - '0')*10+(ascii[i+1] - '0');     

        i+=2;
        bcd[count++] = data;
    }

    return count;
}

uint32_t Ascii2Uint(const char* ascii, uint16_t *len)
{
    int i=0;
    uint32_t  uintData = 0;

    for(i=0; i<*len; i++)
    {
        if((ascii[i] < '0')||(ascii[i] > '9'))
            break;

       uintData = uintData*10 + (ascii[i]-'0');
    }

    *len = i;

    return uintData;
}

void memdump(const void*mem, uint32_t nbytes)
{
    int i;
    uint8_t *buf = (uint8_t*)mem;

    for(i=0; i<nbytes; i++)
    {
        APP_TRACE("%02x ", buf[i]);
        if((i & MEMDUMP_MAXCHARS_ONELINE) == MEMDUMP_MAXCHARS_ONELINE )
          APP_TRACE("\r\n");
    }

    if((i&MEMDUMP_MAXCHARS_ONELINE) != MEMDUMP_MAXCHARS_ONELINE )
      APP_TRACE("\r\n");
}


int strnicmp(const char* str1, const char* str2, uint32_t maxlen)
{
    int i;

    for(i=0; i<maxlen; i++)
    {
        if(toupper(str1[i]) != toupper(str2[i]))
          return (str1[i] > str2[i])? 1:-1;
        if(str1[i] == 0)
          return 0;
    }

    return 0;
}
