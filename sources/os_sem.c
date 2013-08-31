///////////////////////////////////////////////////////////////////////////////
//
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	os_sem.c
//	Author: Bala B.
//	Description: OS Semaphore implementation
//
///////////////////////////////////////////////////////////////////////////////

#include "os_core.h"
#include "os_sem.h"
#include "util.h"

// Placeholders for all the semaphore objects
OS_SemaphoreCB g_semaphore_pool[MAX_SEMAPHORE_COUNT];
UINT32 g_semaphore_usage_mask[(MAX_SEMAPHORE_COUNT + 31) >> 5];

extern volatile OS_GenericTask * g_current_task;
extern _OS_Queue g_ready_q;
extern _OS_Queue g_ap_ready_q;
extern _OS_Queue g_ap_wait_q;
extern void _OS_ReSchedule();

static BOOL assert_open(OS_Sem sem);

OS_Error _OS_SemAlloc(OS_Sem *sem, UINT32 value)
{
	UINT32 intsts;
	OS_Error status;
	
	if(!sem) {
		status = ARGUMENT_ERROR;
		goto exit;
	}
		
	if(!g_current_process) {
		status = PROCESS_INVALID;
		goto exit;
	}
		
	OS_ENTER_CRITICAL(intsts);

	// Get a free Semaphore resource from the pool
	*sem = (OS_Sem) GetFreeResIndex(g_semaphore_usage_mask, MAX_SEMAPHORE_COUNT);
		
	if(*sem < 0) 
	{
		OS_EXIT_CRITICAL(intsts);
		status = RESOURCE_EXHAUSTED;
		goto exit;
	}
	
	OS_SemaphoreCB *semobj = (OS_SemaphoreCB *)&g_semaphore_pool[*sem];

	semobj->count = value;
	semobj->owner = (OS_ProcessCB *) g_current_process;
	_OS_QueueInit(&semobj->periodic_task_queue);
	_OS_QueueInit(&semobj->aperiodic_task_queue);
	
	OS_EXIT_CRITICAL(intsts);
	status = SUCCESS;
	
exit:
	return status;
}

OS_Error _OS_SemWait(OS_Sem sem)
{
	UINT32 intsts;
	OS_Error status;
	OS_GenericTask * cur_task = (OS_GenericTask *) g_current_task;

	OS_ENTER_CRITICAL(intsts);

	// Keep trying till we get the Semaphore	
	while(1)
	{
		if((status = assert_open(sem)) != SUCCESS) {
			goto exit;
		}
	
		// Get the Semaphore object
		OS_SemaphoreCB * semobj = (OS_SemaphoreCB *)&g_semaphore_pool[sem];
	
		// Make sure that the current process owns the Semaphore.
		if(semobj->owner != (OS_ProcessCB *) g_current_process)
		{
			status = RESOURCE_NOT_OWNED;
			goto exit;
		}	
	
		// If the semaphore count is 0, then block the thread
		if(semobj->count == 0)
		{
			// Block the thread			
			if(IS_PERIODIC_TASK(cur_task->attributes))
			{
				// Delete the current task from ready tasks queue
				_OS_QueueDelete(&g_ready_q, (void*)cur_task); 
				
				// Add the current task to the semaphore's blocked queue for periodic tasks
				_OS_QueueInsert(&semobj->periodic_task_queue, (void*)cur_task, 
					((OS_PeriodicTask *)cur_task)->alarm_time); 
			}
			else
			{
				// Delete the current task from ready tasks queue
				_OS_QueueDelete(&g_ap_ready_q, (void*)cur_task); 
				
				// Add the current task to the semaphore's blocked queue for aperiodic tasks
				_OS_QueueInsert(&semobj->aperiodic_task_queue, (void*)cur_task, 
					((OS_AperiodicTask *)cur_task)->priority);
			}
			
			// It is OK to keep the interrupts disabled before switching. This saves us entering critical
			// section again upon re-entering this task.
			_OS_ReSchedule();
		}	
		else
		{
			semobj->count--;	
			status = SUCCESS;
			break;
		}
	}
exit:
	OS_EXIT_CRITICAL(intsts);
	return status;
}

OS_Error _OS_SemPost(OS_Sem sem)
{
	OS_GenericTask* task = NULL;
	UINT32 intsts;
	OS_Error status;
	UINT64 key = 0;

	OS_ENTER_CRITICAL(intsts);

	if((status = assert_open(sem)) != SUCCESS) {
		goto exit;
	}
	
	// Get the Semaphore object
	OS_SemaphoreCB * semobj = (OS_SemaphoreCB *)&g_semaphore_pool[sem];
	
	// Make sure that the current process owns the Semaphore.
	if(semobj->owner != (OS_ProcessCB *) g_current_process)
	{
		status = RESOURCE_NOT_OWNED;
		goto exit;
	}	

	if(semobj->count == 0)
	{
		// Unblock a waiting task. First check the periodic queue
		_OS_QueueGet(&semobj->periodic_task_queue, (void**)&task, &key);
		if(task)
		{
			// Place this task in the periodic task ready queue
			_OS_QueueInsert(&g_ready_q,(void*)task, key);
		}
		
		else 
		{
			// Now check the Aperiodic queue
			_OS_QueueGet(&semobj->aperiodic_task_queue, (void**)&task, &key);
			if(task)
			{
				// Insert this task into Aperiodic task queue
				_OS_QueueInsert(&g_ap_ready_q,(void*)task,key);
			}
		}
	}

	// Increase the resource count
	semobj->count++;
		
	// If there is a task getting ready, we need to call reschedule to give it 
	// an opportunity to run (if that had higher priority)
	if(task) 
	{
		// It is OK to keep the interrupts disabled before switching.	
		_OS_ReSchedule();
	}
	
	status = SUCCESS;
exit:
	OS_EXIT_CRITICAL(intsts);
	return status;
}

OS_Error _OS_SemFree(OS_Sem sem)
{
	OS_Error status;
	UINT32 intsts;
	OS_GenericTask* task = NULL;
	UINT64 key = 0;
	
	OS_ENTER_CRITICAL(intsts);
	
	if((status = assert_open(sem)) != SUCCESS) {
		goto exit;
	}
	
	// Get the Semaphore object
	OS_SemaphoreCB * semobj = (OS_SemaphoreCB *)&g_semaphore_pool[sem];
	
	// Make sure that the current process owns the Semaphore.
	if(semobj->owner != (OS_ProcessCB *) g_current_process)
	{
		status = RESOURCE_NOT_OWNED;
		goto exit;
	}	

	// We need to unblock all waiting threads in its wait queues and make them ready
	while(TRUE)
	{
		// First check the periodic queue
		_OS_QueueGet(&semobj->periodic_task_queue, (void**)&task, &key);

		if(!task) break;
		
		// Place this task in the periodic task ready queue
		_OS_QueueInsert(&g_ready_q,(void*)task, key);
	}	

	// Then check the Aperiodic queue
	while(TRUE)
	{
		_OS_QueueGet(&semobj->aperiodic_task_queue, (void**)&task, &key);

		if(!task) break;
		
		// Insert this task into Aperiodic ready queue
		_OS_QueueInsert(&g_ap_ready_q,(void*)task, key);
	}	

	semobj->count = 0;
	semobj->owner = NULL;
	status = SUCCESS;
	
exit:
	OS_EXIT_CRITICAL(intsts);
	return status;
}

OS_Error _OS_SemGetValue(OS_Sem sem, INT32* val)
{
	OS_Error status;
	UINT32 intsts;
	
	OS_ENTER_CRITICAL(intsts);
	
	if((status = assert_open(sem)) != SUCCESS) {
		goto exit;
	}
	
	// Get the Semaphore object
	OS_SemaphoreCB * semobj = (OS_SemaphoreCB *)&g_semaphore_pool[sem];
	
	// Make sure that the current process owns the Semaphore.
	if(semobj->owner != (OS_ProcessCB *) g_current_process)
	{
		status = RESOURCE_NOT_OWNED;
		goto exit;
	}	

	if(val) *val = semobj->count;	
	status = SUCCESS;
	
exit:
	OS_EXIT_CRITICAL(intsts);
	return status;
}

static OS_Error assert_open(OS_Sem sem)
{
	if(sem < 0 || sem >= MAX_SEMAPHORE_COUNT)
	{
		return ARGUMENT_ERROR;
	}
	
	if(!IsResourceBusy(g_semaphore_usage_mask, sem))
	{
		return RESOURCE_NOT_OPEN;	
	}
	
	return SUCCESS;
}
