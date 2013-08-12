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
