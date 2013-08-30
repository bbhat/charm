///////////////////////////////////////////////////////////////////////////////
//
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	os_syscall.c
//	Author: Bala B.
//	Description: Kernel Mode implementation of system calls
//
///////////////////////////////////////////////////////////////////////////////

#include "os_core.h"
#include "usr/includes/os_syscall.h"

//////////////////////////////////////////////////////////////////////////////
// Function prototypes
//////////////////////////////////////////////////////////////////////////////
static OS_Error syscall_PeriodicTaskCreate(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static OS_Error syscall_AperiodicTaskCreate(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static OS_Error syscall_ProcessCreate(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static OS_Error syscall_ProcessCreateFromFile(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static OS_Error syscall_SemCreate(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static OS_Error syscall_SemWait(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static OS_Error syscall_SemPost(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static OS_Error syscall_SemDestroy(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static OS_Error syscall_SemGetValue(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static OS_Error syscall_GetCurTask(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static OS_Error syscall_TaskYield(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static OS_Error syscall_SetUserLED(const _OS_Syscall_Args * param_info, const void * arg, void * ret);

//////////////////////////////////////////////////////////////////////////////
// Vector containing all syscall handlers
//////////////////////////////////////////////////////////////////////////////
typedef OS_Error (*Syscall_handler)(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static Syscall_handler _syscall_handlers[SYSCALL_MAX_COUNT] = {
		
		// The order of these functions should match the order of enums in os_syscall.h
		syscall_PeriodicTaskCreate,
		syscall_AperiodicTaskCreate,
		syscall_ProcessCreate,
		syscall_ProcessCreateFromFile,
		syscall_SemCreate,
		syscall_SemWait,
		syscall_SemPost,
		syscall_SemDestroy,
		syscall_SemGetValue,
		syscall_GetCurTask,
		syscall_TaskYield, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		syscall_SetUserLED,
	};

///////////////////////////////////////////////////////////////////////////////
// Kernel Side of the Syscall function
///////////////////////////////////////////////////////////////////////////////
OS_Error _OS_KernelSyscall(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{		
	if(param_info && (param_info->id < SYSCALL_MAX_COUNT))
	{
		if(_syscall_handlers[param_info->id])
		{
			return _syscall_handlers[param_info->id](param_info, arg, ret);
		}
	}
	
	KlogStr(KLOG_WARNING, "Error occurred in Kernel function %s", __FUNCTION__);
	return SYSCALL_ERROR;
}

static OS_Error syscall_PeriodicTaskCreate(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	const UINT32 * uint_args = (const UINT32 *)arg;
	
	if(((param_info->arg_bytes >> 2) < 9) || ((param_info->ret_bytes >> 2) < 1))
	{
		return SYSCALL_ERROR;
	}
	
	return _OS_CreatePeriodicTask((UINT32)uint_args[0],
								(UINT32)uint_args[1],
								(UINT32)uint_args[2],
								(UINT32)uint_args[3],
								(UINT32 *)uint_args[4],
								(UINT32)uint_args[5],
								(INT8 *)uint_args[6],
								USER_TASK,
								(OS_PeriodicTask *)ret,
								(void *)uint_args[7],
								(void *)uint_args[8]);
}

static OS_Error syscall_TaskYield(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	_OS_TaskYield();
	return SUCCESS;
}

static OS_Error syscall_AperiodicTaskCreate(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	const UINT32 * uint_args = (const UINT32 *)arg;
	
	if(((param_info->arg_bytes >> 2) < 6) || ((param_info->ret_bytes >> 2) < 1))
	{
		return SYSCALL_ERROR;
	}
	
	return _OS_CreateAperiodicTask((UINT32)uint_args[0],
								(UINT32 *)uint_args[1],
								(UINT32)uint_args[2],
								(INT8 *)uint_args[3],
								USER_TASK,
								(OS_PeriodicTask *)ret,
								(void *)uint_args[4],
								(void *)uint_args[5]);
}

static OS_Error syscall_ProcessCreate(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	return SYSCALL_ERROR;
}

static OS_Error syscall_ProcessCreateFromFile(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	return SYSCALL_ERROR;
}

static OS_Error syscall_SemCreate(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	return SYSCALL_ERROR;
}

static OS_Error syscall_SemWait(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	return SYSCALL_ERROR;
}

static OS_Error syscall_SemPost(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	return SYSCALL_ERROR;
}

static OS_Error syscall_SemDestroy(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	return SYSCALL_ERROR;
}

static OS_Error syscall_SemGetValue(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	return SYSCALL_ERROR;
}

static OS_Error syscall_GetCurTask(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	return SYSCALL_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
// Function: PFM_SetUserLED
// The mask indicates which LEDs should be turned ON/OFF/Toggled depending on 
// the options provided
///////////////////////////////////////////////////////////////////////////////
static OS_Error syscall_SetUserLED(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	const UINT32 * uint_args = (const UINT32 *)arg;
	
	if((param_info->arg_bytes >> 2) < 2)
	{
		return SYSCALL_ERROR;
	}
	
	return PFM_SetUserLED((UINT32)uint_args[0], (UINT32)uint_args[1]);
}
