#ifndef __BSP_H__
#define __BSP_H__
#include    <stddef.h>

#include    <ucos_ii.h>
#include    <cpu.h>
#include    <cpu_core.h>
#include    <stm32f4xx_gpio.h>

#define BSP_Printf  APP_TRACE

#define ENABLE_INT()        __set_PRIMASK(0)
#define DISABLE_INT()       __set_PRIMASK(1)

void bsp_Init(void);

void NVIC_Configuration(void);

CPU_INT32U  BSP_CPU_ClkFreq (void);

void BSP_Tick_Init (void);

#if ((APP_CFG_PROBE_OS_PLUGIN_EN == DEF_ENABLED) && \
     (OS_PROBE_HOOKS_EN          == 1))
void  OSProbe_TmrInit (void);
#endif

#if ((APP_CFG_PROBE_OS_PLUGIN_EN == DEF_ENABLED) && \
     (OS_PROBE_HOOKS_EN          == 1))
CPU_INT32U  OSProbe_TmrRd (void);
#endif

#if (CPU_CFG_TS_TMR_EN == DEF_ENABLED)
void  CPU_TS_TmrInit (void);
#endif

#if (CPU_CFG_TS_TMR_EN == DEF_ENABLED)
CPU_TS_TMR  CPU_TS_TmrRd (void);
#endif

void system_reset(void);

#endif
