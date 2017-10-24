#ifndef __LWSHELL_H_20170607144955__
#define __LWSHELL_H_20170607144955__

#include <stdint.h>

typedef int (*read_ch_func_t)(uint32_t param, char *data);
typedef int (*write_func_t)(uint32_t param, const char *data, uint32_t len);

extern void console_run(read_ch_func_t readch_func, write_func_t write_func, uint32_t dev);
#endif
