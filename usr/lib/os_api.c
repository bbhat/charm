///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	os_api.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: OS API Source file. Declares library functions used to call OS APIs
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_api.h"
#include "os_syscall.h"

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
	_OS_Syscall_Args param_info;
	UINT32 arg[9];
	UINT32 ret[1];
	OS_Error result;
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_PERIODIC_TASK_CREATE;
	param_info.version = SYSCALL_VER_1_0;
	param_info.arg_bytes = sizeof(arg);
	param_info.ret_bytes = sizeof(ret);
	
	arg[0] = period_in_us;
	arg[1] = deadline_in_us;
	arg[2] = budget_in_us;
	arg[3] = phase_shift_in_us;
	arg[4] = (UINT32)stack;
	arg[5] = stack_size_in_bytes;
	arg[6] = (UINT32)task_name;
	arg[7] = (UINT32)periodic_entry_function;
	arg[8] = (UINT32)pdata;
	
	result = _OS_Syscall(&param_info, &arg, &ret);
	
	// Store the return value
	*task = (OS_Task) ret[0];
	
	return result;
}

OS_Error OS_CreateAperiodicTask(
	UINT16 priority,				// Smaller the number, higher the priority
	UINT32 *stack,
	UINT32 stack_size_in_bytes,
	const INT8 * task_name,
	OS_Task *task,
	void (*task_entry_function)(void *pdata),
	void *pdata)
{
	return SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// Process creation APIs
// Using processes is optional. It is possible to create tasks under the default 
// kernel process and do everything in those tasks if desired.
///////////////////////////////////////////////////////////////////////////////

// API for creating a process. 
// Input:
//		process_name: pointer to the process name
//		process_entry_function: This is the pointer to the process entry function.
//			The process_entry_function should initialize all process wide data structures
//			and create all tasks
OS_Error OS_CreateProcess(
		OS_Process *process,
		const INT8 * process_name,
		void (*process_entry_function)(void *pdata),
		void *pdata
	)
{
	return SUCCESS;
}
