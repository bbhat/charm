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
static OS_Error syscall_MutexCreate(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static OS_Error syscall_MutexLock(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static OS_Error syscall_MutexUnlock(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static OS_Error syscall_MutexDestroy(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static OS_Error syscall_GetCurTask(const _OS_Syscall_Args * param_info, const void * arg, void * ret);

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
		syscall_MutexCreate,
		syscall_MutexLock,
		syscall_MutexUnlock,
		syscall_MutexDestroy,
		syscall_GetCurTask,
	};

OS_Error _OS_Syscall(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{		
	if(param_info && (param_info->id < SYSCALL_MAX_COUNT))
	{
		if(_syscall_handlers[param_info->id])
		{
			return _syscall_handlers[param_info->id](param_info, arg, ret);
		}
	}
	
	KlogStr(KLOG_WARNING, "Error occurred in Kernel function %s", __FUNCDNAME__);
	return SYSCALL_ERROR;
}

static OS_Error syscall_PeriodicTaskCreate(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	const UINT32 * uint_args = (const UINT32 *)arg;
	UINT32 * uint_ret = (UINT32 *)ret;
	
	if(((param_info->arg_bytes >> 2) < 7) || ((param_info->ret_bytes >> 2) < 1))
	{
		return SYSCALL_ERROR;
	}
	
	return OS_CreatePeriodicTask((UINT32)uint_args[0],
								(UINT32)uint_args[1],
								(UINT32)uint_args[2],
								(UINT32)uint_args[3],
								(UINT32 *)uint_args[4],
								(UINT32)uint_args[5],
								(INT8 *)uint_args[6],
								(OS_PeriodicTask *)uint_ret[0],
								(void *)uint_args[7],
								(void *)uint_args[8]);
}

static OS_Error syscall_AperiodicTaskCreate(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	return SYSCALL_ERROR;
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

static OS_Error syscall_MutexCreate(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	return SYSCALL_ERROR;
}

static OS_Error syscall_MutexLock(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	return SYSCALL_ERROR;
}

static OS_Error syscall_MutexUnlock(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	return SYSCALL_ERROR;
}

static OS_Error syscall_MutexDestroy(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	return SYSCALL_ERROR;
}

static OS_Error syscall_GetCurTask(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	return SYSCALL_ERROR;
}
