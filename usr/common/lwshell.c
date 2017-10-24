

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <ucos_ii.h>

#include "common/list.h"
#include "common/PosixError.h"

#include "app_cfg.h"
#include "bsp.h"
#include "ff.h"
#include "motors.h"
#include "bms.h"
#include "configuration.h"

#include "common/lwshell.h"
#include "common/tools.h"

#include "rsi_data_types.h"
#include "rsi_common_apis.h"
#include "rsi_wlan_apis.h"
#include "rsi_socket.h"
#include "rsi_error.h"

/*
 * TODO:
 *  dynamically register & unregister command
 *  then lwshell will becomes a real common component
 */

#define     MAX_CONSOLE_DEVICE  4
#define     MAX_LINE_DATA       128
#define     MAX_PATH_LEN        256

#define     MAX_ARGS            8

struct cmd_handler
{
    const   char* cmd;
    const   char* comment;
    int     (*handler)(const char* args[], write_func_t output_func, uint32_t param);
};

static int ls(const char* args[], write_func_t output, uint32_t param);
static int rm(const char* args[], write_func_t output, uint32_t param);
static int rmdir(const char* args[], write_func_t output, uint32_t param);
static int cat(const char* args[], write_func_t output, uint32_t param);
static int cd(const char* args[], write_func_t output, uint32_t param);
static int pwd(const char* args[], write_func_t output, uint32_t param);
static int set(const char* args[], write_func_t output, uint32_t param);
static int cali_camera(const char* args[], write_func_t output, uint32_t param);
static int sysinfo(const char* args[], write_func_t output, uint32_t param);
static int sysstate(const char* args[], write_func_t output, uint32_t param);
static int reset(const char* args[], write_func_t output, uint32_t param);
static int usage(const char* args[], write_func_t output, uint32_t param);
static int motors(const char* args[], write_func_t output, uint32_t param);
static int dumper(const char* args[], write_func_t output, uint32_t param);
static int charge(const char* args[], write_func_t output, uint32_t param);
static int power(const char* args[], write_func_t output, uint32_t param);
static int help(const char* args[], write_func_t output, uint32_t param);
static int debug(const char* args[], write_func_t output, uint32_t param);

extern bool  spi_debug;

static struct cmd_handler builtin_cmds[] =
{
    {"cd",      "change directory",  &cd},
    {"ls",      "list contens in directory",  &ls},
    {"rm",      "remove a file",  &rm},
    {"rmdir",   "remove a directory",  &rmdir},
    {"cat",     "optputs the text file contents",  &cat},
    {"pwd",     "print working directory", &pwd},
    {"set",     "set [param [val]]:     set parameters", &set},
    {"usage",   "print the cpu usage and other statistic information", &usage},
    {"reset",   "reset the system", &reset},
    {"calibrate", "calibrate the camera", &cali_camera},
    {"motors",  "drive the two wheels to move.", &motors},
    {"dumper",  "drive the dumper up and (or) down", &dumper},
    {"charge",   "chargeing on/off", &charge},
    {"power",   "switch the motors power", &power},
    {"debug",   "switch the debug information", &debug},

    {"sys-state", "check the system information. for example: ip, version...", &sysstate},
    {"sys-info", "check the system information. for example: ip, version...", &sysinfo},


    {"help",    "print this information", &help}
};

static int motors(const char* args[], write_func_t output, uint32_t param)
{
    int32_t pps_l, pps_r, msecs;

    pps_l = atoi(args[0]);
    pps_r = atoi(args[1]);

    msecs = atoi(args[2]);

    if( ((pps_l == 0) && (pps_r == 0))
      || (msecs == 0) )
      return -PERR_EINVAL;

    motor_set_direction( (pps_l>0)? MOVE_FOREWARD_L:MOVE_ROLLBACK_L,
                         (pps_r>0)? MOVE_FOREWARD_R:MOVE_ROLLBACK_R);

    OSTimeDly(20);
    DriveWithPluse(abs(pps_l), abs(pps_r));

    OSTimeDly(MS_TO_TICKS(msecs));
    DriveWithPluse(0, 0);

    return 0;
}

static int dumper(const char* args[], write_func_t output, uint32_t param)
{
    int32_t pps, msecs;

    pps = atoi(args[0]);
    msecs = atoi(args[1]);

    if( (pps == 0) || (msecs == 0) )
        return -PERR_EINVAL;

    dumper_set_direction(pps>0? DUMPER_MOTOR_DIR_UP:DUMPER_MOTOR_DIR_DOWN);

    OSTimeDly(20);
    dumper_drive_with_pluse(abs(pps));

    OSTimeDly(MS_TO_TICKS(msecs));
    dumper_drive_with_pluse(0);

    return 0;
}

static int charge(const char* args[], write_func_t output, uint32_t param)
{
    if(strcmp(args[0], "on") == 0)
        bms_charge(1);
    else if(strcmp(args[0], "off") == 0)
        bms_charge(0);
    else
      return -PERR_EINVAL;

    return 0;
}

static int power(const char* args[], write_func_t output, uint32_t param)
{
    if(strcmp(args[0], "on") == 0)
    {
        MOTORS_ATTACH();
        //motors_power_onoff(1);
    }
    else if(strcmp(args[0], "off") == 0)
    {
        MOTORS_DETACH();
        //motors_power_onoff(0);
    }
    else
      return -PERR_EINVAL;

    return 0;
}

static int debug(const char* args[], write_func_t output, uint32_t param)
{
    if(strcmp(args[0], "on") == 0)
        spi_debug = true;
    else if(strcmp(args[0], "off") == 0)
        spi_debug = false;
    else
      return -PERR_EINVAL;

    return 0;
}

static int help(const char* args[], write_func_t output, uint32_t param)
{
    output(param, "builtin commands:\r\n", 0);

    for(int i=0; i<ARRAY_SIZE(builtin_cmds); i++)
    {
        output(param, "\t", 1);
        output(param, builtin_cmds[i].cmd, 0);
        output(param, ":", 1);
        output(param, builtin_cmds[i].comment, 0);
        output(param, "\r\n", 2);
    }

    return 0;
}

static int reset(const char* args[], write_func_t output, uint32_t param)
{
    output(param, "system reset...\r\n", 0);
    OSTimeDly(200);

    system_reset();

    return 0;
}

static int usage(const char* args[], write_func_t output, uint32_t param)
{
    char buf[32];
    int l;

    l = sprintf(buf, "cpu usage: %2d%%\r\n", OSCPUUsage);
    output(param, buf, l);

    return 0;
}

extern float detY0;
extern float detTheta0;

void Calibrate(void);

static int cali_camera(const char* args[], write_func_t output, uint32_t param)
{
    char    buf[32];
    int     l;

    Calibrate();

    sprintf(buf, "%f", detY0);
    write_config("detY", buf);
    sprintf(buf, "%f", detTheta0);
    write_config("detTheta", buf);

    if(output)
    {
        l = sprintf(buf, "\tdetY0 = %f", detY0);
        output(param, buf, l);

        l = sprintf(buf, "\tdetTheta0 = %f", detTheta0);
        output(param, buf, l);
    }

    return 0;
}

static int sysstate(const char* args[], write_func_t output, uint32_t param)
{
    const battery_info_t *bat_info;
    char    buf[64];
    int     l;

    bms_query();

    if(is_dumper_homing())
        output(param, "dumper in position.\r\n", 0);
    else
        output(param, "dumper not in position.\r\n", 0);

    bat_info = bms_get_battery_info();
    output(param, "battery info: ", 0);
    l = sprintf(buf, "vol: %f current: %.2f power: %d temperature: %.2f\r\n",
            bat_info->voltage, bat_info->current, bat_info->power, bat_info->temperature);
    output(param, buf, l);

    if(rsi_hal_intr_pin_status(1) > 0)
        output(param, "wifi intr: high\r\n", 0);
    else
        output(param, "wifi intr: low\r\n", 0);

    return 0;
}

static int sysinfo(const char* args[], write_func_t output, uint32_t param)
{
    rsi_rsp_wireless_info_t wlan_info;
    char buf[20];
    int l;

    output(param, "Software version: " SW_VERSION "\r\n", 0);

    l = sprintf(buf, "AGVID: %d\r\n", agv_id);
    output(param, buf, l);

    rsi_wlan_get(RSI_WLAN_INFO, (uint8_t*)&wlan_info, sizeof(wlan_info));

    l = sprintf(buf, "%d.%d.%d.%d\r\n",
              wlan_info.ipv4_address[0], wlan_info.ipv4_address[1],
              wlan_info.ipv4_address[2], wlan_info.ipv4_address[3]);

    output(param, "IP:", 0);
    output(param, buf, l);

    l = sprintf(buf, "%02x-%02x-%02x-%02x-%02x-%02x\r\n",
              wlan_info.mac_address[0], wlan_info.mac_address[1],
              wlan_info.mac_address[2], wlan_info.mac_address[3],
              wlan_info.mac_address[4], wlan_info.mac_address[5]);

    output(param, "MAC:", 0);
    output(param, buf, l);

    return 0;
}

struct output
{
    write_func_t func;
    uint32_t    dev;
};

static void show_config_item(const char* name, uint32_t type, const char* val, uint32_t param)
{
    struct output *out = (struct output*)param;

    switch(type)
    {
        case PARAM_TYPE_INT32:
          out->func(out->dev, "\tint ", 0);
          break;

        case PARAM_TYPE_FLOAT:
          out->func(out->dev, "\tdouble ", 0);
          break;

        case PARAM_TYPE_STR:
          out->func(out->dev, "\tstring ", 0);
          break;

        default:
          break;
    }

    out->func(out->dev, name, 0);
    out->func(out->dev, " = ", 3);
    out->func(out->dev, val, 0);
    out->func(out->dev, "\r\n", 2);
}

static int set(const char* args[], write_func_t output, uint32_t param)
{
    char    val[32];
    int     err;
    struct output out = {output, param};

    if( (args[0] == NULL) || (args[0][0] == 0) )
    {
        list_config(show_config_item, (uint32_t)&out);
        return 0;
    }
    else if( (args[1] == NULL) || (args[1][0] == 0) )
    {
        err = read_config(args[0], val, sizeof(val));
        if( (err != 0) && (output != NULL) )
        {
            output(param, "data not found\r\n", 0);
            return 0;
        }

        if(output != NULL)
        {
            output(param, args[0], 0);
            output(param, " = ", 3);
            output(param, val, 0);
            output(param, "\r\n", 2);
        }
    }
    else
    {
        err = write_config(args[0], args[1]);
        if( (err < 0) && (output != NULL) )
            output(param, "data not found\r\n", 0);
        else if ( (err == 1) && (output != NULL) )
            output(param, "to make it work, a reset is needed\r\n", 0);
    }

    return 0;
}

#define MAX_FNAME_LEN       63

FRESULT f_deldir(const TCHAR *path)
{
  FRESULT res;
  DIR   dir;     /* 文件夹对象 */ //36  bytes
  FILINFO fno;   /* 文件属性 */   //32  bytes
  TCHAR file[MAX_FNAME_LEN + 2] = {0};
#if _USE_LFN
  TCHAR lname[MAX_FNAME_LEN + 2] = {0};
#endif

#if _USE_LFN
  fno.lfsize = MAX_FNAME_LEN;
  fno.lfname = lname;    //必须赋初值
#endif
  //打开文件夹
  res = f_opendir(&dir, path);

  //持续读取文件夹内容
  while((res == FR_OK) && (FR_OK == f_readdir(&dir, &fno)))
  {
    //若是"."或".."文件夹，跳过
    if(0 == strlen(fno.fname))          break;      //若读到的文件名为空
    if(0 == strcmp(fno.fname, "."))     continue;   //若读到的文件名为当前文件夹
    if(0 == strcmp(fno.fname, ".."))    continue;   //若读到的文件名为上一级文件夹

    memset(file, 0, sizeof(file));
#if _USE_LFN
    if(fno.lfname[0] != 0)
      sprintf((char*)file, "%s/%s", path, (*fno.lfname) ? fno.lfname : fno.fname);
    else
#endif
      sprintf((char*)file, "%s/%s", path, fno.fname);

    if (fno.fattrib & AM_DIR)
    {//若是文件夹，递归删除
      res = f_deldir(file);
    }
    else
    {//若是文件，直接删除
      res = f_unlink(file);
    }
  }

  //删除本身
  if(res == FR_OK)    res = f_unlink(path);

  return res;
}

static int rmdir(const char* args[], write_func_t output, uint32_t param)
{
    f_deldir(args[0]);

    return 0;
}

static int rm(const char* args[], write_func_t output, uint32_t param)
{
    const char* path= args[0];

    f_unlink(path);

    return 0;
}

void print_stkusage(uint8_t task)
{
    OS_STK_DATA stkdata;

    OSTaskStkChk(OS_PRIO_SELF, &stkdata);

    //printf("task usage of task-%d\r\n", task);
    //printf("used:%d     free%d\r\n", stkdata.OSUsed, stkdata.OSFree);
}

static int ls(const char* args[], write_func_t output, uint32_t param)
{
    DIR dir;
    FRESULT ret;
    FILINFO finfo;
    char   lfn[MAX_FNAME_LEN+1];
    const char*path = args[0];
    char    buf[32];
    int l;

    //print_stkusage(OS_PRIO_SELF);

    finfo.lfname = lfn;
    finfo.lfsize = sizeof(lfn);

    if(path == NULL)
        path = ".";

    ret = f_opendir(&dir, path);
    if(ret != FR_OK)
    {
        if(output)
        {
            output(param, "open dir", 0);
            output(param, path, 0);
            output(param, "failed\n", 0);
        }

        return -PERR_ENODEV;
    }

    if(!output)
        return 0;

    while(1)
    {
        ret = f_readdir(&dir, &finfo);
        if( (ret != FR_OK) || (finfo.fname[0] == 0) )
            break;

        if(finfo.fattrib & AM_DIR)
        {
            output(param, "\td\t", 0);

            if(finfo.lfname[0] != 0)
              output(param, finfo.lfname, 0);
            else
              output(param, finfo.fname, 0);

            output(param, "\r\n", 2);
        }
        else
        {
            l = sprintf(buf, "\t%12d ", finfo.fsize);
            output(param, "\tf\t", 0);
            if(finfo.lfname[0] != 0)
                output(param, finfo.lfname, 0);
            else
                output(param, finfo.fname, 0);

            output(param, buf, l);
            output(param, "\r\n", 2);
        }
    }

    return 0;
}

static int cat(const char* args[], write_func_t output, uint32_t param)
{
    FIL         file;
    FRESULT     ret;
    char        buf[128];
    const char  *fname = args[0];
    uint32_t    start;
    int len;

    if(fname == NULL)
      return -PERR_EINVAL;

    if(output == NULL)
      return -PERR_EINVAL;
    ret = f_open(&file, fname, FA_READ|FA_OPEN_EXISTING);
    if(ret != FR_OK)
    {
        output(param, "Open file failed.\r\n", 0);
        return -PERR_ENODEV;
    }

    if(args[1] == NULL)
        start = 0;
    else
    {
        start = atoi(args[1]);
        len = sprintf(buf, "cat len: %d.\r\n", start);
        output(param, buf, len);
        if( (start != 0) && (start < f_size(&file)) )
            start = f_size(&file)-start;
    }
    
    if(start != 0)
        f_lseek(&file, start);

    len = sprintf(buf, "cat from: %d.\r\n", start);
    output(param, buf, len);

    output(param, "fname:  ", 0);
    output(param, fname, 0);
    len = sprintf(buf, "%d bytes.\r\n", f_size(&file));
    output(param, buf, len);
    while(!f_eof(&file))
    {
        if(f_gets(buf, sizeof(buf), &file) == NULL)
            break;

        output(param, buf, 0);
    }

    output(param, "\r\n", 2);
    f_close(&file);

    return 0;
}

static int cd(const char* args[], write_func_t output, uint32_t param)
{
    const char* path = args[0];
    FRESULT ret;

    ret = f_chdir(path);
    if(ret != FR_OK)
    {
        if(output)
          output(param, "Error: chdir failed.\r\n", 0);
    }
    return 0;
}

static int pwd(const char* args[], write_func_t output, uint32_t param)
{
    char    buf[128];

    if(output == NULL)
        return -PERR_EINVAL;

    f_getcwd(buf, sizeof(buf));

    output(param, buf, 0);
    output(param, "\r\n", 2);

    return 0;
}

static struct cmd_handler* get_cmd_handler(const char* cmd)
{
    int i;

    for(i=0; i<ARRAY_SIZE(builtin_cmds); i++)
    {
        if(strcmp(cmd, builtin_cmds[i].cmd) == 0)
        {
            return &builtin_cmds[i];
        }
    }

    return NULL;
}

static int exec_cmd_builtin(const char*cmd, const char *args[], write_func_t write_func, uint32_t dev)
{
    struct cmd_handler *hd;

    hd = get_cmd_handler(cmd);
    if(hd == NULL)
    {
        if(write_func)
            write_func(dev, "Invalid command.\r\n", 0);

        return -PERR_ENOTSUP;
    }

    hd->handler(args, write_func, dev);

    return 0;
}

static int exec(const char *args[], write_func_t write_func, uint32_t dev)
{
    const char    *cmd = args[0];

#if 0
    printf("exec: ");
    for(int i=0; (args[i] != NULL) && (i<MAX_ARGS); i++)
        printf("%s, ", args[i]);
    printf("\r\n");
#endif
    return exec_cmd_builtin(cmd, &args[1], write_func, dev);
}

#define IS_SEPARATOR(c) ((c==' ') || (c=='\t'))

static int  split_line(char *data, const char*args[], uint32_t max_num)
{
    int i=0;
    int cnt = 0;

    while(i<max_num)
    {
        if( (data[cnt] == '\n') || (data[cnt] == '\r') || (data[cnt] == '\0') )
            break;

        while(IS_SEPARATOR(data[cnt]))
          cnt++;

        if( (data[cnt] == '\n') || (data[cnt] == '\r') || (data[cnt] == '\0') )
          break;

        args[i++] = &data[cnt];

        while( !IS_SEPARATOR(data[cnt]) && (data[cnt]!='\n') && (data[cnt]!='\r') && (data[cnt] != '\0') )
            cnt++;

        if( (data[cnt] == '\n') || (data[cnt] == '\r') || (data[cnt] == '\0') )
          break;

        data[cnt++] = '\0';   //
    }

    if(i<max_num)
      args[i] = NULL;

    return i;
}

static void parse_and_exec(char *data, write_func_t write_func, uint32_t dev)
{
    const char *args[MAX_ARGS] = {NULL};
    int n;

    n = split_line(data, args, MAX_ARGS);

    if(n > 0)
      exec(args, write_func, dev);
}

static void print_prompt(write_func_t write_func, uint32_t dev)
{
    const char prompt[] = "> ";

    write_func(dev, prompt, sizeof(prompt)-1);
}

void console_run(read_ch_func_t readch_func, write_func_t write_func, uint32_t dev)
{
    char        data[128] = {0};
    uint32_t    cnt = 0;
    int         len;
    int         err = 0;
    bool        damaged = false;
    char        ch;
    const char crnl[2] = "\r\n";

    print_prompt(write_func, dev);

    do
    {
        len = readch_func(dev, &ch);
        if(len == 1)
        {
            if(ch == 8) //backspace
            {
                if(cnt >= 1)
                {
                  write_func(dev, &ch, 1);
                  ch = ' ';
                  write_func(dev, &ch, 1);
                  ch = 8;
                  write_func(dev, &ch, 1);
                  data[cnt--] = 0;
                }
                continue;
            }
            else if(!isprint(ch) && (ch != '\r') && (ch != '\n') )
            {
                //write_func(dev, &ch, 1);
                memset(data, 0, cnt);
                cnt = 0;
                write_func(dev, crnl, sizeof(crnl));
                print_prompt(write_func, dev);
                continue;
            }
            else if(ch == '\r')
            {
                write_func(dev, crnl, 2);
            }

            write_func(dev, &ch, 1);
            data[cnt++] = ch;

            if( (data[cnt-1] == '\r') || (data[cnt-1] == '\n') )
            {
                if( (!damaged) && (cnt > 1) )
                {
                    data[cnt-1] = '\0';
                    parse_and_exec(data, write_func, dev);
                }
                else
                    damaged = false;

                memset(data, 0, cnt);
                cnt = 0;

                //write_func(dev, crnl, sizeof(crnl));
                print_prompt(write_func, dev);
            }
            else if(cnt == sizeof(data))
            {
                damaged = true;
                cnt = 0;
            }
        }
        else
            OSTimeDly(MS_TO_TICKS(50));

        if(len < 0)
            err = len;

    }while(!err);
}
