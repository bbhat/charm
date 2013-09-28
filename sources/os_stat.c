///////////////////////////////////////////////////////////////////////////////
//
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	os_sem.c
//	Author: Bala B.
//	Description: OS Statistics related functions
//
///////////////////////////////////////////////////////////////////////////////

#include "os_stat.h"
#include "os_timer.h"
#include "target.h"

#if OS_ENABLE_CPU_STATS==1

extern OS_AperiodicTask * g_idle_task;

// Some statistics counters to keep track.
UINT32 g_max_scheduler_elapsed_count;
static UINT32 stat_task_count;

static UINT64 previous_idle_time;
static UINT64 previous_timestamp;

// Variables to keep track of the idle task execution
UINT32 g_idle_max_count;
UINT32 g_idle_count;
FP32 _OS_CPUUsage;

///////////////////////////////////////////////////////////////////////////////
// Statistics variable initialization
///////////////////////////////////////////////////////////////////////////////
void _OS_StatInit(void)
{
	g_max_scheduler_elapsed_count = 0;
	periodic_timer_intr_counter = 0;
	budget_timer_intr_counter = 0;
	g_idle_max_count = 0;
	g_idle_count = 0;
}

///////////////////////////////////////////////////////////////////////////////
// Statistics task
// This runs every 100ms and recomputes the current CPU utilization
// 
///////////////////////////////////////////////////////////////////////////////
void _OS_StatisticsFn(void * ptr)
{
	if(stat_task_count++ == 0)
	{
		// Initialize the stat counters during the first invocation of the stat task. 
		// This is because we don't want to consider the affect of compulsory cache misses.
		_OS_StatInit();
		return;
	}
	
	// On most ARM platforms, floating point multiplication is cheaper than integer division
	UINT64 current_idle_time = g_idle_task->accumulated_budget;
	UINT64 current_timestamp = OS_GetElapsedTime();
	
	UINT32 idle_time = (UINT32)(current_idle_time - previous_idle_time); 
	UINT32 duration = (UINT32)(current_timestamp - previous_timestamp);
	
	previous_idle_time = g_idle_task->accumulated_budget;
	previous_timestamp = current_timestamp;
	
	UINT32 cpu_load = (UINT32)(((FP32)(duration - idle_time) / duration) * 100.0);
		
 	Syslog32("STAT: CPU Usage (%) ", cpu_load);
 	Syslog32("STAT: Max scheduler time in us = ", CONVERT_TMR0_TICKS_TO_us(g_max_scheduler_elapsed_count));
 	g_max_scheduler_elapsed_count = 0;
// 	Syslog32("STAT: periodic_timer_intr_counter = ", periodic_timer_intr_counter);
//	Syslog32("STAT: budget_timer_intr_counter = ", budget_timer_intr_counter);
	
	// TODO: This logic is now outdated as the OS uses wait_for_interrupt in idle task
// 	static UINT64 prev_elapsed_time = 0;
// 	UINT64 new_elapsed_time;
// 	UINT32 diff_time;
// 	UINT32 intsts;
// 	FP32 usage;
// 	
// 	// Capture the initial idle count for STAT_TASK_PERIOD
// 	if(!g_idle_max_count) g_idle_max_count = g_idle_count * 10;	
// 
// 	new_elapsed_time = OS_GetElapsedTime();
// 	OS_ENTER_CRITICAL(intsts);	// Enter critical section
// 	diff_time = (UINT32)(new_elapsed_time - prev_elapsed_time);
// 	
// 	// _OS_CPUUsage now indicates the free time
// 	usage = ((FP32)g_idle_count * STAT_TASK_PERIOD) / ((FP32)diff_time * g_idle_max_count);	
// 	
// 	// Now _OS_CPUUsage stands for 
// 	_OS_CPUUsage = (usage > 1.0) ? 0 : 1.0 - usage;	
// 	g_idle_count = 0;
// 	prev_elapsed_time = new_elapsed_time;
// 	OS_EXIT_CRITICAL(intsts);	// Exit critical section
}

#endif // OS_ENABLE_CPU_STATS
