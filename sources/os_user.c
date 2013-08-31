///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	os_user.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: User Space functions
//	
///////////////////////////////////////////////////////////////////////////////

#include "usr/includes/os_api.h"
#include "usr/includes/os_syscall.h"

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
	UINT32 ret[1];
	OS_Error result;
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_SEM_ALLOC;
	param_info.version = SYSCALL_VER_1_0;
	param_info.arg_bytes = sizeof(arg);
	param_info.ret_bytes = sizeof(ret);
	
	arg[0] = value;	
	result = _OS_Syscall(&param_info, &arg, &ret, SYSCALL_BASIC);
	
	// Store the return value
	*sem = (OS_Sem) ret[0];
		
	return result;	
}

OS_Error OS_SemWait(OS_Sem sem)
{
	_OS_Syscall_Args param_info;
	UINT32 arg[1];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_SEM_WAIT;
	param_info.version = SYSCALL_VER_1_0;
	param_info.arg_bytes = sizeof(arg);
	param_info.ret_bytes = 0;
	
	arg[0] = sem;	
	return _OS_Syscall(&param_info, &arg, NULL, SYSCALL_SWITCHING);
}

OS_Error OS_SemPost(OS_Sem sem)
{
	_OS_Syscall_Args param_info;
	UINT32 arg[1];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_SEM_POST;
	param_info.version = SYSCALL_VER_1_0;
	param_info.arg_bytes = sizeof(arg);
	param_info.ret_bytes = 0;
	
	arg[0] = sem;	
	return _OS_Syscall(&param_info, &arg, NULL, SYSCALL_SWITCHING);
}

OS_Error OS_SemFree(OS_Sem sem)
{
	_OS_Syscall_Args param_info;
	UINT32 arg[1];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_SEM_FREE;
	param_info.version = SYSCALL_VER_1_0;
	param_info.arg_bytes = sizeof(arg);
	param_info.ret_bytes = 0;
	
	arg[0] = sem;	
	return _OS_Syscall(&param_info, &arg, NULL, SYSCALL_SWITCHING);
}

OS_Error OS_SemGetValue(OS_Sem sem, INT32 *val)
{
	_OS_Syscall_Args param_info;
	UINT32 arg[1];
	UINT32 ret[1];
	OS_Error result;
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_SEM_GET_VALUE;
	param_info.version = SYSCALL_VER_1_0;
	param_info.arg_bytes = sizeof(arg);
	param_info.ret_bytes = sizeof(ret);
	
	arg[0] = sem;	
	result = _OS_Syscall(&param_info, &arg, &ret, SYSCALL_BASIC);
	
	// Store the return value
	if(val) {
		*val = ret[0];
	}
		
	return result;	
}
