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

#define ARRAYSIZE(arg)	(sizeof(arg) / sizeof(arg[0]))

OS_Return OS_CreatePeriodicTask(
	UINT32 period_in_us,
	UINT32 deadline_in_us,
	UINT32 budget_in_us,
	UINT32 phase_shift_in_us,
	UINT32 *stack,
	UINT32 stack_size_in_bytes,
	const INT8 * task_name,
	OS_Task_t *task,
	void (*periodic_entry_function)(void *pdata),
	void *pdata)
{
	_OS_Syscall_Args param_info;
	UINT32 arg[9];
	UINT32 ret[2];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_PERIODIC_TASK_CREATE;
	param_info.sub_id = 0;
	param_info.arg_count = ARRAYSIZE(arg);
	param_info.ret_count = ARRAYSIZE(ret);
	
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
	*task = (OS_Task_t) ret[1];
	
	return (OS_Return) ret[0];
}

OS_Return OS_CreateAperiodicTask(
	UINT16 priority,				// Smaller the number, higher the priority
	UINT32 *stack,
	UINT32 stack_size_in_bytes,
	const INT8 * task_name,
	OS_Task_t *task,
	void (*task_entry_function)(void *pdata),
	void *pdata)
{
	_OS_Syscall_Args param_info;
	UINT32 arg[6];
	UINT32 ret[2];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_APERIODIC_TASK_CREATE;
	param_info.sub_id = 0;
	param_info.arg_count = ARRAYSIZE(arg);
	param_info.ret_count = ARRAYSIZE(ret);
	
	arg[0] = priority;
	arg[1] = (UINT32)stack;
	arg[2] = stack_size_in_bytes;
	arg[3] = (UINT32)task_name;
	arg[4] = (UINT32)task_entry_function;
	arg[5] = (UINT32)pdata;
	
	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_BASIC);
	
	// Store the return value
	*task = (OS_Task_t) ret[1];
	
	return (OS_Return) ret[0];
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
OS_Return OS_CreateProcess(
		OS_Process_t *process,
		const INT8 * process_name,
		UINT16 attributes,
		void (*process_entry_function)(void *pdata),
		void *pdata)
{
	// TODO: Implement this function
	return SUCCESS;
}

// Function for getting current process handle
OS_Process_t OS_GetCurrentProcess()
{
	_OS_Syscall_Args param_info;
	UINT32 ret[1];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_GET_CUR_PROC;
	param_info.sub_id = 0;
	param_info.arg_count = 0;
	param_info.ret_count = ARRAYSIZE(ret);
	
	_OS_Syscall(&param_info, NULL, &ret, SYSCALL_BASIC);	
	
	return ret[0];
}

///////////////////////////////////////////////////////////////////////////////
// Memory Mapping functions
///////////////////////////////////////////////////////////////////////////////

OS_VirtualAddr OS_MapPhysicalMemory(
		OS_Process_t process,
		OS_PhysicalAddr paddr,
		UINT32 size,
		UINT32 attr,
		OS_Return * result)
{
	_OS_Syscall_Args param_info;
	UINT32 arg[4];
	UINT32 ret[2];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_MAP_PHYSICAL_MEM;
	param_info.sub_id = 0;
	param_info.arg_count = ARRAYSIZE(arg);
	param_info.ret_count = ARRAYSIZE(ret);
	
	arg[0] = process;	
	arg[1] = paddr;	
	arg[2] = size;	
	arg[3] = attr;
	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_BASIC);
	
	*result = (OS_Return) ret[0];		
	return (OS_VirtualAddr) ret[1];
}

OS_Return OS_UnmapMemory(
		OS_Process_t process,
		OS_VirtualAddr vaddr,
		UINT32 size)
{
	_OS_Syscall_Args param_info;
	UINT32 arg[3];
	UINT32 ret[1];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_UNMAP_MEM;
	param_info.sub_id = 0;
	param_info.arg_count = ARRAYSIZE(arg);
	param_info.ret_count = ARRAYSIZE(ret);
	
	arg[0] = process;	
	arg[1] = (UINT32) vaddr;	
	arg[2] = size;
	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_BASIC);
	
	return (OS_Return) ret[0];
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
	param_info.sub_id = 0;
	param_info.arg_count = ARRAYSIZE(arg);
	param_info.ret_count = 0;
	
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
	param_info.sub_id = 0;
	param_info.arg_count = 0;
	param_info.ret_count = 0;
	
	// This system call will result in context switch, so call advanced version
	_OS_Syscall(&param_info, NULL, NULL, SYSCALL_SWITCHING);	
}

///////////////////////////////////////////////////////////////////////////////
// Semaphore Functions
///////////////////////////////////////////////////////////////////////////////
OS_Return OS_SemAlloc(OS_Sem_t *sem, UINT32 value, BOOL binary)
{
	_OS_Syscall_Args param_info;
	UINT32 arg[2];
	UINT32 ret[2];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_SEM_ALLOC;
	param_info.sub_id = 0;
	param_info.arg_count = ARRAYSIZE(arg);
	param_info.ret_count = ARRAYSIZE(ret);
	
	arg[0] = value;	
	arg[1] = binary;	
	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_BASIC);
	
	// Store the return value
	*sem = (OS_Sem_t) ret[1];
		
	return (OS_Return) ret[0];	
}

OS_Return OS_SemWait(OS_Sem_t sem)
{
	_OS_Syscall_Args param_info;
	UINT32 arg[1];
	UINT32 ret[1];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_SEM_WAIT;
	param_info.sub_id = 0;
	param_info.arg_count = ARRAYSIZE(arg);
	param_info.ret_count = ARRAYSIZE(ret);
	
	arg[0] = sem;	
	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_SWITCHING);
	
	return (OS_Return) ret[0];
}

OS_Return OS_SemPost(OS_Sem_t sem)
{
	_OS_Syscall_Args param_info;
	UINT32 arg[1];
	UINT32 ret[1];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_SEM_POST;
	param_info.sub_id = 0;
	param_info.arg_count = ARRAYSIZE(arg);
	param_info.ret_count = ARRAYSIZE(ret);
	
	arg[0] = sem;
	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_SWITCHING);
	
	return (OS_Return) ret[0];
}

OS_Return OS_SemFree(OS_Sem_t sem)
{
	_OS_Syscall_Args param_info;
	UINT32 arg[1];
	UINT32 ret[0];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_SEM_FREE;
	param_info.sub_id = 0;
	param_info.arg_count = ARRAYSIZE(arg);
	param_info.ret_count = ARRAYSIZE(ret);
	
	arg[0] = sem;	
	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_SWITCHING);
	
	return (OS_Return) ret[0];
}

OS_Return OS_SemGetValue(OS_Sem_t sem, INT32 *val)
{
	_OS_Syscall_Args param_info;
	UINT32 arg[1];
	UINT32 ret[2];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_SEM_GET_VALUE;
	param_info.sub_id = 0;
	param_info.arg_count = ARRAYSIZE(arg);
	param_info.ret_count = ARRAYSIZE(ret);
	
	arg[0] = sem;	
	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_BASIC);
	
	// Store the return value
	if(val) {
		*val = ret[1];
	}
		
	return (OS_Return) ret[0];
}

///////////////////////////////////////////////////////////////////////////////
// Statistics Functions
///////////////////////////////////////////////////////////////////////////////
OS_Return OS_GetStatCounters(OS_StatCounters * ptr)
{
	_OS_Syscall_Args param_info;
	void * arg[1];
	UINT32 ret[1];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_OS_GET_STAT;
	param_info.sub_id = 0;
	param_info.arg_count = ARRAYSIZE(arg);
	param_info.ret_count = ARRAYSIZE(ret);
	
	arg[0] = (void *)ptr;	
	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_BASIC);
			
	return (OS_Return) ret[0];
}

OS_Return OS_GetTaskStatCounters(OS_Task_t task, OS_TaskStatCounters * ptr)
{
	_OS_Syscall_Args param_info;
	void * arg[2];
	UINT32 ret[1];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_TASK_GET_STAT;
	param_info.sub_id = 0;
	param_info.arg_count = ARRAYSIZE(arg);
	param_info.ret_count = ARRAYSIZE(ret);
	
	arg[0] = (void *)task;
	arg[1] = (void *)ptr;	
	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_BASIC);
			
	return (OS_Return) ret[0];	
}

OS_Return OS_GetTaskAllocMask(UINT32 * alloc_mask, UINT32 count, UINT32 starting_task)
{
	_OS_Syscall_Args param_info;
	void * arg[3];
	UINT32 ret[1];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_GET_TASK_ALLOC_MASK;
	param_info.sub_id = 0;
	param_info.arg_count = ARRAYSIZE(arg);
	param_info.ret_count = ARRAYSIZE(ret);
	
	arg[0] = (void *)alloc_mask;
	arg[1] = (void *)count;
	arg[2] = (void *)starting_task;

	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_BASIC);
		
	return (OS_Return) ret[0];	
}

OS_Return OS_DriverLookup(const INT8 * driver_name, OS_Driver_t * driver)
{
	_OS_Syscall_Args param_info;
	void * arg[1];
	UINT32 ret[2];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_DRIVER_STANDARD_CALL;
	param_info.sub_id = SUBCALL_DRIVER_LOOKUP;
	param_info.arg_count = ARRAYSIZE(arg);
	param_info.ret_count = ARRAYSIZE(ret);
	
	arg[0] = (void *)driver_name;

	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_BASIC);
	
	if(driver) {
	    *driver = ret[1];
	}
	
	return (OS_Return) ret[0];	
}

OS_Return OS_DriverOpen(OS_Driver_t driver, OS_DriverAccessMode mode)
{
	_OS_Syscall_Args param_info;
	void * arg[2];
	UINT32 ret[1];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_DRIVER_STANDARD_CALL;
	param_info.sub_id = SUBCALL_DRIVER_OPEN;
	param_info.arg_count = ARRAYSIZE(arg);
	param_info.ret_count = ARRAYSIZE(ret);
	
	arg[0] = (void *)driver;
	arg[1] = (void *)mode;

	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_BASIC);
	
	return (OS_Return) ret[0];	
}

OS_Return OS_DriverClose(OS_Driver_t driver)
{
	_OS_Syscall_Args param_info;
	void * arg[1];
	UINT32 ret[1];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_DRIVER_STANDARD_CALL;
	param_info.sub_id = SUBCALL_DRIVER_CLOSE;
	param_info.arg_count = ARRAYSIZE(arg);
	param_info.ret_count = ARRAYSIZE(ret);
	
	arg[0] = (void *)driver;

	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_BASIC);
	
	return (OS_Return) ret[0];	
}

OS_Return OS_DriverRead(OS_Driver_t driver, void * buffer, UINT32 * size, BOOL waitOK)
{
	_OS_Syscall_Args param_info;
	void * arg[4];
	UINT32 ret[2];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_DRIVER_STANDARD_CALL;
	param_info.sub_id = SUBCALL_DRIVER_READ;
	param_info.arg_count = ARRAYSIZE(arg);
	param_info.ret_count = ARRAYSIZE(ret);
	
	arg[0] = (void *)driver;
	arg[1] = (void *)buffer;
	arg[2] = (void *)*size;
	arg[3] = (void *)waitOK;

	ret[1] = 0;
	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_SWITCHING);
	*size = ret[1];
	
	return (OS_Return) ret[0];	
}

OS_Return OS_DriverWrite(OS_Driver_t driver, const void * buffer, UINT32 * size, BOOL waitOK)
{
	_OS_Syscall_Args param_info;
	void * arg[4];
	UINT32 ret[2];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_DRIVER_STANDARD_CALL;
	param_info.sub_id = SUBCALL_DRIVER_WRITE;
	param_info.arg_count = ARRAYSIZE(arg);
	param_info.ret_count = ARRAYSIZE(ret);
	
	arg[0] = (void *)driver;
	arg[1] = (void *)buffer;
	arg[2] = (void *)*size;
	arg[3] = (void *)waitOK;
	
	ret[1] = 0;
	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_SWITCHING);
	*size = ret[1];
	
	return (OS_Return) ret[0];	
}

OS_Return OS_DriverConfigure(OS_Driver_t driver, const void * buffer, UINT32 size)
{
	_OS_Syscall_Args param_info;
	void * arg[3];
	UINT32 ret[1];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_DRIVER_STANDARD_CALL;
	param_info.sub_id = SUBCALL_DRIVER_CONFIGURE;
	param_info.arg_count = ARRAYSIZE(arg);
	param_info.ret_count = ARRAYSIZE(ret);
	
	arg[0] = (void *)driver;
	arg[1] = (void *)buffer;
	arg[2] = (void *)size;

	_OS_Syscall(&param_info, &arg, &ret, SYSCALL_BASIC);
	
	return (OS_Return) ret[0];	
}

///////////////////////////////////////////////////////////////////////////////
// Function to deal with Display
///////////////////////////////////////////////////////////////////////////////
void * OS_GetDisplayFrameBuffer(UINT32 * width, UINT32 * height, UINT32 * buffer_size)
{
	_OS_Syscall_Args param_info;
	UINT32 ret[4];
	
	// Prepare the argument info structure
	param_info.id = SYSCALL_GET_DISP_FRAME_BUFFER;
	param_info.sub_id = 0;
	param_info.arg_count = 0;
	param_info.ret_count = ARRAYSIZE(ret);
	
	_OS_Syscall(&param_info, NULL, &ret, SYSCALL_BASIC);
	
	if(width) *width = ret[1];
	if(height) *height = ret[2];
	if(buffer_size) *buffer_size = ret[3];
	
	return (void *) ret[0];
}
