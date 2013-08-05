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

///////////////////////////////////////////////////////////////////////////////
// This is the main entry function for all user periodic functions           //
// ********** IMPORTANT: This is a User Mode function ***********************//
///////////////////////////////////////////////////////////////////////////////
void UserTaskEntryMain(void (*entry_function)(void *pdata), void *pdata)
{
	_OS_Syscall_Args param_info;

	// Prepare the argument info structure for Task Yield
	param_info.id = SYSCALL_TASK_YIELD;
	param_info.version = SYSCALL_VER_1_0;
	param_info.arg_bytes = 0;
	param_info.ret_bytes = 0;

	while(1)
	{
		// Call the thread handler function
		entry_function(pdata);
		
		_OS_Syscall(&param_info, NULL, NULL);	
	}
}

void AperiodicUserTaskEntry(void (*entry_function)(void *pdata), void *pdata)
{
	// Call the thread handler function
	entry_function(pdata);
		
	while(1)
	{
		_OS_Syscall_Args param_info;
		
		// Prepare the argument info structure for Task Yield
		param_info.id = SYSCALL_TASK_YIELD;
		param_info.version = SYSCALL_VER_1_0;
		param_info.arg_bytes = 0;
		param_info.ret_bytes = 0;
		
		_OS_Syscall(&param_info, NULL, NULL);	
	}
	
}
