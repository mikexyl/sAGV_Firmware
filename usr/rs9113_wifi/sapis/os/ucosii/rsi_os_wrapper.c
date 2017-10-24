/**
 * @file    rsi_os_wrapper.c
 * @version 0.1
 * @date    15 Aug,2015
 *
 *  Copyright(C) Redpine Signals 2015
 *  All rights reserved by Redpine Signals.
 *
 *  @section License
 *  This program should be used on your own responsibility.
 *  Redpine Signals assumes no responsibility for any losses
 *  incurred by customers or third parties arising from the use of this file.
 *
 *  @brief : This file contain os handlers for FREE RTOS platform.
 *
 *  @section Description  This file contains function handlers for FREE RTOS Platform.
 *  This is sample os porting file which is ported for Free RTOS.User should port this file
 *  if OS is other than Free RTOS
 *
 *
 */

#ifndef RSI_FREE_RTOS
#define RSI_FREE_RTOS
/**
 * Include files
 * */
#include <rsi_driver.h>
#include <ucos_ii.h>

#include "cpu.h"
#include "os_cpu.h"

#include "app_cfg.h"

/*==============================================*/
/**
 * @fn          rsi_reg_flags_t rsi_critical_section_entry()
 * @brief       This API's to enter critical section
 * @param[in]   none,
 * @return      flags, interrupt status before entering critical section
 *
 *
 * @section description
 * This API disable interrupt to enter crtical section.
 *
 *
 */
rsi_reg_flags_t rsi_critical_section_entry()
{
	//! hold interrupt status before entering critical section
	//! disable interrupts
	rsi_reg_flags_t xflags;
  uint32_t  cpu_sr;
	//! hold interrupt status before entering critical section
	//! disable interrupts
	xflags = 0;

  OS_ENTER_CRITICAL();

  xflags = cpu_sr;

	//! return stored interrupt status
	return (xflags);
}



/*==============================================*/
/**
 * @fn         rsi_critical_section_exit(rsi_reg_flags_t xflags)
 * @brief      This API to exit critical section
 * @param[in]  xflags, interrupt status to restore interrupt on exit from critical section
 * @return     None
 *
 *
 * @section description
 * This API to exit/restore critical section.
 *
 *
 */

void  rsi_critical_section_exit(rsi_reg_flags_t xflags)
{
  uint32_t cpu_sr = xflags;

	//! restore interrupts while exiting critical section
    OS_EXIT_CRITICAL();
}



/*==============================================*/
/**
 * @fn           rsi_error_t rsi_mutex_create(rsi_mutex_handle_t *mutex)
 * @brief        This API is creates the mutex 
 * @param[in]    mutex handle pointer  
 * @param[out]   Mutex created   
 * @return       0 = success 
 *              <0 = failure
 * @section description
 * This function is OS Abstraction layer API which creates the mutex 
 *
 *
 */

rsi_error_t rsi_mutex_create(rsi_mutex_handle_t *mutex)
{
  uint8_t err;

  *mutex = (rsi_mutex_handle_t)OSMutexCreate(OS_PRIO_MUTEX_CEIL_DIS, &err);

	if(*mutex == 0)
	{
    	APP_TRACE("rsi create mutex failed.\r\n");
		return  RSI_ERROR_IN_OS_OPERATION;
	}

	return RSI_ERROR_NONE;
}



/*==============================================*/
/**
 * @fn           rsi_error_t rsi_mutex_lock(volatile rsi_mutex_handle_t *mutex)
 * @brief        This API is takes the mutex 
 * @param[in]    mutex handle pointer  
 * @return       0 = success 
 *              <0 = failure
 * @section description
 * This function is OS Abstraction layer API which takes the mutex
 *
 *
 */
rsi_error_t rsi_mutex_lock(volatile rsi_mutex_handle_t *mutex)
{
    uint8_t err;

    if(mutex == NULL)
        return RSI_ERROR_INVALID_PARAM;

    OSMutexPend((OS_EVENT*)(*mutex), 0, &err);
    if(err != OS_ERR_NONE)
    {
        APP_TRACE("rsi mutex lock failed.\r\n");
        return RSI_ERROR_IN_OS_OPERATION;
    }
    else
		    return RSI_ERROR_NONE;
}


/*==============================================*/
/**
 * @fn           rsi_error_t rsi_mutex_unlock(volatile rsi_mutex_handle_t *mutex)
 * @brief        This API is gives the mutex 
 * @param[in]    mutex handle pointer  
 * @return       0 = success 
 *              <0 = failure
 * @section description
 * This function is OS Abstraction layer API which gives the mutex 
 *
 */
rsi_error_t rsi_mutex_unlock(volatile rsi_mutex_handle_t *mutex)
{
    uint8_t err;
    if(mutex == NULL)
    {
      return RSI_ERROR_INVALID_PARAM;
    }

    err = OSMutexPost((OS_EVENT*)(*mutex));
    if(err != OS_ERR_NONE)
        return RSI_ERROR_IN_OS_OPERATION;
    else
        return RSI_ERROR_NONE;
}



/*==============================================*/
/**
 * @fn           rsi_error_t rsi_mutex_destory(rsi_mutex_handle_t *mutex)
 * @brief        This API is destroyes the mutex 
 * @param[in]    mutex handle pointer  
 * @return       0 = success 
 *              <0 = failure
 * @section description
 * This function is OS Abstraction layer API which destroy/delete the mutex 
 *
 */
rsi_error_t rsi_mutex_destory(rsi_mutex_handle_t *mutex)
{
  uint8_t err;
	if(mutex == NULL)
	{
		return RSI_ERROR_INVALID_PARAM;
	}

  OSMutexDel((OS_EVENT*)(*mutex), OS_DEL_ALWAYS, &err);

	return RSI_ERROR_NONE;
}



/*==============================================*/
/**
 * @fn           rsi_error_t rsi_semaphore_create(rsi_semaphore_handle_t *semaphore,uint32_t count)
 * @brief        This API is creates the semaphore instance 
 * @param[in]    Semaphore handle pointer  
 * @param[in]    resource count
 * @param[out]   Semaphore handler created   
 * @return       0 = success 
 *              <0 = failure
 * @section description
 * This function is OS Abstraction layer API which creates the semaphore 
 *
 */
rsi_error_t rsi_semaphore_create(rsi_semaphore_handle_t *semaphore,uint32_t count)
{
  *semaphore = (rsi_semaphore_handle_t)OSSemCreate(count);

	if(*semaphore == NULL)
	{
    	APP_TRACE("rsi create semaphore failed.\r\n");
		return  RSI_ERROR_IN_OS_OPERATION;
	}

	return RSI_ERROR_NONE;
}

/*==============================================*/
/**
 * @fn           rsi_error_t rsi_semaphore_destroy(rsi_semaphore_handle_t *semaphore)
 * @brief        This API is destroys the semaphore instance
 * @param[in]    Semaphore handle pointer  
 * @return       0 = success 
 *              <0 = failure
 * @section description
 * This function is OS Abstraction layer API which destroys the semaphore 
 *
 */
rsi_error_t rsi_semaphore_destroy(rsi_semaphore_handle_t *semaphore)
{
    uint8_t err;
	if(semaphore == NULL)
	{
		return RSI_ERROR_INVALID_PARAM;
	}

  OSSemDel((OS_EVENT*)(*semaphore), OS_DEL_ALWAYS, &err);

  if(err != OS_ERR_NONE)
    return RSI_ERROR_IN_OS_OPERATION;
  else
    return RSI_ERROR_NONE;
}



/*==============================================*/
/**
 * @fn          rsi_error_t rsi_semaphore_wait(rsi_semaphore_handle_t *semaphore, uint32_t timeout_ms ) 
 * @brief       This API is used by wireless library to acquire or wait for semaphore.
 * @param[in]   Semaphore handle pointer  
 * @param[in]   Maximum time to wait to acquire semaphore. If timeout_ms is 0 then wait
                till acquire semaphore.
 * @return      0 = Success
 *              <0 = failure
 * @section description
 * This API is used by Wireless Library to acquire or wait for semaphore.
 *
 */

rsi_error_t rsi_semaphore_wait(rsi_semaphore_handle_t *semaphore, uint32_t timeout_ms )
{
    uint8_t err;

	if(semaphore == NULL)
	{
		return RSI_ERROR_INVALID_PARAM;
	}

    OSSemPend((OS_EVENT*)(*semaphore), timeout_ms, &err);

    if(err == OS_ERR_NONE)
      return RSI_ERROR_NONE;
    else
    {
      APP_TRACE("rsi sem wait failed.\r\n");
      return RSI_ERROR_IN_OS_OPERATION;
    }
}


/*==============================================*/
/**
 * @fn          rsi_error_t rsi_semaphore_post(rsi_semaphore_handle_t *semaphore) 
 * @brief       This API is used by wireless library to release semaphore, which was acquired.
 * @param[in]   Semaphore handle pointer  
 * @param[in]   Maximum time to wait to acquire semaphore. If timeout_ms is 0 then wait
                till acquire semaphore.
 * @return      0 = Success
 *              <0 = failure
 * @section description
 * This API is used by Wireless Library to acquire or wait for semaphore.
 *
 */


rsi_error_t rsi_semaphore_post(rsi_semaphore_handle_t *semaphore)
{
	if(semaphore == NULL)
	{
		return RSI_ERROR_INVALID_PARAM;
	}

    if(OSSemPost((OS_EVENT*)(*semaphore)) == OS_ERR_NONE)
    {
      return RSI_ERROR_NONE;
    }
    else
      return RSI_ERROR_IN_OS_OPERATION;
}



/*==============================================*/
/**
 * @fn          rsi_error_t rsi_semaphore_reset(rsi_semaphore_handle_t *semaphore) 
 * @brief       This API is used by wireless library Wireless Library to the semaphore to initial state
 * @param[in]   Semaphore handle pointer  
 * @return      0 = Success
 *              <0 = failure
 * @section description
 * This API is used by Wireless Library to reset the semaphore.
 *
 */
rsi_error_t rsi_semaphore_reset(rsi_semaphore_handle_t *semaphore)
{
    uint8_t err;

	if(semaphore == NULL)
	{
		return RSI_ERROR_INVALID_PARAM;
	}

    OSSemSet((OS_EVENT*)(*semaphore), 0, &err);

    if(err == OS_ERR_NONE)
      return RSI_ERROR_NONE;
    else
      return RSI_ERROR_IN_OS_OPERATION;
}


/*==============================================*/
/**
 * @fn           rsi_error_t rsi_task_create( rsi_task_function_t task_function,uint8_t *task_name,
                 uint32_t stack_size, void *parameters,
                 uint32_t task_priority,rsi_task_handle_t  *task_handle)
 * @brief        This API is used to create different tasks in OS supported platforms 
 * @param[in]    Pointer to function to be executed by created thread. Prototype of the function
 * @param[in]    Name of the created task  
 * @param[in]    Stack size given to the created task  
 * @param[in]    Pointer to the parameters to be passed to task function
 * @param[in]    task priority 
 * @param[in]    task handle/instance created 
 * @return       0 = Success
 *              <0 = Failure
 * @section description
 * This API is used by Wireless Library to create platform specific OS
 * task/thread. 
 *
 *
 */

#if 0
rsi_error_t rsi_task_create( rsi_task_function_t task_function,uint8_t *task_name,
		uint32_t stack_size, void *parameters,
		uint32_t task_priority, rsi_task_handle_t  *task_handle)
{
#ifdef SAPIS_BT_STACK_ON_HOST
	task_priority = 7;
#endif
	if(pdPASS == xTaskCreate( task_function, (char const*)task_name,
			stack_size,
			parameters,
			task_priority,
			(TaskHandle_t*)task_handle))
	{
		return RSI_ERROR_NONE;
	}

	return RSI_ERROR_IN_OS_OPERATION;
}


/*==============================================*/
/**
 * @fn          void rsi_task_destroy(rsi_task_handle_t *task_handle)
 * @brief       This function deletes the task created
 * @param[in]   Task handle/instance to be deleted
 * @return      None
 * @section description
 * This API is used to delete/destroy the task created
 *
 */


void rsi_task_destroy(rsi_task_handle_t *task_handle)
{
	vTaskDelete((TaskHandle_t *) task_handle);
}



/*==============================================*/
/**
 * @fn           void rsi_start_os_scheduler()
 * @brief        This function schedules the tasks created
 * @param[in]    None 
 * @return       None
 * @section description
 * This API Schedules the tasks created
 *
 */
void rsi_start_os_scheduler()
{
  
	vTaskStartScheduler();
}
#endif

#endif

