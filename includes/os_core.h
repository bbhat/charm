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
	RAMDISK_INVALID = 26,
	CONFIGURATION_ERROR = 27,
	INVALID_ELF_FILE = 28,
	
	UNKNOWN = 99	

} OS_Error;

#include "os_process.h"
#include "os_task.h"
#include "os_sem.h"
#include "os_mutex.h"

UINT32 _disable_interrupt();
void _enable_interrupt(UINT32);

#define  OS_ENTER_CRITICAL(x)	x = _disable_interrupt()
#define  OS_EXIT_CRITICAL(x)	_enable_interrupt(x)

#if OS_ENABLE_ASSERTS==1
#define ASSERT(x)	if(!(x)) panic("ASSSERT Failed %s\n", #x);
#else
#define ASSERT(x)
#endif

///////////////////////////////////////////////////////////////////////////////
// Misc OS Data codes
///////////////////////////////////////////////////////////////////////////////
typedef void (*OS_InterruptVector)(void *);

///////////////////////////////////////////////////////////////////////////////
// Global Data
///////////////////////////////////////////////////////////////////////////////
extern BOOL _OS_IsRunning;
extern volatile void * g_current_task;

///////////////////////////////////////////////////////////////////////////////
// Task creation APIs
///////////////////////////////////////////////////////////////////////////////
OS_Error OS_CreatePeriodicTask(
	UINT32 period_in_us,
	UINT32 deadline_in_us,
	UINT32 budget_in_us,
	UINT32 phase_shift_in_us,
	UINT32 * stack,
	UINT32 stack_size_in_bytes,
#if OS_WITH_TASK_NAME==1
	const INT8 * task_name,
#endif //OS_WITH_TASK_NAME	
	OS_PeriodicTask *task,
	void (*periodic_entry_function)(void *pdata),
	void *pdata);

OS_Error OS_CreateAperiodicTask(
	UINT16 priority,				// Smaller the number, higher the priority
	UINT32 * stack,
	UINT32 stack_size_in_bytes,
#if OS_WITH_TASK_NAME==1
	const INT8 * task_name,
#endif //OS_WITH_TASK_NAME		
	OS_AperiodicTask *task,
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

///////////////////////////////////////////////////////////////////////////////
// The following funcstion starts the OS scheduling
// Note that this function never returns
///////////////////////////////////////////////////////////////////////////////
void OS_Start();

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

///////////////////////////////////////////////////////////////////////////////
// Date and Time functions
///////////////////////////////////////////////////////////////////////////////
#if ENABLE_RTC==1

#include "rtc.h"

OS_Error OS_GetDateAndTime(OS_DateAndTime *date_and_time);
OS_Error OS_SetDateAndTime(const OS_DateAndTime *date_and_time);
OS_Error OS_GetTime(OS_Time *time);
#if ENABLE_RTC_ALARM==1
OS_Error OS_SetAlarm(const OS_DateAndTime *date_and_time);
OS_Error OS_GetAlarm(OS_DateAndTime *date_and_time);
#endif // ENABLE_RTC_ALARM
#endif // ENABLE_RTC

///////////////////////////////////////////////////////////////////////////////
// Semaphore functions
///////////////////////////////////////////////////////////////////////////////
OS_Error OS_SemInit(OS_Sem *sem, INT16 pshared, UINT32 value);
OS_Error OS_SemWait(OS_Sem *sem);
OS_Error OS_SemPost(OS_Sem *sem);
OS_Error OS_SemDestroy(OS_Sem *sem);
OS_Error OS_SemGetvalue(OS_Sem *sem, INT32 *val);

///////////////////////////////////////////////////////////////////////////////
// Mutex functions
///////////////////////////////////////////////////////////////////////////////
OS_Error OS_MutexInit(OS_Mutex *mutex);
OS_Error OS_MutexLock(OS_Mutex *mutex);
OS_Error OS_MutexUnlock(OS_Mutex *mutex);
OS_Error OS_MutexDestroy(OS_Mutex *mutex);

///////////////////////////////////////////////////////////////////////////////
// Function to get the currently running thread. It returns a void pointer 
// which may be used as a Periodic / Aperiodic Task pointers
///////////////////////////////////////////////////////////////////////////////
void * OS_GetCurrentTask();

///////////////////////////////////////////////////////////////////////////////
// Function to set Interrupt Vector for a given index
// This function returns the old interrupt vector
///////////////////////////////////////////////////////////////////////////////
OS_InterruptVector OS_SetInterruptVector(OS_InterruptVector isr, UINT32 index);

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
