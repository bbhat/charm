///////////////////////////////////////////////////////////////////////////////
//
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	os_sem.c
//	Author: Bala B.
//	Description: OS Semaphore implementation
//
///////////////////////////////////////////////////////////////////////////////

#include "os_sem.h"
#include "os_timer.h"
#include "os_sched.h"
#include "util.h"

// Placeholders for all the semaphore objects
OS_SemaphoreCB g_semaphore_pool[MAX_SEMAPHORE_COUNT];
UINT32 g_semaphore_usage_mask[(MAX_SEMAPHORE_COUNT + 31) >> 5];

// Bit #1 in the semaphore attributes indicates if this is a binary semaphore or not
#define BINARY_SEMAPHORE_MASK		1

// Helper macros
#define IS_BINARY_SEMAPHORE(attributes)		(attributes & BINARY_SEMAPHORE_MASK)
#define IS_COUNTING_SEMAPHORE(attributes)		(!(attributes & BINARY_SEMAPHORE_MASK))

static OS_Return assert_open(OS_Sem_t sem);

OS_Return _OS_SemAlloc(OS_Sem_t *sem, UINT32 value, BOOL binary)
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
	
	if(binary && value > 1) {
		status = BAD_ARGUMENT;
		goto exit;	
	}
		
	// Get a free Semaphore resource from the pool
	*sem = (OS_Sem_t) GetFreeResIndex(g_semaphore_usage_mask, MAX_SEMAPHORE_COUNT);
		
	if(*sem < 0) {
		status = RESOURCE_EXHAUSTED;
		goto exit;
	}
	
	OS_SemaphoreCB *semobj = (OS_SemaphoreCB *)&g_semaphore_pool[*sem];
	
	// Block the thread resource
	SetResourceStatus(g_semaphore_usage_mask, *sem, FALSE);

	semobj->attributes = (binary ? BINARY_SEMAPHORE_MASK : 0);
	semobj->count = value;
	semobj->owner = (OS_Process *) g_current_process;
	
	_OS_QueueInit(&semobj->periodic_wait_queue);
	_OS_QueueInit(&semobj->aperiodic_wait_queue);
	
	status = SUCCESS;
	
exit:
	return status;
}

OS_Return _OS_SemWait(OS_Sem_t sem)
{
	OS_Return status;
	
#if OS_ENABLE_CPU_STATS==1
	// We need to track how long did we take to schedule the next task
    g_sched_starting_counter_value = _OS_Timer_GetCount(PERIODIC_TIMER);
#endif	

	if((status = assert_open(sem)) != SUCCESS) {
		goto exit;
	}

	// Get the Semaphore object
	OS_SemaphoreCB * semobj = (OS_SemaphoreCB *)&g_semaphore_pool[sem];

	// Make sure that the current process owns the Semaphore.
	if(semobj->owner != (OS_Process *) g_current_process) {
		status = RESOURCE_NOT_OWNED;
		goto exit;
	}

    // Get the task execution time
    UINT32 budget_spent = _OS_Timer_GetTimeElapsed_us(BUDGET_TIMER);
    g_current_task->accumulated_budget += budget_spent;
	
	if(IS_PERIODIC_TASK(g_current_task->attributes)) {
	
	    // Adjust the remaining  budget for the current task
	    ASSERT(budget_spent <= g_current_task->p.remaining_budget);
	    g_current_task->p.remaining_budget -= budget_spent;		
	}

	// If the semaphore count is 0, then block the thread
	if(semobj->count == 0) {

		// Block the current task
		_OS_SchedulerBlockCurrentTask();
		
		// Block the thread			
		if(IS_PERIODIC_TASK(g_current_task->attributes)) {
		
			// Add the current task to the semaphore's blocked queue for periodic tasks
			// Note that this is not a priority queue
			_OS_NPQueueInsert(&semobj->periodic_wait_queue, (_OS_TaskQNode *)g_current_task);			
		}
		else {
		
			// Add the current task to the semaphore's blocked queue for aperiodic tasks
			// Note that we are going to use this as a priority queue
			_OS_PQueueInsertWithKey(&semobj->aperiodic_wait_queue, (_OS_TaskQNode *)g_current_task, 
				g_current_task->ap.priority());
		}
		
		Klog32(KLOG_SEMAPHORE_DEBUG, "Semaphore :- ", semobj->count);				
	}	
	else {
	
		// Decrement the semaphore count in order to acquire it
		semobj->count--;
		Klog32(KLOG_SEMAPHORE_DEBUG, "Semaphore - ", semobj->count);		
		
		// The return path for this function is through _OS_Schedule, so it is important to
		// update the result in the syscall_result
		if(g_current_task->syscall_result) {
			g_current_task->syscall_result[0] = SUCCESS;
		}
	}	
	
	_OS_Schedule();
	
exit:
	return status;
}

OS_Return _OS_SemPost(OS_Sem_t sem)
{
	OS_Return status;
	OS_Task * selected_task = NULL;
	
#if OS_ENABLE_CPU_STATS==1
	// We need to track how long did we take to schedule the next task
    g_sched_starting_counter_value = _OS_Timer_GetCount(PERIODIC_TIMER);
#endif

	if((status = assert_open(sem)) != SUCCESS) {
		goto exit;
	}
	
	// Get the Semaphore object
	OS_SemaphoreCB * semobj = (OS_SemaphoreCB *)&g_semaphore_pool[sem];
	
	// Make sure that the current process owns the Semaphore.
	if(semobj->owner != (OS_Process *) g_current_process) {
		status = RESOURCE_NOT_OWNED;
		goto exit;
	}
	
    // Get the task execution time
    UINT32 budget_spent = _OS_Timer_GetTimeElapsed_us(BUDGET_TIMER);
    g_current_task->accumulated_budget += budget_spent;
	
	if(IS_PERIODIC_TASK(g_current_task->attributes)) {
	
	    // Adjust the remaining  budget for the current task
	    ASSERT(budget_spent <= g_current_task->p.remaining_budget);
	    g_current_task->p.remaining_budget -= budget_spent;		
	}

	if(semobj->count == 0) {
	
		// Unblock a waiting task. First check the periodic queue
		// We need to go through all periodic waiting tasks and identify a ready task with
		// an earliest deadline. Since the periodic wait queue is not priority queue
		// we need to check all waiting tasks before we pick one
		OS_Task * task = (OS_Task *) semobj->periodic_wait_queue.head;
		UINT64 deadline = (UINT64) -1;	// Largest value
		while(task) {
		
			// Check if the task has a job waiting. Otherwise ignore the task
			if(task->p.job_release_time <= g_current_period_us) {
				if(deadline > task->p.alarm_time()) {
					deadline = task->p.alarm_time();
					selected_task = task;
				}
			}
			
			// Go to next task in the semaphore periodic wait queue (non-priority queue)
			task = (OS_Task *) task->qp.np_next;
		}

		if(selected_task) {
		
			// Take this task out from both semaphore wait queue
			_OS_NPQueueDelete(&semobj->periodic_wait_queue, (_OS_TaskQNode *) selected_task);			
		}
		else {
		
			// Check the aperiodic_wait_queue
			_OS_PQueueGet(&semobj->aperiodic_wait_queue, (_OS_TaskQNode **) &selected_task);		
		}
	}
	
	if(selected_task) {
	
		// Reblock this task into the scheduler queue
		_OS_SchedulerUnblockTask(selected_task);
			
		// If a task is getting ready, then there is no need to increment the semaphore count
		// The return path for this function is through _OS_Schedule, so it is important to
		// update the result in the syscall_result
		if(selected_task->syscall_result) 
			selected_task->syscall_result[0] = SUCCESS;
		Klog32(KLOG_SEMAPHORE_DEBUG, "Semaphore :+ ", semobj->count);		
	}
	else if(!semobj->count || IS_COUNTING_SEMAPHORE(semobj->attributes)) {
	
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
	OS_Task* task = NULL;
	
#if OS_ENABLE_CPU_STATS==1
    g_sched_starting_counter_value = _OS_Timer_GetCount(PERIODIC_TIMER);
#endif
	
	if((status = assert_open(sem)) != SUCCESS) {
		goto exit;
	}
	
	// Get the Semaphore object
	OS_SemaphoreCB * semobj = (OS_SemaphoreCB *)&g_semaphore_pool[sem];
	
	// Make sure that the current process owns the Semaphore.
	if(semobj->owner != (OS_Process *) g_current_process) {
		status = RESOURCE_NOT_OWNED;
		goto exit;
	}

    // Get the task execution time
    UINT32 budget_spent = _OS_Timer_GetTimeElapsed_us(BUDGET_TIMER);
    g_current_task->accumulated_budget += budget_spent;
	
	if(IS_PERIODIC_TASK(g_current_task->attributes)) {
	
	    // Adjust the remaining  budget for the current task
	    ASSERT(budget_spent <= g_current_task->p.remaining_budget);
	    g_current_task->p.remaining_budget -= budget_spent;		
	}

	// We need to unblock all waiting periodic tasks
	while(TRUE) {
	
		// First the periodic wait queue
		_OS_NPQueueGet(&semobj->periodic_wait_queue, (_OS_TaskQNode **)&task);		
		if(!task) break;
		
		// Unblock this task
		_OS_SchedulerUnblockTask(task);

		// The return path for waiting tasks is through _OS_Schedule, so it is important to
		// update the result in the syscall_result	
		if(task->syscall_result) {
			task->syscall_result[0] = RESOURCE_DELETED;
		}
	}	

	// Then check the Aperiodic queue
	while(TRUE) {
	
		// The aperiodic_wait_queue is a priority queue
		_OS_PQueueGet(&semobj->aperiodic_wait_queue, (_OS_TaskQNode **)&task);
		if(!task) break;

		// Unblock this task
		_OS_SchedulerUnblockTask(task);
		
		// The return path for waiting tasks is through _OS_Schedule, so it is important to
		// update the result in the syscall_result	
		if(task->syscall_result) 
			task->syscall_result[0] = RESOURCE_DELETED;		
	}	

	semobj->attributes = 0;
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
	if(semobj->owner != (OS_Process *) g_current_process) {
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
	if(sem < 0 || sem >= MAX_SEMAPHORE_COUNT) {
		return BAD_ARGUMENT;
	}
	
	if(!IsResourceBusy(g_semaphore_usage_mask, sem)) {
		return RESOURCE_NOT_OPEN;	
	}
	
	return SUCCESS;
}
