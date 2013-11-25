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

// Syscall Parameter structure for most used, non-varying syscalls
_OS_Syscall_Args task_yield_params = {
		.id = SYSCALL_TASK_YIELD,
		.version = SYSCALL_VER_1_0,
		.arg_bytes = 0,
		.ret_bytes = 0
	};

///////////////////////////////////////////////////////////////////////////////
// This is the main entry function for all user periodic functions           //
// ********** IMPORTANT: This is a User Mode function ***********************//
///////////////////////////////////////////////////////////////////////////////
void UserTaskEntryMain(void (*entry_function)(void *pdata), void *pdata)
{
	while(1)
	{
		// Call the thread handler function
		entry_function(pdata);
		
		_OS_Syscall(&task_yield_params, NULL, NULL, SYSCALL_SWITCHING);	
	}
}

void AperiodicUserTaskEntry(void (*entry_function)(void *pdata), void *pdata)
{
	// Call the thread handler function
	entry_function(pdata);
		
	while(1)
	{		
		_OS_Syscall(&task_yield_params, NULL, NULL, SYSCALL_SWITCHING);	
	}
	
}

///////////////////////////////////////////////////////////////////////////////
// Function to yield
// Can be used with both Periodic / Aperiodic Tasks
///////////////////////////////////////////////////////////////////////////////
void OS_TaskYield()
{	
	// This system call will result in context switch, so call advanced version
	_OS_Syscall(&task_yield_params, NULL, NULL, SYSCALL_SWITCHING);	
}

///////////////////////////////////////////////////////////////////////////////
// Semaphore Functions
///////////////////////////////////////////////////////////////////////////////
OS_Error OS_SemAlloc(OS_Sem *sem, UINT32 value)
{
	_OS_Syscall_Args param_info;
	UINT32 arg[1];
	UINT32 ret[2];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_SEM_ALLOC;
	param_info.version = SYSCALL_VER_1_0;
	param_info.arg_bytes = sizeof(arg);
	param_info.ret_bytes = sizeof(ret);
	
	arg[0] = value;	
	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_BASIC);
	
	// Store the return value
	*sem = (OS_Sem) ret[1];
		
	return (OS_Error) ret[0];
}

OS_Error OS_SemWait(OS_Sem sem)
{
	_OS_Syscall_Args param_info;
	UINT32 arg[1];
	UINT32 ret;
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_SEM_WAIT;
	param_info.version = SYSCALL_VER_1_0;
	param_info.arg_bytes = sizeof(arg);
	param_info.ret_bytes = sizeof(ret);
	
	arg[0] = sem;	
	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_SWITCHING);
	
	return (OS_Error) ret;
}

OS_Error OS_SemPost(OS_Sem sem)
{
	_OS_Syscall_Args param_info;
	UINT32 arg[1];
	UINT32 ret;
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_SEM_POST;
	param_info.version = SYSCALL_VER_1_0;
	param_info.arg_bytes = sizeof(arg);
	param_info.ret_bytes = sizeof(ret);
	
	arg[0] = sem;	
	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_SWITCHING);
	
	return (OS_Error) ret;
}

OS_Error OS_SemFree(OS_Sem sem)
{
	_OS_Syscall_Args param_info;
	UINT32 arg[1];
	UINT32 ret;
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_SEM_FREE;
	param_info.version = SYSCALL_VER_1_0;
	param_info.arg_bytes = sizeof(arg);
	param_info.ret_bytes = sizeof(ret);
	
	arg[0] = sem;	
	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_SWITCHING);
	
	return (OS_Error) ret;
}

OS_Error OS_SemGetValue(OS_Sem sem, INT32 *val)
{
	_OS_Syscall_Args param_info;
	UINT32 arg[1];
	UINT32 ret[2];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_SEM_GET_VALUE;
	param_info.version = SYSCALL_VER_1_0;
	param_info.arg_bytes = sizeof(arg);
	param_info.ret_bytes = sizeof(ret);
	
	arg[0] = sem;	
	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_BASIC);
	
	// Store the return value
	if(val) {
		*val = ret[1];
	}
		
	return (OS_Error) ret[0];
}

void PFM_SetUserLED(LED_Number led, LED_Options options)
{
	_OS_Syscall_Args param_info;
	UINT32 arg[2];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_PFM_LED_SET;
	param_info.version = SYSCALL_VER_1_0;
	param_info.arg_bytes = sizeof(arg);
	param_info.ret_bytes = 0;
	
	arg[0] = led;
	arg[1] = options;
	
	_OS_Syscall(&param_info, &arg, NULL, SYSCALL_BASIC);
}

///////////////////////////////////////////////////////////////////////////////
// Function: PFM_SerialLog
///////////////////////////////////////////////////////////////////////////////
UINT32 PFM_SerialLog(const INT8 * str, UINT32 size)
{
	_OS_Syscall_Args param_info;
	void * arg[2];
	UINT32 ret[1];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_PFM_SERIAL_LOG;
	param_info.version = SYSCALL_VER_1_0;
	param_info.arg_bytes = sizeof(arg);
	param_info.ret_bytes = sizeof(ret);
	
	arg[0] = (void *)str;
	arg[1] = size;	
	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_BASIC);
			
	return ret[0];	
}

///////////////////////////////////////////////////////////////////////////////
// Statistics Functions
///////////////////////////////////////////////////////////////////////////////
OS_Error OS_GetStatCounters(OS_StatCounters * ptr)
{
	_OS_Syscall_Args param_info;
	void * arg[1];
	UINT32 ret[1];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_OS_GET_STAT;
	param_info.version = SYSCALL_VER_1_0;
	param_info.arg_bytes = sizeof(arg);
	param_info.ret_bytes = sizeof(ret);
	
	arg[0] = (void *)ptr;	
	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_BASIC);
			
	return (OS_Error) ret[0];
}


OS_Error OS_GetTaskStatCounters(OS_Task task, OS_TaskStatCounters * ptr)
{
	_OS_Syscall_Args param_info;
	void * arg[2];
	UINT32 ret[1];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_TASK_GET_STAT;
	param_info.version = SYSCALL_VER_1_0;
	param_info.arg_bytes = sizeof(arg);
	param_info.ret_bytes = sizeof(ret);
	
	arg[0] = (void *)task;
	arg[1] = (void *)ptr;	
	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_BASIC);
			
	return (OS_Error) ret[0];	
}

OS_Error OS_GetTaskAllocMask(UINT32 * alloc_mask, UINT32 count, UINT32 starting_task)
{
	_OS_Syscall_Args param_info;
	void * arg[3];
	UINT32 ret[1];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_GET_TASK_ALLOC_MASK;
	param_info.version = SYSCALL_VER_1_0;
	param_info.arg_bytes = sizeof(arg);
	param_info.ret_bytes = sizeof(ret);
	
	arg[0] = (void *)alloc_mask;
	arg[1] = (void *)count;
	arg[2] = (void *)starting_task;

	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_BASIC);
		
	return (OS_Error) ret[0];	
}

UINT64 OS_GetElapsedTime()
{
	// TODO
	return NOT_IMPLEMENTED;
}
