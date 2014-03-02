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
#include "os_sched.h"

#if OS_ENABLE_CPU_STATS==1

void _OS_StatInit(void);
OS_Return _OS_GetStatCounters(OS_StatCounters * ptr);
OS_Return _OS_GetTaskStatCounters(OS_Task_t task, OS_TaskStatCounters * ptr);

#endif

#endif // _OS_STAT_H
