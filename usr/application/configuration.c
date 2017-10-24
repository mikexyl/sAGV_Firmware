#include <stdlib.h>
#include <stdint.h>

#include "app_cfg.h"
#include "common/tools.h"
#include "common/PosixError.h"
#include "minIni.h"
#include "configuration.h"


#define CONFIG_FILENAME     "/config.ini"

#define CONFIG_ATTR_RESET_ONCHANGE  1

typedef struct
{
    const char* name;
    const char* sec;
    paramtype_t type;

    void        *p_data;
    uint16_t    size;
    uint32_t    attr;
    //config_callbk_onchange_t    func_onchange;
} config_item_t;

extern float detY0;
extern float detTheta0;

extern char wifi_ssid[32];
extern char wifi_password[32];
extern char local_ip_str[16];
extern char remote_ip_str[16];
extern int32_t RSI_BAND;
extern int32_t RSI_SCAN_CHANNEL_BIT_MAP_2_4;

static config_item_t params[] =
{
    {"agvid", "DEVICE", PARAM_TYPE_INT32, &agv_id, sizeof(agv_id), CONFIG_ATTR_RESET_ONCHANGE}, // NULL},
    {"ssid", "WIFI", PARAM_TYPE_STR, wifi_ssid, sizeof(wifi_ssid), CONFIG_ATTR_RESET_ONCHANGE}, // NULL},
    {"password", "WIFI", PARAM_TYPE_STR, wifi_password, sizeof(wifi_password), CONFIG_ATTR_RESET_ONCHANGE},
    {"controler_ip", "COM", PARAM_TYPE_STR, remote_ip_str, sizeof(remote_ip_str), CONFIG_ATTR_RESET_ONCHANGE}, //NULL},
    {"local_port", "COM", PARAM_TYPE_INT32, &local_port, sizeof(local_port), CONFIG_ATTR_RESET_ONCHANGE}, //NULL},
    {"controller_port", "COM", PARAM_TYPE_INT32, &controller_port, sizeof(controller_port), CONFIG_ATTR_RESET_ONCHANGE}, //NULL},
#if 0
    {"local_ip", "COM", PARAM_TYPE_STR, local_ip_str, sizeof(local_ip_str), CONFIG_ATTR_RESET_ONCHANGE},// NULL},
#endif

    {"band", "WIFI", PARAM_TYPE_INT32, &RSI_BAND, sizeof(RSI_BAND), CONFIG_ATTR_RESET_ONCHANGE}, // 0:2.4G 1:5G
    {"channels", "WIFI", PARAM_TYPE_INT32, &RSI_SCAN_CHANNEL_BIT_MAP_2_4, sizeof(RSI_SCAN_CHANNEL_BIT_MAP_2_4), CONFIG_ATTR_RESET_ONCHANGE},//bit 0 as channel 1.
    {"detY", "MOTION_CTRL", PARAM_TYPE_FLOAT, &detY0, 0, 0},// NULL},
    {"detTheta", "MOTION_CTRL", PARAM_TYPE_FLOAT, &detTheta0, 0, 0},// NULL},
};

static OS_EVENT             *mutex=NULL;

int configuration_init(void)
{
    uint8_t err;

    mutex = OSMutexCreate(3, &err);
    if(mutex == NULL)
    {
        printf("configuration: create mutex failed with err = %d\r\n",
               err);
        return -PERR_ENOMEM;
    }


    return 0;
}

static void print_param(config_item_t *item)
{
    char str_val[32];

    if(item->p_data == NULL)
        return ;

    APP_TRACE("\t%s = ", item->name);

    switch(item->type)
    {
    case PARAM_TYPE_INT32:
      APP_TRACE("%d\r\n", *((int32_t *)item->p_data));
      break;

    case PARAM_TYPE_FLOAT:
      APP_TRACE("%.4f\r\n", *((float*)item->p_data));
      break;

    case PARAM_TYPE_STR:
      APP_TRACE("%s\r\n", (const char*)item->p_data);
      break;

    default:
      break;
    }
}

void load_configuration(void)
{
    int i;

    APP_TRACE("load configuration\r\n");

    for(i=0; i<ARRAY_SIZE(params); i++)
    {
        if(params[i].p_data == NULL)
            continue;

        switch(params[i].type)
        {
        case PARAM_TYPE_INT32:
            *((int32_t *)params[i].p_data) = ini_getl(params[i].sec, params[i].name, *((int32_t*)params[i].p_data), CONFIG_FILENAME);
            break;

        case PARAM_TYPE_FLOAT:
            *((float*)params[i].p_data) = ini_getf(params[i].sec, params[i].name, *((float*)params[i].p_data), CONFIG_FILENAME);
            break;

        case PARAM_TYPE_STR:
            ini_gets(params[i].sec, params[i].name, ((char*)params[i].p_data),
                     ((char*)params[i].p_data), params[i].size, CONFIG_FILENAME);
            break;

        default:
            break;
        }

        print_param(&params[i]);
    }
}

static config_item_t*get_config_item(const char*name)
{
    int i;

    for(i=0; i<ARRAY_SIZE(params); i++)
    {
        if(strcmp(params[i].name, name) == 0)
        {
            return &params[i];
        }
    }

    return NULL;
}

static void write_val(void *p_data, uint16_t size, const char*val, paramtype_t type)
{
    switch(type)
    {
        case PARAM_TYPE_INT32:
            *((int32_t*)p_data) = atoi(val);
            break;

        case PARAM_TYPE_FLOAT:
            *((float*)p_data) = atof(val);
            break;

        case PARAM_TYPE_STR:
        default:
            strncpy(p_data, val, size);
            break;
    }
}

int write_config(const char* name, const char* val)
{
    config_item_t *item;

    item = get_config_item(name);
    if(item == NULL)
    {
        return -PERR_ENODEV;
    }

    if( (item->sec != NULL) && (item->name != NULL) )
        ini_puts(item->sec, item->name, val, CONFIG_FILENAME);

    if(item->p_data != NULL)
        write_val(item->p_data, item->size, val, item->type);

    if(item->attr & CONFIG_ATTR_RESET_ONCHANGE)
        return 1;
    else
        return 0;
}

int read_config(const char* name, char*val, uint16_t size)
{
    config_item_t *item;

    item = get_config_item(name);
    if(item == NULL)
        return -PERR_ENODEV;

    if( (item->sec != NULL) && (item->name != NULL) )
        ini_gets(item->sec, item->name, "", val, size, CONFIG_FILENAME);
    else
        return -PERR_ENODEV;

    return 0;
}

void list_config(list_config_callbk_t func, uint32_t cbdata)
{
    int i;
    char val[64];
    config_item_t *item;

    for(i=0; i<ARRAY_SIZE(params); i++)
    {
        item = &params[i];
        if( (item->sec != NULL) && (item->name != NULL) )
        {
            ini_gets(item->sec, item->name, "", val, sizeof(val), CONFIG_FILENAME);
        }
        else
            continue;

        func(item->name, item->type, val, cbdata);
    }
}
