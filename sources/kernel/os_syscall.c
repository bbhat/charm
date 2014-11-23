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
#include "os_stat.h"
#include "os_driver.h"
#include "../usr/includes/os_syscall.h"

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
static void syscall_TaskComplete(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static void syscall_SetUserLED(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static void syscall_OSGetStat(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static void syscall_TaskGetStat(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static void syscall_GetTaskAllocMask(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static void syscall_DriverStandardCall(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static void syscall_DriverCustomCall(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static void syscall_GetCurProcess(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static void syscall_MapPhysicalMem(const _OS_Syscall_Args * param_info, const void * arg, void * ret);
static void syscall_UnmapMem(const _OS_Syscall_Args * param_info, const void * arg, void * ret);

//////////////////////////////////////////////////////////////////////////////
// Other function prototypes
//////////////////////////////////////////////////////////////////////////////
void _OS_TaskYield(void);
OS_Return _PFM_SetUserLED(LED_Number led, LED_Options options);
OS_Return _OS_GetTaskAllocMask(UINT32 * alloc_mask, UINT32 count, UINT32 starting_task);

OS_Task * g_current_task;

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
		syscall_TaskComplete,
		syscall_OSGetStat,
		syscall_TaskGetStat,
		syscall_GetTaskAllocMask,
		syscall_DriverStandardCall, 
		syscall_DriverCustomCall,
		syscall_GetCurProcess,
		syscall_MapPhysicalMem,
		syscall_UnmapMem,
		0, 0, 0, 0, 
		0, 0, 0, 0, 
		0, 0, 0, 0, 
		syscall_SetUserLED
	};

///////////////////////////////////////////////////////////////////////////////
// Kernel Side of the Syscall function
///////////////////////////////////////////////////////////////////////////////
void _OS_KernelSyscall(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{	
	if(!param_info || (param_info->id >= SYSCALL_MAX_COUNT) || !_syscall_handlers[param_info->id])
	{
		KlogStr(KLOG_WARNING, "Error occurred in Kernel function %s", __FUNCTION__);
		if(ret) ((UINT32 *)ret)[0] = SYSCALL_ARGUMENT_ERROR;
		return;
	}
	
	Klog32(KLOG_SYSCALL, "Syscall Id - ", param_info->id);
	
	// Note down the return pointer which may be needed to update the output parameters
	if(g_current_task) g_current_task->syscall_result = (UINT32 *)ret;
	_syscall_handlers[param_info->id](param_info, arg, ret);
}

void syscall_PeriodicTaskCreate(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	const UINT32 * uint_args = (const UINT32 *)arg;
	UINT32 * uint_ret = (UINT32 *)ret;
	OS_Return result = SYSCALL_ARGUMENT_ERROR;
	
	if((param_info->arg_count >= 9) && (param_info->ret_count >= 2))
	{	
		result = _OS_CreatePeriodicTask((UINT32)uint_args[0],
									(UINT32)uint_args[1],
									(UINT32)uint_args[2],
									(UINT32)uint_args[3],
									(UINT32 *)uint_args[4],
									(UINT32)uint_args[5],
									(INT8 *)uint_args[6],
									USER_TASK,
									(OS_Task_t *)(uint_ret+1),
									(void *)uint_args[7],
									(void *)uint_args[8]);
	}
								
	if(uint_ret) uint_ret[0] = result;
}

void syscall_TaskYield(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	_OS_TaskYield();
}

void syscall_TaskComplete(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	UINT32 * uint_ret = (UINT32 *)ret;
	
	// Complete current aperiodic task. This removes the task from future scheduling
	// and moves it into permanent blocked queue
	OS_Return result = _OS_CompleteAperiodicTask();
	
	if(uint_ret) uint_ret[0] = result;
}

void syscall_AperiodicTaskCreate(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	const UINT32 * uint_args = (const UINT32 *)arg;
	UINT32 * uint_ret = (UINT32 *)ret;
	OS_Return result = SYSCALL_ARGUMENT_ERROR;
	
	if((param_info->arg_count >= 6) && (param_info->ret_count >= 2))
	{
		result = _OS_CreateAperiodicTask((UINT32)uint_args[0],
								(UINT32 *)uint_args[1],
								(UINT32)uint_args[2],
								(INT8 *)uint_args[3],
								USER_TASK,
								(OS_Task_t *)(uint_ret+1),
								(void *)uint_args[4],
								(void *)uint_args[5]);
	}
	
	if(uint_ret) uint_ret[0] = result;
}

void syscall_ProcessCreate(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	if(ret) ((UINT32 *)ret)[0] = SYSCALL_ARGUMENT_ERROR;
}

void syscall_ProcessCreateFromFile(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	if(ret) ((UINT32 *)ret)[0] = SYSCALL_ARGUMENT_ERROR;
}

void syscall_GetCurProcess(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	UINT32 * uint_ret = (UINT32 *)ret;
	
	if(uint_ret && (param_info->ret_count >= 1))
	{
		ASSERT(g_kernel_process);
		uint_ret[0] = g_kernel_process->id;
	}
}

void syscall_MapPhysicalMem(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	const UINT32 * uint_args = (const UINT32 *)arg;
	UINT32 * uint_ret = (UINT32 *)ret;
	OS_Return result = SYSCALL_ARGUMENT_ERROR;
	do
	{
		if((param_info->arg_count < 4) && (param_info->ret_count < 2)) break;
		
		OS_Process_t proc = (OS_Process_t) uint_args[0];
		if(proc >= MAX_PROCESS_COUNT) break;
		
		OS_Process *pcb = &g_process_pool[proc];
		if(!pcb) break;		
	
#if ENABLE_MMU

		_MMU_PTE_AccessPermission ap;
		UINT32 attr = uint_args[3];
		switch(attr & MMAP_PROT_MASK)
		{
		case MMAP_PROT_READ_ONLY:
			ap = KERNEL_RO_USER_RO;
			break;
			
		case MMAP_PROT_READ_WRITE:
			ap = KERNEL_RW_USER_RW;
			break;
			
		case MMAP_PROT_NO_ACCESS:
		default:
			ap = KERNEL_NA_USER_NA;
			break;
		}

		BOOL cacheable = ((attr & MMAP_CACHE_MASK) == MMAP_CACHEABLE);
		BOOL write_Buffer = ((attr & MMAP_WRITE_BUFFER_MASK) == MMAP_WRITE_BUFFER_ENABLE);
		
#if USER_PAGE_SIZE==4

		// Create a map in the user process
		result = _MMU_add_l2_small_page_va_to_pa_map(pcb->ptable,
								uint_args[1],
								uint_args[1],
								uint_args[2], 
								ap, cacheable, write_Buffer);

#elif USER_PAGE_SIZE==64

		// Create a map in the user process
		result = _MMU_add_l2_large_page_va_to_pa_map(pcb->ptable,
								uint_args[1],
								uint_args[1],
								uint_args[2], 
								ap, cacheable, write_Buffer);

#else
	#error "Supported values for USER_PAGE_SIZE is either 4 or 64"
#endif
		// Store the Virtual address we have used (we are using Virtual Address == Physical Address)
		uint_ret[1] = uint_args[1];
#endif // ENABLE_MMU	

	} while(0);
	
	if(uint_ret) uint_ret[0] = result;
}

void syscall_UnmapMem(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	const UINT32 * uint_args = (const UINT32 *)arg;
	UINT32 * uint_ret = (UINT32 *)ret;
	OS_Return result = SYSCALL_ARGUMENT_ERROR;
	do
	{
		if(param_info->arg_count < 3) break;
		
		OS_Process_t proc = (OS_Process_t) uint_args[0];
		if(proc >= MAX_PROCESS_COUNT) break;
		
		OS_Process *pcb = &g_process_pool[proc];
		if(!pcb) break;		
	
#if ENABLE_MMU
#if USER_PAGE_SIZE==4

		// Create a map in the user process
		result = _MMU_add_l2_small_page_va_to_pa_map(pcb->ptable,
								uint_args[1],
								uint_args[1],
								uint_args[2], 
								KERNEL_NA_USER_NA, FALSE, FALSE);

#elif USER_PAGE_SIZE==64

		// Create a map in the user process
		result = _MMU_add_l2_large_page_va_to_pa_map(pcb->ptable,
								uint_args[1],
								uint_args[1],
								uint_args[2], 
								KERNEL_NA_USER_NA, FALSE, FALSE);

#else
	#error "Supported values for USER_PAGE_SIZE is either 4 or 64"
#endif
#endif // ENABLE_MMU	

	} while(0);
	
	if(uint_ret) uint_ret[0] = result;
}

void syscall_SemAlloc(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	const UINT32 * uint_args = (const UINT32 *)arg;
	UINT32 * uint_ret = (UINT32 *)ret;
	OS_Return result = SYSCALL_ARGUMENT_ERROR;
	
	if((param_info->arg_count >= 2) && (param_info->ret_count >= 2))
	{
		result = _OS_SemAlloc((OS_Sem_t *)(uint_ret+1), uint_args[0], uint_args[1]);
	}
	
	if(uint_ret) uint_ret[0] = result;
}

void syscall_SemWait(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	const UINT32 * uint_args = (const UINT32 *)arg;
	OS_Return result = SYSCALL_ARGUMENT_ERROR;
	
	if(param_info->arg_count >= 1)
	{		
		result = _OS_SemWait(uint_args[0]);
	}
	
	if(ret) ((UINT32 *)ret)[0] = result;
}

void syscall_SemPost(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	const UINT32 * uint_args = (const UINT32 *)arg;
	OS_Return result = SYSCALL_ARGUMENT_ERROR;
	
	if(param_info->arg_count >= 1)
	{
		result = _OS_SemPost(uint_args[0]);
	}
	
	if(ret) ((UINT32 *)ret)[0] = result;
}

void syscall_SemFree(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	const UINT32 * uint_args = (const UINT32 *)arg;
	OS_Return result = SYSCALL_ARGUMENT_ERROR;
	
	if(param_info->arg_count >= 1)
	{
		result = _OS_SemFree(uint_args[0]);
	}
	
	if(ret) ((UINT32 *)ret)[0] = result;
}

void syscall_SemGetValue(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	const UINT32 * uint_args = (const UINT32 *)arg;
	UINT32 * uint_ret = (UINT32 *)ret;
	OS_Return result = SYSCALL_ARGUMENT_ERROR;
	
	if((param_info->arg_count >= 1) && (param_info->ret_count >= 2))
	{
		_OS_SemGetValue(uint_args[0], (uint_ret+1));
	}
	
	if(uint_ret) uint_ret[0] = result;
}

void syscall_GetCurTask(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	// TODO: Implement this function
}

///////////////////////////////////////////////////////////////////////////////
// Function: PFM_SetUserLED
// The mask indicates which LEDs should be turned ON/OFF/Toggled depending on 
// the options provided
///////////////////////////////////////////////////////////////////////////////
void syscall_SetUserLED(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	const UINT32 * uint_args = (const UINT32 *)arg;
	
	if(param_info->arg_count >= 2)
	{
		_PFM_SetUserLED((UINT32)uint_args[0], (UINT32)uint_args[1]);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Statistics functions
///////////////////////////////////////////////////////////////////////////////
void syscall_OSGetStat(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	UINT32 * uint_ret = (UINT32 *)ret;
	
#if OS_ENABLE_CPU_STATS==1	
	const UINT32 * uint_args = (const UINT32 *)arg;
	OS_Return result = SYSCALL_ARGUMENT_ERROR;

	if(param_info->arg_count >= 1)
	{
		result = _OS_GetStatCounters((OS_StatCounters *)uint_args[0]);
	}
	
	if(uint_ret) uint_ret[0] = result;
#else
	if(uint_ret) uint_ret[0] = NOT_CONFIGURED;	
#endif
}

void syscall_TaskGetStat(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	UINT32 * uint_ret = (UINT32 *)ret;
#if OS_ENABLE_CPU_STATS==1
	const UINT32 * uint_args = (const UINT32 *)arg;
	OS_Return result = SYSCALL_ARGUMENT_ERROR;
	
	if(param_info->arg_count >= 2)
	{
		result = _OS_GetTaskStatCounters((OS_Task_t )uint_args[0], (OS_TaskStatCounters *)uint_args[1]);
	}
	
	if(uint_ret) uint_ret[0] = result;
#else
	if(uint_ret) uint_ret[0] = NOT_CONFIGURED;	
#endif	
}

void syscall_GetTaskAllocMask(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
	UINT32 * uint_ret = (UINT32 *)ret;
#if OS_ENABLE_CPU_STATS==1
	const UINT32 * uint_args = (const UINT32 *)arg;
	OS_Return result = SYSCALL_ARGUMENT_ERROR;
	
	if(param_info->arg_count >= 3)
	{
		result = _OS_GetTaskAllocMask((UINT32 *)uint_args[0], uint_args[1], uint_args[2]);
	}
	
	if(uint_ret) uint_ret[0] = result;
#else
	if(uint_ret) uint_ret[0] = NOT_CONFIGURED;	
#endif
}

void syscall_DriverStandardCall(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
    const UINT32 * uint_args = (const UINT32 *)arg;
	UINT32 * uint_ret = (UINT32 *)ret;
	OS_Return result = SYSCALL_ARGUMENT_ERROR;
	
	Klog32(KLOG_SYSCALL, "Syscall SubId - ", param_info->sub_id);
	
	switch(param_info->sub_id)
	{
    case SUBCALL_DRIVER_LOOKUP:
        if((param_info->arg_count >= 1) && (param_info->ret_count >= 2))
        {
        	result = _OS_DriverLookup((const INT8 *)uint_args[0], (OS_Driver_t *)(uint_ret+1));
        }
        break;
        
    case SUBCALL_DRIVER_OPEN:
        if(param_info->arg_count >= 2)
        {
        	result = _OS_DriverOpen((OS_Driver_t) uint_args[0], (OS_DriverAccessMode) uint_args[1]);
        }
        break;
            
    case SUBCALL_DRIVER_CLOSE:
        if(param_info->arg_count >= 1)
        {
        	result = _OS_DriverClose((OS_Driver_t) uint_args[0]);
        }
        break;
            
    case SUBCALL_DRIVER_READ:
        if((param_info->arg_count >= 4) && (param_info->ret_count >= 2))
        {
        	uint_ret[1] = uint_args[2];
        	result = _OS_DriverRead((OS_Driver_t) uint_args[0], (void *) uint_args[1], 
        							&uint_ret[1], (BOOL) uint_args[3]);
        }
        break;
        
    case SUBCALL_DRIVER_WRITE:
        if((param_info->arg_count >= 4) && (param_info->ret_count >= 2))
        {
        	uint_ret[1] = uint_args[2];
        	result = _OS_DriverWrite((OS_Driver_t) uint_args[0], (const void *) uint_args[1], 
        							&uint_ret[1], (BOOL) uint_args[3]);
        }
        break;
        
    case SUBCALL_DRIVER_CONFIGURE:
    
        break;
	}
	
	if(uint_ret) uint_ret[0] = result;
}

void syscall_DriverCustomCall(const _OS_Syscall_Args * param_info, const void * arg, void * ret)
{
}
