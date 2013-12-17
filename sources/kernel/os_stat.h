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
extern UINT32 g_max_scheduler_elapsed_count;
extern UINT32 g_periodic_timer_intr_counter;
extern UINT32 g_budget_timer_intr_counter;

void _OS_StatInit(void);
OS_Return _OS_GetStatCounters(OS_StatCounters * ptr);
OS_Return _OS_GetTaskStatCounters(OS_Task_t task, OS_TaskStatCounters * ptr);

#endif

#endif // _OS_STAT_H
