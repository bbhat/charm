///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	os_user.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: User Space functions
//	
///////////////////////////////////////////////////////////////////////////////

#include "../usr/includes/os_api.h"
#include "../usr/includes/os_syscall.h"

#define ARRAYSIZE(arg)	(sizeof(arg) / sizeof(arg[0]))

///////////////////////////////////////////////////////////////////////////////
// Following are the user mode functions that are in the kernel binary. 
// Place them on a separate physical page so that we can give user level
// permissions for that page
///////////////////////////////////////////////////////////////////////////////
void UserTaskEntryMain(void (*entry_function)(void *pdata), void *pdata) 
	__attribute__ ((section (".user")));
void AperiodicUserTaskEntry(void (*entry_function)(void *pdata), void *pdata) 
	__attribute__ ((section (".user")));
	
///////////////////////////////////////////////////////////////////////////////
// This is the main entry function for all user periodic functions           //
// ********** IMPORTANT: This is a User Mode function ***********************//
///////////////////////////////////////////////////////////////////////////////
void UserTaskEntryMain(void (*entry_function)(void *pdata), void *pdata)
{
	// Syscall Parameter structure for most used, non-varying syscalls
	_OS_Syscall_Args task_yield_params = {
			.id = SYSCALL_TASK_YIELD,
			.sub_id = 0,
			.arg_count = 0,
			.ret_count = 0
		};

	while(1)
	{
		// Call the thread handler function
		entry_function(pdata);
		
		_OS_Syscall(&task_yield_params, NULL, NULL, SYSCALL_SWITCHING);	
	}
}

void AperiodicUserTaskEntry(void (*entry_function)(void *pdata), void *pdata)
{
	_OS_Syscall_Args param_info;
	UINT32 ret[1];
	
	// Call the thread handler function
	entry_function(pdata);
	
	// If the aperiodic task completes, then we should block it permanently
	// by calling complete on that task
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_TASK_COMPLETE;
	param_info.sub_id = 0;
	param_info.arg_count = 0;
	param_info.ret_count = ARRAYSIZE(ret);
	
	_OS_Syscall(&param_info, NULL, &ret, SYSCALL_SWITCHING);		
}

///////////////////////////////////////////////////////////////////////////////
// Function to yield
// Can be used with both Periodic / Aperiodic Tasks
///////////////////////////////////////////////////////////////////////////////
void OS_TaskYield()
{	
	// Syscall Parameter structure for most used, non-varying syscalls
	_OS_Syscall_Args task_yield_params = {
			.id = SYSCALL_TASK_YIELD,
			.sub_id = 0,
			.arg_count = 0,
			.ret_count = 0
		};

	// This system call will result in context switch, so call advanced version
	_OS_Syscall(&task_yield_params, NULL, NULL, SYSCALL_SWITCHING);	
}

///////////////////////////////////////////////////////////////////////////////
// Semaphore Functions
///////////////////////////////////////////////////////////////////////////////
OS_Return OS_SemAlloc(OS_Sem_t *sem, UINT32 value)
{
	_OS_Syscall_Args param_info;
	UINT32 arg[1];
	UINT32 ret[2];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_SEM_ALLOC;
	param_info.sub_id = 0;
	param_info.arg_count = ARRAYSIZE(arg);
	param_info.ret_count = ARRAYSIZE(ret);
	
	arg[0] = value;	
	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_BASIC);
	
	// Store the return value
	*sem = (OS_Sem_t) ret[1];
		
	return (OS_Return) ret[0];
}

OS_Return OS_SemWait(OS_Sem_t sem)
{
	_OS_Syscall_Args param_info;
	UINT32 arg[1];
	UINT32 ret[1];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_SEM_WAIT;
	param_info.sub_id = 0;
	param_info.arg_count = ARRAYSIZE(arg);
	param_info.ret_count = ARRAYSIZE(ret);
	
	arg[0] = sem;	
	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_SWITCHING);
	
	return (OS_Return) ret[0];
}

OS_Return OS_SemPost(OS_Sem_t sem)
{
	_OS_Syscall_Args param_info;
	UINT32 arg[1];
	UINT32 ret[1];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_SEM_POST;
	param_info.sub_id = 0;
	param_info.arg_count = ARRAYSIZE(arg);
	param_info.ret_count = ARRAYSIZE(ret);
	
	arg[0] = sem;	
	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_SWITCHING);
	
	return (OS_Return) ret[0];
}

OS_Return OS_SemFree(OS_Sem_t sem)
{
	_OS_Syscall_Args param_info;
	UINT32 arg[1];
	UINT32 ret[1];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_SEM_FREE;
	param_info.sub_id = 0;
	param_info.arg_count = ARRAYSIZE(arg);
	param_info.ret_count = ARRAYSIZE(ret);
	
	arg[0] = sem;	
	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_SWITCHING);
	
	return (OS_Return) ret[0];
}

OS_Return OS_SemGetValue(OS_Sem_t sem, INT32 *val)
{
	_OS_Syscall_Args param_info;
	UINT32 arg[1];
	UINT32 ret[2];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_SEM_GET_VALUE;
	param_info.sub_id = 0;
	param_info.arg_count = ARRAYSIZE(arg);
	param_info.ret_count = ARRAYSIZE(ret);
	
	arg[0] = sem;	
	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_BASIC);
	
	// Store the return value
	if(val) {
		*val = ret[1];
	}
		
	return (OS_Return) ret[0];
}

void PFM_SetUserLED(LED_Number led, LED_Options options)
{
	_OS_Syscall_Args param_info;
	UINT32 arg[2];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_PFM_LED_SET;
	param_info.sub_id = 0;
	param_info.arg_count = ARRAYSIZE(arg);
	param_info.ret_count = 0;
	
	arg[0] = led;
	arg[1] = options;
	
	_OS_Syscall(&param_info, &arg, NULL, SYSCALL_BASIC);
}

///////////////////////////////////////////////////////////////////////////////
// Statistics Functions
///////////////////////////////////////////////////////////////////////////////
OS_Return OS_GetStatCounters(OS_StatCounters * ptr)
{
	_OS_Syscall_Args param_info;
	void * arg[1];
	UINT32 ret[1];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_OS_GET_STAT;
	param_info.sub_id = 0;
	param_info.arg_count = ARRAYSIZE(arg);
	param_info.ret_count = ARRAYSIZE(ret);
	
	arg[0] = (void *)ptr;	
	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_BASIC);
			
	return (OS_Return) ret[0];
}


OS_Return OS_GetTaskStatCounters(OS_Task_t task, OS_TaskStatCounters * ptr)
{
	_OS_Syscall_Args param_info;
	void * arg[2];
	UINT32 ret[1];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_TASK_GET_STAT;
	param_info.sub_id = 0;
	param_info.arg_count = ARRAYSIZE(arg);
	param_info.ret_count = ARRAYSIZE(ret);
	
	arg[0] = (void *)task;
	arg[1] = (void *)ptr;	
	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_BASIC);
			
	return (OS_Return) ret[0];	
}

OS_Return OS_GetTaskAllocMask(UINT32 * alloc_mask, UINT32 count, UINT32 starting_task)
{
	_OS_Syscall_Args param_info;
	void * arg[3];
	UINT32 ret[1];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_GET_TASK_ALLOC_MASK;
	param_info.sub_id = 0;
	param_info.arg_count = ARRAYSIZE(arg);
	param_info.ret_count = ARRAYSIZE(ret);
	
	arg[0] = (void *)alloc_mask;
	arg[1] = (void *)count;
	arg[2] = (void *)starting_task;

	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_BASIC);
		
	return (OS_Return) ret[0];	
}

UINT64 OS_GetElapsedTime()
{
	// TODO
	return NOT_IMPLEMENTED;
}
