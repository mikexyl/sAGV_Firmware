#ifndef __INTF_LOCK_H_20170628154916__
#define __INTF_LOCK_H_20170628154916__

#ifdef  RSI_WITH_OS

#include <rsi_error.h>

rsi_error_t intf_lock_init(void);

rsi_error_t lock_intf(void);

rsi_error_t unlock_intf(void);

#else

#define intf_lock_init()      do{;} while(0)
#define lock_intf()           do{;} while(0)
#define unlock_intf()         do{;} while(0)

#endif

#endif
