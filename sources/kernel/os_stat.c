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
#include "util.h"

#if OS_ENABLE_CPU_STATS==1

extern OS_Task * g_idle_task;
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

OS_Return _OS_GetStatCounters(OS_StatCounters * ptr)
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

OS_Return _OS_GetTaskStatCounters(OS_Task_t task, OS_TaskStatCounters * ptr)
{
	if(!ptr)
		return INVALID_ARG;
	
	// This function can only be called by process with admin previleges.
	if(!(g_current_process->attributes & ADMIN_PROCESS))
		return NOT_ADMINISTRATOR;
	
	if(task >= MAX_TASK_COUNT)
		return INVALID_TASK;
	
	OS_Task * tcb = &g_task_pool[task];
	
	if(!tcb)
		return INVALID_TASK;
		
	ptr->task_time_us = tcb->accumulated_budget;
	ptr->total_time_us = _OS_GetElapsedTime();
	
	strncpy(ptr->name, tcb->name, sizeof(ptr->name) - 1);
	ptr->name[sizeof(ptr->name) - 1] = '\0';
	
    if(IS_PERIODIC_TASK(tcb->attributes))
	{
		ptr->period = tcb->periodic.period;
		ptr->budget = tcb->periodic.budget;
		ptr->exec_count = tcb->periodic.exec_count;
		ptr->TBE_count = tcb->periodic.TBE_count;
		ptr->dline_miss_count = tcb->periodic.dline_miss_count;
	}
	else
	{
		ptr->period = 0;
		ptr->budget = 0;
		ptr->exec_count = 0;
		ptr->TBE_count = 0;
		ptr->dline_miss_count = 0;
	}
		
	return SUCCESS;	
}

#endif // OS_ENABLE_CPU_STATS
