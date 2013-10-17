///////////////////////////////////////////////////////////////////////////////
//
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	os_sem.c
//	Author: Bala B.
//	Description: OS Statistics related functions
//
///////////////////////////////////////////////////////////////////////////////

#include "os_api.h"
#include "printf.h"

#define OS_STAT_TASK_STACK_SIZE           0x400        // In Words
#define STAT_TASK_PERIOD                  5000000     // 5 sec
#define MAX_TASK_COUNT					  64

static OS_Task  g_stat_task;
static UINT32 	g_stat_task_stack [OS_STAT_TASK_STACK_SIZE];
static UINT64 	g_previous_idle_time;
static UINT64 	g_previous_timestamp;
static const UINT32 MAX_INDEX = (MAX_TASK_COUNT >> 5);
static UINT32	g_task_alloc_mask[MAX_TASK_COUNT >> 5];

typedef struct
{
	UINT64 task_time_us;
	UINT64 total_time_us;
		
} Task_stat;

Task_stat prev_stat[MAX_TASK_COUNT];

void ShowTaskStatistics(void);

static __inline__ UINT32 clz(UINT32 input)
{
	unsigned int result;
	
	__asm__ volatile("clz %0, %1" : "=r" (result) : "r" (input));
	
	return result;
}

///////////////////////////////////////////////////////////////////////////////
// Statistics task
// This runs every 100ms and recomputes the current CPU utilization
///////////////////////////////////////////////////////////////////////////////
void StatisticsTaskFn(void * ptr)
{
	OS_StatCounters os_stat;
	UINT32 cpu_load;
	
	if(OS_GetStatCounters(&os_stat) == SUCCESS)
	{
		UINT32 idle_time = (UINT32)(os_stat.idle_time_us - g_previous_idle_time); 
		UINT32 duration = (UINT32)(os_stat.total_time_us - g_previous_timestamp);
	
		g_previous_idle_time = os_stat.idle_time_us ;
		g_previous_timestamp = os_stat.total_time_us;
		
		cpu_load = (UINT32)(((duration - idle_time) * 100.0/ duration));

		printf("\n\nSTAT: Total CPU %u\%\, Max Scheduler Time %u us", cpu_load, os_stat.max_scheduler_elapsed_us);
		
		// Show task specific statistics
		ShowTaskStatistics();
	}
}

void ShowTaskStatistics()
{
	static BOOL initialized = FALSE;
	int i;
	UINT32 mask;
	OS_TaskStatCounters stat;
	OS_Error status;

	if(!initialized)
	{
		OS_GetTaskAllocMask(g_task_alloc_mask, MAX_INDEX, 0);
		initialized = TRUE;
	}
	
	// Print the heading
	printf("\nId               Name CPU        TBE      Dline");
		
	for(i = MAX_INDEX - 1; i >= 0; i--)
	{
		mask = g_task_alloc_mask[i];
		while(mask)
		{
			UINT32 item = (i << 5) + (31 - clz(mask));
			mask &= ~(1 << (item & 0x1f));
			OS_Task task = (OS_Task)item;
			status = OS_GetTaskStatCounters(task, &stat);
			
			if(status == SUCCESS)
			{
				UINT32 task_time = (UINT32) (stat.task_time_us - prev_stat[item].task_time_us);
				UINT32 total_time = (UINT32) (stat.total_time_us - prev_stat[item].total_time_us);
				UINT32 cpu_load = (UINT32)((task_time * 100.0 / total_time));

				prev_stat[item].task_time_us = stat.task_time_us;
				prev_stat[item].total_time_us = stat.total_time_us;

				printf("\n[%2u] %16s %2u\%", item, stat.name, cpu_load);
				
				if(stat.period > 0)
				{
					printf(" %10u %10u", stat.TBE_count, stat.dline_miss_count);	
				}
			}
			else
			{
				printf("\nError %d getting statistics for task %d", status, item);
			}
		}
	}
}


int main(int argc, char *argv[])
{
    OS_CreatePeriodicTask(STAT_TASK_PERIOD, STAT_TASK_PERIOD, 
        STAT_TASK_PERIOD / 50, STAT_TASK_PERIOD, g_stat_task_stack, sizeof(g_stat_task_stack), 
        "STATISTICS", 
        &g_stat_task, StatisticsTaskFn, 0);
        
	return 0;
}
