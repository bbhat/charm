///////////////////////////////////////////////////////////////////////////////
//
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	os_sem.c
//	Author: Bala B.
//	Description: OS Statistics related functions
//
///////////////////////////////////////////////////////////////////////////////

#include "os_api.h"

#define OS_STAT_TASK_STACK_SIZE           0x40        // In Words
#define STAT_TASK_PERIOD                  5000000     // 5 sec

static OS_Task  g_stat_task;
static UINT32 	g_stat_task_stack [OS_STAT_TASK_STACK_SIZE];
static UINT64 	g_previous_idle_time;
static UINT64 	g_previous_timestamp;

UINT32 cpu_load;
	
///////////////////////////////////////////////////////////////////////////////
// Statistics task
// This runs every 100ms and recomputes the current CPU utilization
// 
///////////////////////////////////////////////////////////////////////////////
void StatisticsTaskFn(void * ptr)
{
	OS_StatCounters os_stat;
	OS_StatCounters task_stat;
	
	if(OS_GetStatCounters(&os_stat) == SUCCESS)
	{
		UINT32 idle_time = (UINT32)(os_stat.idle_time_us - g_previous_idle_time); 
		UINT32 duration = (UINT32)(os_stat.total_time_us - g_previous_timestamp);
	
		g_previous_idle_time = os_stat.idle_time_us ;
		g_previous_timestamp = os_stat.total_time_us;
	
		cpu_load = (UINT32)(((FP32)(duration - idle_time) / duration) * 100.0);
		
	 	//Syslog32("STAT: CPU Usage (%) ", cpu_load);
	 	//Syslog32("STAT: Max scheduler time in us = ", CONVERT_TMR0_TICKS_TO_us(os_stat.max_scheduler_elapsed_count));		
	}
}

int main(int argc, char *argv[])
{
    OS_CreatePeriodicTask(STAT_TASK_PERIOD, STAT_TASK_PERIOD, 
        STAT_TASK_PERIOD / 50, 0, g_stat_task_stack, sizeof(g_stat_task_stack), 
        "STATISTICS", 
        &g_stat_task, StatisticsTaskFn, 0);
        
	return 0;
}
