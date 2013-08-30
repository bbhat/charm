///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	os_task.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: OS Task Related routines
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_core.h"
#include "os_queue.h"
#include "os_timer.h"
#include "os_task.h"
#include "util.h"

// function prototype declaration
static BOOL ValidateNewThread(UINT32 period, UINT32 budget);

#ifdef _USE_STD_LIBS
	#define FAULT(x, ...) printf(x, ...);
#else
	#define FAULT(x, ...)
#endif

// Macro for calculating the thread usage
#define CALC_THREAD_CPU_USAGE(period, budget) ((FP32)budget / (FP32)period)
#define MIN(a,b) ((a)<(b)?(a):(b))
#define ALIGNED_ARRAY(ptr) ASSERT((int) ptr % 8 == 0)

// Variable declaration/definition
static UINT32 g_task_id_counter = 0;
static FP32 g_total_allocated_cpu = 0.0;

// Placeholders for all the task control blocks
OS_GenericTask	g_task_pool[MAX_TASK_COUNT];
UINT32 			g_task_usage_mask[(MAX_TASK_COUNT + 31) >> 5];

extern _OS_Queue g_ready_q;
extern _OS_Queue g_wait_q;
extern _OS_Queue g_ap_ready_q;
extern _OS_Queue g_block_q;
extern volatile UINT64 g_global_time;	// This variable gets updated everytime the Timer ISR is called.
extern volatile UINT64 g_next_wakeup_time; // This variable holds the next scheduled wakeup time in uSecs
extern void _OS_ReSchedule();
extern void _OS_SetAlarm(OS_PeriodicTask *task, UINT64 abs_time_in_us, BOOL is_new_job, BOOL update_timer);

UINT32 *_OS_BuildKernelTaskStack(UINT32 * stack_ptr, void (*task_function)(void *), void * arg);
UINT32 *_OS_BuildUserTaskStack(UINT32 * stack_ptr, void (*task_function)(void (*entry_function)(void *pdata), 
	void *pdata), void * arg);
void KernelTaskEntryMain(void *pdata);
void UserTaskEntryMain(void (*entry_function)(void *pdata), void *pdata);
void AperiodicKernelTaskEntry(void *pdata);
void AperiodicUserTaskEntry(void (*entry_function)(void *pdata), void *pdata);
void _OS_ReSchedule();

///////////////////////////////////////////////////////////////////////////////
// OS_CreatePeriodicTask - API to create periodic tasks
///////////////////////////////////////////////////////////////////////////////
OS_Error OS_CreatePeriodicTask(
	UINT32 period_in_us,
	UINT32 deadline_in_us,
	UINT32 budget_in_us,
	UINT32 phase_shift_in_us,
	UINT32 *stack,
	UINT32 stack_size_in_bytes,
	const INT8 * task_name,
	OS_Task *task,
	void (*periodic_entry_function)(void *pdata),
	void *pdata)
{	
	return _OS_CreatePeriodicTask(
			period_in_us,
			deadline_in_us,
			budget_in_us,
			phase_shift_in_us,
			stack,
			stack_size_in_bytes,
			task_name,
			USER_TASK,
			task,
			periodic_entry_function,
			pdata);
}

///////////////////////////////////////////////////////////////////////////////
// OS_CreatePeriodicTask - API to create periodic tasks
//		OS Internal function with more arguments
///////////////////////////////////////////////////////////////////////////////
OS_Error _OS_CreatePeriodicTask(
	UINT32 period_in_us,
	UINT32 deadline_in_us,
	UINT32 budget_in_us,
	UINT32 phase_shift_in_us,
	UINT32 *stack,
	UINT32 stack_size_in_bytes,
	const INT8 * task_name,
	UINT16 options,
	OS_Task *task,
	void (*periodic_entry_function)(void *pdata),
	void *pdata)
{
	FP32 this_thread_cpu;
	UINT32 stack_size;
	UINT32 intsts;
	OS_PeriodicTask *tcb;

	if(!task)
	{
	    FAULT("Invalid Task");
		return INVALID_TASK;
	}

    if(!periodic_entry_function || !stack)
	{
	    FAULT("One or more invalid %s arguments", "task");
		return INVALID_ARG;
	}

	if(period_in_us < TASK_MIN_PERIOD)
	{
		FAULT("The period should be greater than %d uSec\n", TASK_MIN_PERIOD);
		return INVALID_PERIOD;
	}
	if(deadline_in_us < budget_in_us)
	{
		FAULT("The deadline should be at least as much as its budget\n");
		return INVALID_DEADLINE;
	}
	if(deadline_in_us > period_in_us)
	{
		FAULT("The deadline should be less than or equal to the period\n");
		return INVALID_DEADLINE;
	}
	if(budget_in_us < TASK_MIN_BUDGET)
	{
		FAULT("The budget should be greater than %d uSec\n", TASK_MIN_BUDGET);
		return INVALID_BUDGET;
	}
	
	// Validate for minimum stack size for user stacks. Kernel stacks know what they want
	if(IS_USER_TASK(options) && (stack_size_in_bytes < OS_MIN_USER_STACK_SIZE))	
	{
		FAULT("Stack size should be at least %d bytes", OS_MIN_USER_STACK_SIZE);
		return INSUFFICIENT_STACK;
	}

	// Now get a free TCB resource from the pool
	*task = (OS_Task) GetFreeResIndex(g_task_usage_mask, MAX_TASK_COUNT);
	if(*task < 0) 
	{
		FAULT("_OS_CreateAperiodicTask failed for process %s: Exhausted all resources\n", g_current_process->name);
		return RESOURCE_EXHAUSTED;	
	}

	// Get a pointer to the actual TCB	
	tcb = (OS_PeriodicTask *)&g_task_pool[*task];

	// Ensure that the stack is 8 byte aligned
	//ALIGNED_ARRAY(stack);

	// Convert the stack_size_in_bytes into number of words
	stack_size = stack_size_in_bytes >> 2; 

	tcb->attributes = (PERIODIC_TASK | options);
#if OS_WITH_VALIDATE_TASK==1
	tcb->signature = TASK_SIGNATURE;
#endif
	strncpy(tcb->name, task_name, OS_TASK_NAME_SIZE);

	tcb->budget = budget_in_us;
	tcb->period = period_in_us;
	tcb->deadline = deadline_in_us;
	tcb->phase = phase_shift_in_us;
	tcb->stack = stack;
	tcb->stack_size = stack_size;
	tcb->top_of_stack = stack + stack_size;	// Stack grows bottom up
	tcb->task_function = periodic_entry_function;
	tcb->pdata = pdata;
	tcb->remaining_budget = 0;
	tcb->accumulated_budget = 0;
	tcb->stored_release_time = 0;
	tcb->exec_count = 0;
	tcb->TBE_count = 0;
	tcb->dline_miss_count = 0;
	//task->next_release_time = phase_shift_in_us;
	tcb->alarm_time = 0;
	
	// Note down the owner process
	tcb->owner_process = g_current_process;

	OS_ENTER_CRITICAL(intsts);
	//if(!ValidateNewThread(period_in_us, budget_in_us))
	if(!ValidateNewThread(MIN(period_in_us, deadline_in_us), budget_in_us))
	{
		OS_EXIT_CRITICAL(intsts); 
		FAULT("The total allocated CPU exceeds %d%", 100);
		return EXCEEDS_MAX_CPU;
	}

	tcb->id = ++g_task_id_counter;
	
	// Block the thread resource
	SetResourceStatus(g_task_usage_mask, *task, FALSE);
	
	// Calculate the CPU usage of the current thread
	this_thread_cpu = CALC_THREAD_CPU_USAGE(MIN(period_in_us, deadline_in_us), budget_in_us);
	
	// Update the g_total_allocated_cpu
	g_total_allocated_cpu += this_thread_cpu;
	
	OS_EXIT_CRITICAL(intsts); 	// Exit the critical section

	// Build a Stack for the new thread
	if(IS_SYSTEM_TASK(tcb->attributes))
	{
		tcb->top_of_stack = _OS_BuildKernelTaskStack(tcb->top_of_stack, 
			KernelTaskEntryMain, tcb);
	}
	else	// User stack
	{
		tcb->top_of_stack = _OS_BuildUserTaskStack(tcb->top_of_stack, 
			UserTaskEntryMain, tcb);	
	}
	
	OS_ENTER_CRITICAL(intsts);	// Enter critical section

	_OS_QueueInsert(&g_wait_q, tcb, phase_shift_in_us);		

	OS_EXIT_CRITICAL(intsts); 	// Exit the critical section

	if(_OS_IsRunning)
	{
		_OS_ReSchedule();
	}

  	return SUCCESS; 
}

///////////////////////////////////////////////////////////////////////////////
// OS_CreateAperiodicTask
// 		OS API for creating Aperiodic task
///////////////////////////////////////////////////////////////////////////////
OS_Error OS_CreateAperiodicTask(UINT16 priority, 
	UINT32 * stack, UINT32 stack_size_in_bytes,
	const INT8 * task_name,
	OS_Task * task,
	void(* task_entry_function)(void * pdata),
	void * pdata)
{
	if(priority > MIN_PRIORITY)
	{
		FAULT("The priority value %d should be within 0 and %d", priority, MIN_PRIORITY);
		return INVALID_PRIORITY;
	}

	return _OS_CreateAperiodicTask(priority,
		stack,
		stack_size_in_bytes,
		task_name,
		USER_TASK,
		task,
		task_entry_function,
		pdata);		
}

///////////////////////////////////////////////////////////////////////////////
// The task creation routine for Aperiodic tasks
//		OS Internal function with more arguments
///////////////////////////////////////////////////////////////////////////////
OS_Error _OS_CreateAperiodicTask(UINT16 priority, 
	UINT32 * stack, UINT32 stack_size_in_bytes,
	const INT8 * task_name,
	UINT16 options,
	OS_Task * task,
	void(* task_entry_function)(void * pdata),
	void * pdata)
{
	UINT32 stack_size;
	UINT32 intsts;
	OS_AperiodicTask *tcb;

	if(!task)
	{
		FAULT("Invalid Task");
		return INVALID_TASK;
	}

	if(!task_entry_function || !stack)
	{
		FAULT("One or more invalid %s arguments", "task");
		return INVALID_ARG;
	}
	
	// Validate for minimum stack size for user stacks. Kernel stacks know what they want
	if(IS_USER_TASK(options) && (stack_size_in_bytes < OS_MIN_USER_STACK_SIZE))	
	{
		FAULT("Stack size should be at least %d bytes", OS_MIN_USER_STACK_SIZE);
		return INSUFFICIENT_STACK;
	}	
	
	// Now get a free TCB resource from the pool
	*task = (OS_Task) GetFreeResIndex(g_task_usage_mask, MAX_TASK_COUNT);
	if(*task < 0) 
	{
		FAULT("_OS_CreatePeriodicTask failed for process %s: Exhausted all resources\n", g_current_process->name);
		return RESOURCE_EXHAUSTED;	
	}
	
	// Get a pointer to the TCB
	tcb = (OS_AperiodicTask *)&g_task_pool[*task];

	// Convert the stack_size_in_bytes into number of words
	stack_size = stack_size_in_bytes >> 2;

	tcb->stack = stack;
	tcb->stack_size = stack_size;
	tcb->task_function = task_entry_function;
	tcb->pdata = pdata;
	tcb->priority = priority;
	tcb->attributes = (APERIODIC_TASK | options);
	tcb->top_of_stack = stack + stack_size; // Stack grows bottom up
	
	// Build a Stack for the new thread
	if(IS_SYSTEM_TASK(tcb->attributes))
	{
		tcb->top_of_stack = _OS_BuildKernelTaskStack(tcb->top_of_stack, 
			AperiodicKernelTaskEntry, tcb);
	}
	else	// User stack
	{
		tcb->top_of_stack = _OS_BuildUserTaskStack(tcb->top_of_stack, 
			AperiodicUserTaskEntry, tcb);	
	}

#if OS_WITH_VALIDATE_TASK==1
	tcb->signature = TASK_SIGNATURE;
#endif
	strncpy(tcb->name, task_name, OS_TASK_NAME_SIZE);

	// Note down the owner process
	tcb->owner_process = g_current_process;

	OS_ENTER_CRITICAL(intsts); // Enter critical section
	tcb->id = ++g_task_id_counter;
	
	// Block the resource
	SetResourceStatus(g_task_usage_mask, *task, FALSE);
	
	_OS_QueueInsert(&g_ap_ready_q, tcb, priority); // Add the task to aperiodic wait queue
	OS_EXIT_CRITICAL(intsts); // Exit the critical section
	
	if(_OS_IsRunning)
	{
		_OS_ReSchedule();
	}

	return SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// Validation for sufficient CPU Budget
// ASSUMPTION: The interrupts are disabled when this function is invoked
///////////////////////////////////////////////////////////////////////////////
static BOOL ValidateNewThread(UINT32 period, UINT32 budget)
{
	// The current threads budget in terms of CPU usage %ge
	FP32 this_thread_cpu = CALC_THREAD_CPU_USAGE(period, budget);

	// Dont allow allocation of more than 90% of thr CPU
	if(g_total_allocated_cpu + this_thread_cpu >= 1.0)
	{
		return FALSE;
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// This is the main entry function for all in-kernel periodic functions
///////////////////////////////////////////////////////////////////////////////
void KernelTaskEntryMain(void *pdata)
{
	OS_PeriodicTask * task = (OS_PeriodicTask *) pdata;

#if OS_WITH_VALIDATE_TASK==1
	// Validate the task
	if(task->signature != TASK_SIGNATURE) {
		panic("Invalid Task %p", task);
	}
#endif // OS_WITH_VALIDATE_TASK

	while(1)
	{
		// Call the thread handler function
		task->task_function(task->pdata);
		
		OS_TaskYield();
	}
}

void AperiodicKernelTaskEntry(void *pdata)
{
	OS_AperiodicTask * task = (OS_AperiodicTask *) pdata;
	UINT32 intsts;

	// Call the thread handler function
	task->task_function(task->pdata);

	// If this function ever returns, just block this task by adding it to
	// block q
	OS_ENTER_CRITICAL(intsts);		
	_OS_QueueDelete(&g_ap_ready_q, task);

	// Insert into block q
	_OS_QueueInsert(&g_block_q, task, task->priority);
	
	// TODO: Free the TCB resource
	// SetResourceStatus
	
	OS_EXIT_CRITICAL(intsts);

	// Now call reschedule function
	_OS_ReSchedule();
}

///////////////////////////////////////////////////////////////////////////////
// The following function gets the total time taken by the current
// thread since the thread has begun. Note that this is not the global 
// time, this is just the time taken from only the current thread.
// Note that this function is defined only for periodic tasks
///////////////////////////////////////////////////////////////////////////////
UINT64 OS_GetThreadElapsedTime()
{
    UINT64 thread_elapsed_time = 0;
	UINT64 old_global_time;

	// First get the current task which is running...
	OS_PeriodicTask * task = (OS_PeriodicTask *)OS_GetCurrentTask();
		
	if(task && IS_PERIODIC_TASK(task->attributes)) 
	{
		do
		{
			old_global_time = g_global_time;
			thread_elapsed_time = task->accumulated_budget + _OS_GetTimerValue_us();
		} 
		while(old_global_time != g_global_time); // To ensure that the timer has not expired since we have read both g_global_time and OSW_GetTime		
	}
		
	return thread_elapsed_time;
}
