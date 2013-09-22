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
	UINT32 ret[2];
	
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
	
	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_BASIC);
	
	// Store the return value
	*task = (OS_Task) ret[1];
	
	return (OS_Error) ret[0];
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
	_OS_Syscall_Args param_info;
	UINT32 arg[6];
	UINT32 ret[2];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_APERIODIC_TASK_CREATE;
	param_info.version = SYSCALL_VER_1_0;
	param_info.arg_bytes = sizeof(arg);
	param_info.ret_bytes = sizeof(ret);
	
	arg[0] = priority;
	arg[1] = (UINT32)stack;
	arg[2] = stack_size_in_bytes;
	arg[3] = (UINT32)task_name;
	arg[4] = (UINT32)task_entry_function;
	arg[5] = (UINT32)pdata;
	
	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_BASIC);
	
	// Store the return value
	*task = (OS_Task) ret[1];
	
	return (OS_Error) ret[0];
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

///////////////////////////////////////////////////////////////////////////////
// Function: PFM_SetUserLED
// The led parameter indicates which LED should be turned ON/OFF/Toggled depending on 
// the options provided
///////////////////////////////////////////////////////////////////////////////
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
// Function to yield
// Can be used with both Periodic / Aperiodic Tasks
///////////////////////////////////////////////////////////////////////////////
void OS_TaskYield()
{
	_OS_Syscall_Args param_info;
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_TASK_YIELD;
	param_info.version = SYSCALL_VER_1_0;
	param_info.arg_bytes = 0;
	param_info.ret_bytes = 0;
	
	// This system call will result in context switch, so call advanced version
	_OS_Syscall(&param_info, NULL, NULL, SYSCALL_SWITCHING);	
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
