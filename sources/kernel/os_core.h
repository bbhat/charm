///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	os_core.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Header file for the OS APIs
//	
///////////////////////////////////////////////////////////////////////////////

#ifndef _OS_CORE_H
#define _OS_CORE_H

#include "os_config.h"
#include "os_types.h"

///////////////////////////////////////////////////////////////////////////////
// OS Error Codes
///////////////////////////////////////////////////////////////////////////////
typedef enum
{
	SUCCESS = 0,
	FAILED = -1,

	INSUFFICIENT_STACK = -10,
	INVALID_PERIOD = -11,
	INVALID_BUDGET = -12,
	INVALID_TASK = -13,
	EXCEEDS_MAX_CPU = -14,
	NOT_SUPPORTED = -15,
	BAD_ARGUMENT = -16,
	OUT_OF_SPACE = -17,
	NO_DATA = -18,
	INVALID_PRIORITY = -19,
	OUT_OF_BOUNDS = -20,
	CHKPT_NOT_ENABLED = -21,
	CHKPT_FAIL = -22,
	INVALID_ARG = -23, 
	INVALID_DEADLINE = -24,
	FORMAT_ERROR = -25,
	RAMDISK_INVALID = -26,
	CONFIGURATION_ERROR = -27,
	INVALID_ELF_FILE = -28,
	FILE_ERROR = -29,
	SYSCALL_ERROR = -30,
	INVALID_SWI_ERROR = -30,
	RESOURCE_EXHAUSTED = -31,
	PROCESS_INVALID = -32,			// Certain functions can only be called from a valid process
	RESOURCE_NOT_OPEN = -33,
	RESOURCE_NOT_OWNED = -34,
	RESOURCE_DELETED = -35,
	RESOURCE_NOT_FOUND = -36,
	NOT_ADMINISTRATOR = -37,
	NOT_IMPLEMENTED = -38,
	NOT_CONFIGURED = -39,
	EXCEEDED_MAX_SECTIONS = -40,
	INVALID_PHASE = -41,
    EXCLUSIVE_ACCESS = -42,
    ACCESS_DENIED = -43,
    DEFER_IO_REQUEST = -44,
    RESOURCE_BUSY = -45,
	
	
	UNKNOWN = -99	

} OS_Return;


typedef enum
{
    ACCESS_READ = 1,		// Allow Reading
    ACCESS_WRITE = 2,		// Allow writing
    ACCESS_EXCLUSIVE = 4	// Allow exclusive access for one client. The type of access (Read / Write) should be specified separately
    
} OS_DriverAccessMode;

#include "os_process.h"
#include "os_task.h"
#include "os_sem.h"

UINT32 _disable_interrupt();
void _enable_interrupt(UINT32);

#define  OS_ENTER_CRITICAL(x)	x = _disable_interrupt()
#define  OS_EXIT_CRITICAL(x)	_enable_interrupt(x)

#if ENABLE_ASSERTS==1
#define ASSERT(x)		if(!(x)) panic("ASSSERT Failed %s\n", #x);
#else
#define ASSERT(x)
#endif

#if ENABLE_ASSERTS==1
#define ASSERT_ALWAYS(x)		panic("ASSSERT_ALWAYS: %s\n", x);
#else
#define ASSERT_ALWAYS(x)
#endif

///////////////////////////////////////////////////////////////////////////////
// Misc OS Data types
///////////////////////////////////////////////////////////////////////////////
typedef void (*OS_InterruptVector)(void *);

///////////////////////////////////////////////////////////////////////////////
// Global Data
///////////////////////////////////////////////////////////////////////////////
extern BOOL _OS_IsRunning;
extern OS_GenericTask * g_current_task;

///////////////////////////////////////////////////////////////////////////////
// Task creation APIs
// The period and phase parameters should be multiple of SMALLEST_TASK_PERIOD
// Deadline and Budget can be anything valid (in microseconds)
///////////////////////////////////////////////////////////////////////////////
OS_Return OS_CreatePeriodicTask(
	UINT32 period_in_us,
	UINT32 deadline_in_us,
	UINT32 budget_in_us,
	UINT32 phase_shift_in_us,
	UINT32 * stack,
	UINT32 stack_size_in_bytes,
	const INT8 * task_name,
	OS_Task_t *task,
	void (*periodic_entry_function)(void *pdata),
	void *pdata);

OS_Return OS_CreateAperiodicTask(
	UINT16 priority,				// Smaller the number, higher the priority
	UINT32 * stack,
	UINT32 stack_size_in_bytes,
	const INT8 * task_name,
	OS_Task_t *task,
	void (*task_entry_function)(void *pdata),
	void *pdata);

///////////////////////////////////////////////////////////////////////////////
// Process creation APIs
// Using processes is optional. It is possible to create tasks under the default 
// kernel process and do everything in those tasks if desired.
///////////////////////////////////////////////////////////////////////////////

// OS_CreateProcess:
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
		void *pdata
	);

// OS_CreateProcessFromFile:
// API for creating a process given its path in the file system
// Input:
//		process_name: pointer to the process name
//		exec_path: Path to the process executable file. 
//			The exec file should be in ELF format
OS_Return OS_CreateProcessFromFile(
		OS_Process_t *process,
		const INT8 * process_name,
		UINT16 attributes,
		const INT8 * exec_path,
		void *pdata
	);

///////////////////////////////////////////////////////////////////////////////
// The following function sleeps for the specified duration of time. 
// Note: the sleep duration has only 250uSec resolution
///////////////////////////////////////////////////////////////////////////////
void OS_Sleep(UINT32 interval_in_us);

///////////////////////////////////////////////////////////////////////////////
// The below function, gets the total elapsed time since the beginning
// of the system in microseconds.
///////////////////////////////////////////////////////////////////////////////
UINT64 OS_GetElapsedTime();

///////////////////////////////////////////////////////////////////////////////
// The following function gets the total time taken by the current
// thread since the thread has begun. Note that this is not the global 
// time, this is just the time taken from only the current thread.
// Note that this function is defined only for periodic tasks.
// For aperiodic tasks, this function will return zero.
///////////////////////////////////////////////////////////////////////////////
UINT64 OS_GetThreadElapsedTime();

///////////////////////////////////////////////////////////////////////////////
// Semaphore functions
///////////////////////////////////////////////////////////////////////////////
OS_Return OS_SemAlloc(OS_Sem_t *sem, UINT32 value);
OS_Return OS_SemWait(OS_Sem_t sem);
OS_Return OS_SemPost(OS_Sem_t sem);
OS_Return OS_SemFree(OS_Sem_t sem);
OS_Return OS_SemGetValue(OS_Sem_t sem, INT32 *val);

///////////////////////////////////////////////////////////////////////////////
// Function to get the currently running thread. It returns a void pointer 
// which may be used as a Periodic / Aperiodic Task pointers
///////////////////////////////////////////////////////////////////////////////
void * OS_GetCurrentTask();

///////////////////////////////////////////////////////////////////////////////
// Function to yield
// Can be used with both Periodic / Aperiodic Tasks
///////////////////////////////////////////////////////////////////////////////
void OS_TaskYield();

///////////////////////////////////////////////////////////////////////////////
// Statistics functions
///////////////////////////////////////////////////////////////////////////////
typedef struct
{
	UINT64 idle_time_us;
	UINT64 total_time_us;
	UINT32 max_scheduler_elapsed_us;		// This will be reset each time, its value is read
	UINT32 periodic_timer_intr_counter;
	UINT32 budget_timer_intr_counter;
	
} OS_StatCounters;

typedef struct
{
	UINT64 task_time_us;
	UINT64 total_time_us;
	INT8 name[16];
	UINT32 period;
	UINT32 budget;
	UINT32 exec_count;
	UINT32 TBE_count;
	UINT32 dline_miss_count;
	
} OS_TaskStatCounters;

// Get global OS statistics. This function can be called only from Admin process
// Non-admin tasks will get NOT_ADMINISTRATOR error
OS_Return OS_GetStatCounters(OS_StatCounters * ptr);

// Get Task specific statistics. This function can be called only from Admin process
// Non-admin tasks will get NOT_ADMINISTRATOR error
OS_Return OS_GetTaskStatCounters(OS_Task_t task, OS_TaskStatCounters * ptr);

// Get global Task allocation mask. This function can be called only from Admin process
// Non-admin tasks will get NOT_ADMINISTRATOR error
// alloc_mask - is a bit mask indicating which task numbers are under use.
// count - The number of elements in the task_usage_mask array
// starting_task - The task_number where we are requesting the bit_mask.
//                 This number will be truncated to multiples of 32
// For example, if starting_task=32 and count = 1, then we will get the task_usage_mask 
// from 32 - 63. If starting_task < 32, it will be truncated to 0. 
// If (starting_task >=32 && starting_task < 64), it will be truncated to 32
OS_Return OS_GetTaskAllocMask(UINT32 * alloc_mask, UINT32 count, UINT32 starting_task);

///////////////////////////////////////////////////////////////////////////////
// Get Task Budget Exceeded count
///////////////////////////////////////////////////////////////////////////////
UINT32 OS_GetTBECount();

///////////////////////////////////////////////////////////////////////////////
// Function to set Interrupt Vector for a given index
// This function returns the old interrupt vector
///////////////////////////////////////////////////////////////////////////////
OS_InterruptVector OS_SetInterruptVector(OS_InterruptVector isr, UINT32 index);

///////////////////////////////////////////////////////////////////////////////
// Some platform specific utilitynfunctions
///////////////////////////////////////////////////////////////////////////////
typedef enum 
{
	USER_LED0 = 0,
	USER_LED1,
	USER_LED2,
	USER_LED3
} LED_Number;

typedef enum
{
	LED_ON = 0,
	LED_OFF = 1,
	LED_TOGGLE = 2
} LED_Options;

// The led parameter indicates which LED should be turned ON/OFF/Toggled depending on 
// the options provided
OS_Return PFM_SetUserLED(LED_Number led, LED_Options options);

///////////////////////////////////////////////////////////////////////////////
// Utility functions & Macros
///////////////////////////////////////////////////////////////////////////////
void panic(const INT8 *format, ...);
void SyslogStr(const INT8 * str, const INT8 * value);
#define Syslog(str)	SyslogStr(str, NULL)
void Syslog32(const INT8 * str, UINT32 value);
void Syslog64(const INT8 * str, UINT64 value);

#if OS_KERNEL_LOGGING == 1

#define KlogStr(mask, str, val) \
	do { \
		if(mask & OS_KLOG_MASK) \
			SyslogStr((str), (val)); \
	} while(0)

#define Klog32(mask, str, val) \
	do { \
		if(mask & OS_KLOG_MASK) \
			Syslog32((str), (val)); \
	} while(0)

#define Klog64(mask, str, val) \
	do { \
		if(mask & OS_KLOG_MASK) \
			Syslog64((str), (val)); \
	} while(0)
#else

#define KlogStr(level, str, val)
#define Klog32(level, str, val)
#define Klog64(level, str, val)

#endif // OS_KERNEL_LOGGING

#endif // _OS_CORE_H
