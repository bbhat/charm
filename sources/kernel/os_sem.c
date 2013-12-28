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
#include "os_timer.h"
#include "util.h"

// Placeholders for all the semaphore objects
OS_SemaphoreCB g_semaphore_pool[MAX_SEMAPHORE_COUNT];
UINT32 g_semaphore_usage_mask[(MAX_SEMAPHORE_COUNT + 31) >> 5];

#if OS_ENABLE_CPU_STATS==1
extern UINT32 g_sched_starting_counter_value;
#endif

extern OS_GenericTask * g_current_task;
extern _OS_Queue g_ready_q;
extern _OS_Queue g_wait_q;
extern _OS_Queue g_ap_ready_q;
extern _OS_Queue g_ap_wait_q;
extern void _OS_Schedule();

static OS_Return assert_open(OS_Sem_t sem);

OS_Return _OS_SemAlloc(OS_Sem_t *sem, UINT32 value)
{
	OS_Return status;
	
	if(!sem) {
		status = BAD_ARGUMENT;
		goto exit;
	}
		
	if(!g_current_process) {
		status = PROCESS_INVALID;
		goto exit;
	}
		
	// Get a free Semaphore resource from the pool
	*sem = (OS_Sem_t) GetFreeResIndex(g_semaphore_usage_mask, MAX_SEMAPHORE_COUNT);
		
	if(*sem < 0) 
	{
		status = RESOURCE_EXHAUSTED;
		goto exit;
	}
	
	OS_SemaphoreCB *semobj = (OS_SemaphoreCB *)&g_semaphore_pool[*sem];
	
	// Block the thread resource
	SetResourceStatus(g_semaphore_usage_mask, *sem, FALSE);

	semobj->count = value;
	semobj->owner = (OS_Process *) g_current_process;
	_OS_QueueInit(&semobj->periodic_task_queue);
	_OS_QueueInit(&semobj->aperiodic_task_queue);
	
	status = SUCCESS;
	
exit:
	return status;
}

OS_Return _OS_SemWait(OS_Sem_t sem)
{
	OS_Return status;
	
#if OS_ENABLE_CPU_STATS==1
    g_sched_starting_counter_value = _OS_Timer_GetCount(PERIODIC_TIMER);
#endif	

	if((status = assert_open(sem)) != SUCCESS) {
		goto exit;
	}

	// Get the Semaphore object
	OS_SemaphoreCB * semobj = (OS_SemaphoreCB *)&g_semaphore_pool[sem];

	// Make sure that the current process owns the Semaphore.
	if(semobj->owner != (OS_Process *) g_current_process)
	{
		status = RESOURCE_NOT_OWNED;
		goto exit;
	}

    // Get the task execution time
    UINT32 budget_spent = _OS_Timer_GetTimeElapsed_us(BUDGET_TIMER);
    g_current_task->accumulated_budget += budget_spent;
	
	if(IS_PERIODIC_TASK(g_current_task->attributes))
	{
	    // Adjust the remaining  budget for the current task
	    ASSERT(budget_spent <= ((OS_PeriodicTask *)g_current_task)->remaining_budget);
	    ((OS_PeriodicTask *)g_current_task)->remaining_budget -= budget_spent;		
	}

	// If the semaphore count is 0, then block the thread
	if(semobj->count == 0)
	{
		// Block the thread			
		if(IS_PERIODIC_TASK(g_current_task->attributes))
		{
			// Delete the current task from ready tasks queue
			_OS_QueueDelete(&g_ready_q, (void*)g_current_task); 
			
			// Add the current task to the semaphore's blocked queue for periodic tasks
			_OS_QueueInsert(&semobj->periodic_task_queue, (void*)g_current_task, 
				((OS_PeriodicTask *)g_current_task)->alarm_time); 
		}
		else
		{
			// Delete the current task from ready tasks queue
			_OS_QueueDelete(&g_ap_ready_q, (void*)g_current_task); 
			
			// Add the current task to the semaphore's blocked queue for aperiodic tasks
			_OS_QueueInsert(&semobj->aperiodic_task_queue, (void*)g_current_task, 
				((OS_AperiodicTask *)g_current_task)->priority);
		}
		
		Klog32(KLOG_SEMAPHORE_DEBUG, "Semaphore :- ", semobj->count);				
	}	
	else
	{
		// Decrement the semaphore count in order to acquire it
		semobj->count--;
		Klog32(KLOG_SEMAPHORE_DEBUG, "Semaphore - ", semobj->count);		
		
		// The return path for this function is through _OS_Schedule, so it is important to
		// update the result in the syscall_result
		if(g_current_task->syscall_result) 
			g_current_task->syscall_result[0] = SUCCESS;		
	}	
	
	_OS_Schedule();
	
exit:
	return status;
}

OS_Return _OS_SemPost(OS_Sem_t sem)
{
	OS_GenericTask* task = NULL;
	OS_Return status;
	UINT64 key = 0;
	
#if OS_ENABLE_CPU_STATS==1
    g_sched_starting_counter_value = _OS_Timer_GetCount(PERIODIC_TIMER);
#endif

	if((status = assert_open(sem)) != SUCCESS) {
		goto exit;
	}
	
	// Get the Semaphore object
	OS_SemaphoreCB * semobj = (OS_SemaphoreCB *)&g_semaphore_pool[sem];
	
	// Make sure that the current process owns the Semaphore.
	if(semobj->owner != (OS_Process *) g_current_process)
	{
		status = RESOURCE_NOT_OWNED;
		goto exit;
	}
	
    // Get the task execution time
    UINT32 budget_spent = _OS_Timer_GetTimeElapsed_us(BUDGET_TIMER);
    g_current_task->accumulated_budget += budget_spent;
	
	if(IS_PERIODIC_TASK(g_current_task->attributes))
	{
	    // Adjust the remaining  budget for the current task
	    ASSERT(budget_spent <= ((OS_PeriodicTask *)g_current_task)->remaining_budget);
	    ((OS_PeriodicTask *)g_current_task)->remaining_budget -= budget_spent;		
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
	
	if(task)
	{
		// If a task is getting ready, then there is no need to increment the semaphore count
		// The return path for this function is through _OS_Schedule, so it is important to
		// update the result in the syscall_result
		if(task->syscall_result) 
			task->syscall_result[0] = SUCCESS;
		Klog32(KLOG_SEMAPHORE_DEBUG, "Semaphore :+ ", semobj->count);		
	}
	else
	{
		// Increment the resource count
		semobj->count++;	
		Klog32(KLOG_SEMAPHORE_DEBUG, "Semaphore + ", semobj->count);		
	}
		
	_OS_Schedule();	
	
exit:
	return status;
}

OS_Return _OS_SemFree(OS_Sem_t sem)
{
	OS_Return status;
	OS_GenericTask* task = NULL;
	UINT64 key = 0;
	
#if OS_ENABLE_CPU_STATS==1
    g_sched_starting_counter_value = _OS_Timer_GetCount(PERIODIC_TIMER);
#endif
	
	if((status = assert_open(sem)) != SUCCESS) {
		goto exit;
	}
	
	// Get the Semaphore object
	OS_SemaphoreCB * semobj = (OS_SemaphoreCB *)&g_semaphore_pool[sem];
	
	// Make sure that the current process owns the Semaphore.
	if(semobj->owner != (OS_Process *) g_current_process)
	{
		status = RESOURCE_NOT_OWNED;
		goto exit;
	}

    // Get the task execution time
    UINT32 budget_spent = _OS_Timer_GetTimeElapsed_us(BUDGET_TIMER);
    g_current_task->accumulated_budget += budget_spent;
	
	if(IS_PERIODIC_TASK(g_current_task->attributes))
	{
	    // Adjust the remaining  budget for the current task
	    ASSERT(budget_spent <= ((OS_PeriodicTask *)g_current_task)->remaining_budget);
	    ((OS_PeriodicTask *)g_current_task)->remaining_budget -= budget_spent;		
	}

	// We need to unblock all waiting threads in its wait queues and make them ready
	while(TRUE)
	{
		// First check the periodic queue
		_OS_QueueGet(&semobj->periodic_task_queue, (void**)&task, &key);

		if(!task) break;
		
		// The return path for waiting tasks is through _OS_Schedule, so it is important to
		// update the result in the syscall_result	
		if(task->syscall_result) 
			task->syscall_result[0] = RESOURCE_DELETED;
		
		// Place this task in the periodic task ready queue
		_OS_QueueInsert(&g_ready_q,(void*)task, key);
	}	

	// Then check the Aperiodic queue
	while(TRUE)
	{
		_OS_QueueGet(&semobj->aperiodic_task_queue, (void**)&task, &key);

		if(!task) break;
		
		// The return path for waiting tasks is through _OS_Schedule, so it is important to
		// update the result in the syscall_result	
		if(task->syscall_result) 
			task->syscall_result[0] = RESOURCE_DELETED;
		
		// Insert this task into Aperiodic ready queue
		_OS_QueueInsert(&g_ap_ready_q,(void*)task, key);
	}	

	semobj->count = 0;
	semobj->owner = NULL;
	
	_OS_Schedule();	
	
exit:
	return status;
}

OS_Return _OS_SemGetValue(OS_Sem_t sem, UINT32* val)
{
	OS_Return status;
		
	if((status = assert_open(sem)) != SUCCESS) {
		goto exit;
	}
	
	// Get the Semaphore object
	OS_SemaphoreCB * semobj = (OS_SemaphoreCB *)&g_semaphore_pool[sem];
	
	// Make sure that the current process owns the Semaphore.
	if(semobj->owner != (OS_Process *) g_current_process)
	{
		status = RESOURCE_NOT_OWNED;
		goto exit;
	}	

	if(val) *val = semobj->count;	
	status = SUCCESS;
	
exit:
	return status;
}

static OS_Return assert_open(OS_Sem_t sem)
{
	if(sem < 0 || sem >= MAX_SEMAPHORE_COUNT)
	{
		return BAD_ARGUMENT;
	}
	
	if(!IsResourceBusy(g_semaphore_usage_mask, sem))
	{
		return RESOURCE_NOT_OPEN;	
	}
	
	return SUCCESS;
}
