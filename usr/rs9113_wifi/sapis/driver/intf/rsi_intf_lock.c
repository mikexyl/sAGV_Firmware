#ifdef  RSI_WITH_OS
#include <stdint.h>
#include <rsi_error.h>
#include <rsi_os.h>

static rsi_mutex_handle_t mutex;

rsi_error_t intf_lock_init(void)
{
    return rsi_mutex_create(&mutex);
}

rsi_error_t lock_intf(void)
{
    return rsi_mutex_lock(&mutex);
}

rsi_error_t unlock_intf(void)
{
    return rsi_mutex_unlock(&mutex);
}

#endif
