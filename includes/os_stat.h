///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	os_stat.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Header file for the OS Statistics related functions
//	
///////////////////////////////////////////////////////////////////////////////

#ifndef _OS_STAT_H
#define _OS_STAT_H

#include "os_core.h"

#if OS_ENABLE_CPU_STATS==1

// Some statistics counters to keep track.
extern UINT32 max_scheduler_elapsed_time;
extern UINT32 scheduler_miss_counter;
extern UINT32 sched_intr_counter;
#if ENABLE_SYNC_TIMER==1
extern UINT32 sync_timer_miss_counter;
extern UINT32 sync_intr_counter;
#endif

// Variables to keep track of the idle task execution
extern volatile UINT32 g_idle_max_count;
extern volatile UINT32 g_idle_count;
extern volatile FP32 _OS_CPUUsage;

extern OS_PeriodicTask * g_stat_task;		// A TCB for the idle task
extern UINT32 g_stat_task_stack [OS_STAT_TASK_STACK_SIZE];

// OS Statistics Capture function
void _OS_StatInit(void);
void _OS_StatisticsFn(void * ptr);

#endif

#endif // _OS_STAT_H
