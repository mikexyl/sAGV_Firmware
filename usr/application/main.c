#include  <ucos_ii.h>
#include <stdio.h>
#include <stdint.h>

#include "app_cfg.h"

static OS_STK roottask_stk[ROOTTASK_STK_SIZE];
extern void root_task(void *p_arg);

int main(void)
{
	INT8U   err;

	OSInit();

	/* 创建一个启动任务（也就是主任务）。启动任务会创建所有的应用程序任务 */
	OSTaskCreateExt(root_task,	/* 启动任务函数指针 */
                    (void *)0,		/* 传递给任务的参数 */
                    (OS_STK *)&roottask_stk[ROOTTASK_STK_SIZE - 1], /* 指向任务栈栈顶的指针 */
                    ROOTTASK_PRIO,	/* 任务的优先级，必须唯一，数字越低优先级越高 */
                    ROOTTASK_PRIO,	/* 任务ID，一般和任务优先级相同 */
                    (OS_STK *)&roottask_stk[0],/* 指向任务栈栈底的指针。OS_STK_GROWTH 决定堆栈增长方向 */
                    ROOTTASK_STK_SIZE, /* 任务栈大小 */
                    (void *)0,	/* 一块用户内存区的指针，用于任务控制块TCB的扩展功能
                       （如任务切换时保存CPU浮点寄存器的数据）。一般不用，填0即可 */
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); /* 任务选项字 */

	/* 指定任务的名称，用于调试。这个函数是可选的 */
	OSTaskNameSet(ROOTTASK_PRIO, "root", &err);

	while(1)
	{
    OSStart();
	}
}
