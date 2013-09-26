///////////////////////////////////////////////////////////////////////////////
//
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	os_syscall.c
//	Author: Bala B.
//	Description: Kernel Mode implementation of system calls
//
///////////////////////////////////////////////////////////////////////////////

#include "os_core.h"
#include "os_sem.h"
#include "usr/includes/os_syscall.h"

//////////////////////////////////////////////////////////////////////////////
// Function prototypes
//////////////////////////////////////////////////////////////////////////////
static void syscall_PeriodicTaskCreate(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static void syscall_AperiodicTaskCreate(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static void syscall_ProcessCreate(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static void syscall_ProcessCreateFromFile(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static void syscall_SemAlloc(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static void syscall_SemWait(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static void syscall_SemPost(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static void syscall_SemFree(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static void syscall_SemGetValue(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static void syscall_GetCurTask(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static void syscall_TaskYield(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static void syscall_SetUserLED(const _OS_Syscall_Args * param_info, const void * arg, void * ret);

//////////////////////////////////////////////////////////////////////////////
// Other function prototypes
//////////////////////////////////////////////////////////////////////////////
void _OS_TaskYield(void);
OS_Error _PFM_SetUserLED(LED_Number led, LED_Options options);

OS_GenericTask * g_current_task;

//////////////////////////////////////////////////////////////////////////////
// Vector containing all syscall handlers
//////////////////////////////////////////////////////////////////////////////
typedef void (*Syscall_handler)(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static Syscall_handler _syscall_handlers[SYSCALL_MAX_COUNT] = {
		
		// The order of these functions should match the order of enums in os_syscall.h
		syscall_PeriodicTaskCreate,
		syscall_AperiodicTaskCreate,
		syscall_ProcessCreate,
		syscall_ProcessCreateFromFile,
		syscall_SemAlloc,
		syscall_SemWait,
		syscall_SemPost,
		syscall_SemFree,
		syscall_SemGetValue,
		syscall_GetCurTask,
		syscall_TaskYield, 
		0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0,
		0, syscall_SetUserLED
	};

///////////////////////////////////////////////////////////////////////////////
// Kernel Side of the Syscall function
///////////////////////////////////////////////////////////////////////////////
void _OS_KernelSyscall(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{	
	if(!param_info || (param_info->id >= SYSCALL_MAX_COUNT) || !_syscall_handlers[param_info->id])
	{
		KlogStr(KLOG_WARNING, "Error occurred in Kernel function %s", __FUNCTION__);
		if(ret) ((UINT32 *)ret)[0] = SYSCALL_ERROR;
		return;
	}
	
	// Note down the result pointer which may be needed
	if(g_current_task) g_current_task->syscall_result = (UINT32 *)ret;
	_syscall_handlers[param_info->id](param_info, arg, ret);
}

static void syscall_PeriodicTaskCreate(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	const UINT32 * uint_args = (const UINT32 *)arg;
	UINT32 * uint_ret = (UINT32 *)ret;
	OS_Error result = SYSCALL_ERROR;
	
	if(((param_info->arg_bytes >> 2) >= 9) && ((param_info->ret_bytes >> 2) >= 2))
	{	
		result = _OS_CreatePeriodicTask((UINT32)uint_args[0],
									(UINT32)uint_args[1],
									(UINT32)uint_args[2],
									(UINT32)uint_args[3],
									(UINT32 *)uint_args[4],
									(UINT32)uint_args[5],
									(INT8 *)uint_args[6],
									USER_TASK,
									(OS_Task *)(uint_ret+1),
									(void *)uint_args[7],
									(void *)uint_args[8]);
	}
								
	if(uint_ret) uint_ret[0] = result;
}

static void syscall_TaskYield(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	_OS_TaskYield();
}

static void syscall_AperiodicTaskCreate(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	const UINT32 * uint_args = (const UINT32 *)arg;
	UINT32 * uint_ret = (UINT32 *)ret;
	OS_Error result = SYSCALL_ERROR;
	
	if(((param_info->arg_bytes >> 2) >= 6) && ((param_info->ret_bytes >> 2) >= 2))
	{
		result = _OS_CreateAperiodicTask((UINT32)uint_args[0],
								(UINT32 *)uint_args[1],
								(UINT32)uint_args[2],
								(INT8 *)uint_args[3],
								USER_TASK,
								(OS_Task *)(uint_ret+1),
								(void *)uint_args[4],
								(void *)uint_args[5]);
	}
	
	if(uint_ret) uint_ret[0] = result;
}

static void syscall_ProcessCreate(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	if(ret) ((UINT32 *)ret)[0] = SYSCALL_ERROR;
}

static void syscall_ProcessCreateFromFile(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	if(ret) ((UINT32 *)ret)[0] = SYSCALL_ERROR;
}

static void syscall_SemAlloc(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	const UINT32 * uint_args = (const UINT32 *)arg;
	UINT32 * uint_ret = (UINT32 *)ret;
	OS_Error result = SYSCALL_ERROR;
	
	if(((param_info->arg_bytes >> 2) >= 1) && ((param_info->ret_bytes >> 2) >= 2))
	{
		result = _OS_SemAlloc((OS_Sem *)(uint_ret+1), uint_args[0]);
	}
	
	if(uint_ret) uint_ret[0] = result;
}

static void syscall_SemWait(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	const UINT32 * uint_args = (const UINT32 *)arg;
	OS_Error result = SYSCALL_ERROR;
	
	if((param_info->arg_bytes >> 2) >= 1)
	{		
		result = _OS_SemWait(uint_args[0]);
	}
	
	if(ret) ((UINT32 *)ret)[0] = result;
}

static void syscall_SemPost(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	const UINT32 * uint_args = (const UINT32 *)arg;
	OS_Error result = SYSCALL_ERROR;
	
	if((param_info->arg_bytes >> 2) >= 1)
	{
		result = _OS_SemPost(uint_args[0]);
	}
	
	if(ret) ((UINT32 *)ret)[0] = result;
}

static void syscall_SemFree(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	const UINT32 * uint_args = (const UINT32 *)arg;
	OS_Error result = SYSCALL_ERROR;
	
	if((param_info->arg_bytes >> 2) >= 1)
	{
		result = _OS_SemFree(uint_args[0]);
	}
	
	if(ret) ((UINT32 *)ret)[0] = result;
}

static void syscall_SemGetValue(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	const UINT32 * uint_args = (const UINT32 *)arg;
	UINT32 * uint_ret = (UINT32 *)ret;
	OS_Error result = SYSCALL_ERROR;
	
	if(((param_info->arg_bytes >> 2) >= 1) && ((param_info->ret_bytes >> 2) >= 2))
	{
		_OS_SemGetValue(uint_args[0], (uint_ret+1));
	}
	
	if(uint_ret) uint_ret[0] = result;
}

static void syscall_GetCurTask(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	return SYSCALL_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
// Function: PFM_SetUserLED
// The mask indicates which LEDs should be turned ON/OFF/Toggled depending on 
// the options provided
///////////////////////////////////////////////////////////////////////////////
static void syscall_SetUserLED(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	const UINT32 * uint_args = (const UINT32 *)arg;
	
	if((param_info->arg_bytes >> 2) >= 2)
	{
		_PFM_SetUserLED((UINT32)uint_args[0], (UINT32)uint_args[1]);
	}
}
