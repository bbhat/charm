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

static OS_Task_t  g_stat_task;
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
	FP32 cpu_load;
	
	if(OS_GetStatCounters(&os_stat) == SUCCESS)
	{
		UINT32 idle_time = (UINT32)(os_stat.idle_time_us - g_previous_idle_time); 
		UINT32 duration = (UINT32)(os_stat.total_time_us - g_previous_timestamp);
	
		g_previous_idle_time = os_stat.idle_time_us ;
		g_previous_timestamp = os_stat.total_time_us;
		
		// Adding 0.005 in the below expression is to help rounding to 2 decimal places
		cpu_load = ((duration - idle_time) * 100.0 / duration) + 0.005;
		
		UINT32 int_part = (UINT32) cpu_load;
		UINT32 dec_part = (UINT32)((cpu_load - int_part) * 100.0);	// Just 2 digits are enough
		
		printf("\n\nSTAT: Total CPU %2u.%02u\%\, Max Scheduler Time %u us", 
			int_part, dec_part, os_stat.max_scheduler_elapsed_us);
		
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
	OS_Return status;

	if(!initialized)
	{
		OS_GetTaskAllocMask(g_task_alloc_mask, MAX_INDEX, 0);
		initialized = TRUE;
	}
	
	// Print the heading
	printf("\nId               Name  CPU(%)  Alloc(%)    TBE    Dline");
		
	for(i = MAX_INDEX - 1; i >= 0; i--)
	{
		mask = g_task_alloc_mask[i];
		while(mask)
		{
			UINT32 item = (i << 5) + (31 - clz(mask));
			mask &= ~(1 << (item & 0x1f));
			OS_Task_t task = (OS_Task_t)item;
			status = OS_GetTaskStatCounters(task, &stat);
			
			if(status == SUCCESS)
			{
				UINT32 task_time = (UINT32) (stat.task_time_us - prev_stat[item].task_time_us);
				UINT32 total_time = (UINT32) (stat.total_time_us - prev_stat[item].total_time_us);
				
				// Adding 0.005 in the below expression is to help rounding to 2 decimal places
				FP32 cpu_load = (task_time * 100.0 / total_time) + 0.005;	

				prev_stat[item].task_time_us = stat.task_time_us;
				prev_stat[item].total_time_us = stat.total_time_us;

				UINT32 usage_whole = (UINT32) cpu_load;
				UINT32 usage_dec = (UINT32)((cpu_load - usage_whole) * 100.0);	// Just 2 digit are enough
				
				printf("\n[%2u] %16s  %2u.%02u", item, stat.name, 
					usage_whole, usage_dec);
				
				if(stat.period > 0)
				{
					// Adding 0.005 in the below expression is to help rounding to 2 decimal places
					FP32 alloc_percent = (stat.budget * 100.0  / stat.period) + 0.005;
				
					UINT32 alloc_whole = (UINT32) alloc_percent;
					UINT32 alloc_dec = (UINT32)((alloc_percent - alloc_whole) * 100.0);	// Just 2 digit are enough				

					printf("    %2u.%02u %8u %8u", alloc_whole, alloc_dec, stat.TBE_count, stat.dline_miss_count);	
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
