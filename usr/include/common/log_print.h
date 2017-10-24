#ifndef __LOG_PRINTF_H_20170401161018__
#define __LOG_PRINTF_H_20170401161018__

#include <stdint.h>

#define MAX_PRINT_LENGTH        512

#define LOGFILE_PATH            "/log"
#define LOGFILE_NAME_PREFIX     "sagv"

typedef int (*sendout_callback_t)(uint32_t hd, const char*buf,
                                  uint32_t len);

int log_init(void);

/*
 * Notice:
 *  Do not print a message more that MAX_PRINT_LENGTH bytes
 * */
int log_printf(const char* fmt, ...);

uint32_t log_register_outputcb(sendout_callback_t func, uint32_t param);
int log_unregister_outputcb(uint32_t callback_hd);

#endif
