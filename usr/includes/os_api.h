///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	os_api.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: OS API Header file. Declares library functions used to call OS APIs
//	
///////////////////////////////////////////////////////////////////////////////

#ifndef _OS_API_H
#define _OS_API_H

#include "os_types.h"

///////////////////////////////////////////////////////////////////////////////
// OS Error Codes
///////////////////////////////////////////////////////////////////////////////
typedef enum
{
	SUCCESS = 0,
	FAILED = 1,

	INSUFFICIENT_STACK = 10,
	INVALID_PERIOD = 11,
	INVALID_BUDGET = 12,
	INVALID_TASK = 13,
	EXCEEDS_MAX_CPU = 14,
	NOT_SUPPORTED = 15,
	ARGUMENT_ERROR = 16,
	OUT_OF_SPACE = 17,
	NO_DATA = 18,
	INVALID_PRIORITY = 19,
	OUT_OF_BOUNDS = 20,
	CHKPT_NOT_ENABLED = 21,
	CHKPT_FAIL = 22,
	INVALID_ARG = 23, 
	INVALID_DEADLINE = 24,
	FORMAT_ERROR = 25,

	UNKNOWN = 99	

} OS_Error;

///////////////////////////////////////////////////////////////////////////////
// OS Data types
///////////////////////////////////////////////////////////////////////////////
typedef INT32 _OS_KernelObj_Handle;

typedef _OS_KernelObj_Handle 	OS_Task;
typedef _OS_KernelObj_Handle 	OS_Process;
typedef _OS_KernelObj_Handle	OS_Sem;
typedef _OS_KernelObj_Handle	OS_Mutex;

///////////////////////////////////////////////////////////////////////////////
// Task creation APIs
///////////////////////////////////////////////////////////////////////////////
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
	void *pdata);

OS_Error OS_CreateAperiodicTask(
	UINT16 priority,				// Smaller the number, higher the priority
	UINT32 *stack,
	UINT32 stack_size_in_bytes,
	const INT8 * task_name,
	OS_Task *task,
	void (*task_entry_function)(void *pdata),
	void *pdata);

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
	);

// OS_CreateProcessFromFile:
// API for creating a process given its path in the file system
// Input:
//		process_name: pointer to the process name
//		exec_path: Path to the process executable file. 
//			The exec file should be in ELF format
OS_Error OS_CreateProcessFromFile(
		OS_Process *process,
		const INT8 * process_name,
		const INT8 * exec_path,
		void *pdata
	);

// Future functions


///////////////////////////////////////////////////////////////////////////////
// Semaphore functions
///////////////////////////////////////////////////////////////////////////////
OS_Error OS_SemCreate(OS_Sem *sem, INT16 pshared, UINT32 value);
OS_Error OS_SemWait(OS_Sem *sem);
OS_Error OS_SemPost(OS_Sem *sem);
OS_Error OS_SemDestroy(OS_Sem *sem);
OS_Error OS_SemGetvalue(OS_Sem *sem, INT32 *val);

///////////////////////////////////////////////////////////////////////////////
// Mutex functions
///////////////////////////////////////////////////////////////////////////////
OS_Error OS_MutexCreate(OS_Mutex *mutex);
OS_Error OS_MutexLock(OS_Mutex *mutex);
OS_Error OS_MutexUnlock(OS_Mutex *mutex);
OS_Error OS_MutexDestroy(OS_Mutex *mutex);

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

/*
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
// Get Task Budget Exceeded count
///////////////////////////////////////////////////////////////////////////////
UINT32 OS_GetTBECount();

OS_Error OS_GetDateAndTime(OS_DateAndTime *date_and_time);
OS_Error OS_SetDateAndTime(const OS_DateAndTime *date_and_time);
OS_Error OS_GetTime(OS_Time *time);
OS_Error OS_SetAlarm(const OS_DateAndTime *date_and_time);
OS_Error OS_GetAlarm(OS_DateAndTime *date_and_time);
*/

///////////////////////////////////////////////////////////////////////////////
// Some platform utilities
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
OS_Error PFM_SetUserLED(LED_Number led, LED_Options options);

#endif // _OS_API_H
