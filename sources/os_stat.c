///////////////////////////////////////////////////////////////////////////////
//
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	os_sem.c
//	Author: Bala B.
//	Description: OS Statistics related functions
//
///////////////////////////////////////////////////////////////////////////////

#include "target.h"
#include "os_stat.h"
#include "os_timer.h"

#if OS_ENABLE_CPU_STATS==1

extern OS_AperiodicTask * g_idle_task;
UINT64 _OS_GetElapsedTime();

///////////////////////////////////////////////////////////////////////////////
// Statistics variable initialization
///////////////////////////////////////////////////////////////////////////////
void _OS_StatInit(void)
{
	g_max_scheduler_elapsed_count = 0;
	g_periodic_timer_intr_counter = 0;
	g_budget_timer_intr_counter = 0;
}

OS_Error _OS_GetStatCounters(OS_StatCounters * ptr)
{
	if(!ptr)
		return INVALID_ARG;
	
	// This function can only be called by process with admin previleges.
	if(!(g_current_process->attributes & ADMIN_PROCESS))
		return NOT_ADMINISTRATOR;
	
	ptr->idle_time_us = g_idle_task->accumulated_budget;
	ptr->total_time_us = _OS_GetElapsedTime();
	ptr->max_scheduler_elapsed_us = CONVERT_TMR0_TICKS_TO_us(g_max_scheduler_elapsed_count);
	ptr->periodic_timer_intr_counter = g_periodic_timer_intr_counter;
	ptr->budget_timer_intr_counter = g_budget_timer_intr_counter;
	
	g_max_scheduler_elapsed_count = 0;	// Reset this every time this function is called
	
	return SUCCESS;
}

OS_Error _OS_GetTaskStatCounters(OS_Task *task, OS_TaskStatCounters * ptr)
{
	if(!ptr)
		return INVALID_ARG;
	
	// This function can only be called by process with admin previleges.
	if(!(g_current_process->attributes & ADMIN_PROCESS))
		return NOT_ADMINISTRATOR;
	
	// TODO: Implement this function
	
	ptr->task_time_us = 0;
	ptr->total_time_us = 0;
	ptr->period = 0;
	ptr->budget = 0;
	ptr->exec_count = 0;
	ptr->TBE_count = 0;
	ptr->dline_miss_count = 0;
	
	return SUCCESS;	
}

#endif // OS_ENABLE_CPU_STATS
