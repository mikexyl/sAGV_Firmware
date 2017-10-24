#ifndef __CONFIGURATION_H_20170612221102__
#define __CONFIGURATION_H_20170612221102__

#include <stdint.h>

typedef enum
{
    PARAM_TYPE_INT32,
    PARAM_TYPE_FLOAT,
    PARAM_TYPE_STR,
}paramtype_t;

typedef void(*list_config_callbk_t)(const char* name, uint32_t type, const char* val, uint32_t cbdata);

void load_configuration(void);

int write_config(const char* name, const char* val);

int read_config(const char* name, char*val, uint16_t size);

void list_config(list_config_callbk_t func, uint32_t cbdata);

#endif
