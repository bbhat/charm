///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	os_queue.c
//	Author: Bala B.
//	Description: Main Scheduling function implementation
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_core.h"
#include "os_queue.h"
#include "os_timer.h"
#include "os_stat.h"
#include "util.h"
#include "sysctl.h"

_OS_Queue g_ready_q;
_OS_Queue g_wait_q;
_OS_Queue g_ap_ready_q;
_OS_Queue g_block_q;

// This global variable can be accessed from outside
BOOL _OS_IsRunning = FALSE;

volatile UINT64 g_global_time;		// This variable gets updated everytime the Timer ISR is called.
volatile UINT64 g_next_wakeup_time; 	// This variable holds the next scheduled wakeup time in uSecs
#if ENABLE_SYNC_TIMER==1
static UINT64 g_next_sync_time;	// This is the next scheduled time when the SYNC should happen	
static BOOL g_sync_expected;
#endif
volatile OS_GenericTask * g_current_task;
static UINT32	g_current_timeout;	// The time for which the timer is currently setup

static OS_AperiodicTask * g_idle_task;	// A TCB for the idle task
static UINT32 g_idle_task_stack [OS_IDLE_TASK_STACK_SIZE];
static char os_name_string [] = { OS_NAME_STRING };

// External functions used in here
extern void _OS_InitInterrupts();
extern void _OS_ContextRestore(void *new_task);
extern void _OS_ContextSw(void * new_task);
extern UINT32 *_OS_BuildKernelTaskStack(UINT32 * stack_ptr, 
							void (*task_function)(void *), 
							void * arg);
extern UINT32 *_OS_BuildUserTaskStack(UINT32 * stack_ptr, 
							void (*task_function)(void *), 
							void * arg);

void _OS_Timer0ISRHandler(void *arg);
void _OS_Timer1ISRHandler(void *arg);
void _OS_SetAlarm(OS_PeriodicTask *task, 
					UINT64 abs_time_in_us, 
					BOOL is_new_job,
					BOOL update_timer);
static void _OS_SetNextTimeout(void);
void _OS_ReSchedule(void);
void _OS_Exit(void);
void kernel_process_entry(void * pdata);
void main(int argc, char **argv);

// Static methods
static void _OS_idle_task(void * ptr);

///////////////////////////////////////////////////////////////////////////////
// The following funcstion starts the OS scheduling
///////////////////////////////////////////////////////////////////////////////
void OS_Start()
{
	// Check if the OS is already running
	if(!_OS_IsRunning)
	{
		// Reset the global timer
		g_global_time = 0;
		g_next_wakeup_time = 0;
		g_current_timeout = 0;
#if ENABLE_SYNC_TIMER==1
		g_next_sync_time = SYNC_TIMER_INTERVAL;		// Time of first SYNC
#endif

		// Reset the current task
		g_current_task = 0;
						
		// Now go through the list of all processes and call their entry functions
		g_current_process = g_process_list_head;
		while(g_current_process)
		{
			// process_entry_function would create tasks
			g_current_process->process_entry_function(g_current_process->pdata);
			g_current_process = g_current_process->next;
		}
		
		// Enable L2 Cache if required
#if ENABLE_UNIFIED_L2_CACHE==1
		_MMU_EnableL2Cache();
#endif
		
		// Start scheduling by creating the first interrupt
		UINT32 delay_start = OS_FIRST_SCHED_DELAY;
		_OS_UpdateTimer(&delay_start);	// First timer after 10ms
		
#if ENABLE_SYNC_TIMER==1
		// Start the SYNC timer
		_OS_StartSyncTimer();
#endif // ENABLE_SYNC_TIMER

		_OS_IsRunning = TRUE;	// No need to protect this line and the above
		// 'if' condition as this is called from the initialization for the
		// first time and there is only single thread at that point of time.

		// Start processing interrupts
		_OS_InitInterrupts();
		
		// Call reschedule. 
		_OS_ReSchedule();
		
		// We would never return from the above call. 
		// The current stack continues as SVC stack handling all interrupts
		_OS_Exit();
	}
}

///////////////////////////////////////////////////////////////////////////////
// Kernel Process Entry function
///////////////////////////////////////////////////////////////////////////////
void kernel_process_entry(void * pdata)
{
	OS_Task idle_tcb;
#if OS_ENABLE_CPU_STATS==1
	OS_Task stat_tcb;
#endif
	
	// Create all kernel tasks. Currently there are:
	// 		- Idle task
	//		- Statistics task
	
	// Create the IDLE task 
	_OS_CreateAperiodicTask(MIN_PRIORITY + 1,
		g_idle_task_stack, 
		OS_IDLE_TASK_STACK_SIZE << 2,	// In Bytes
		"idle",
		SYSTEM_TASK,
		&idle_tcb,
		_OS_idle_task,
		NULL);
		
		if(idle_tcb != INVALID) 
		{
			g_idle_task = (OS_AperiodicTask *)&g_task_pool[idle_tcb];
		}
		
		// Create the statistics task
#if OS_ENABLE_CPU_STATS==1
	_OS_CreatePeriodicTask(STAT_TASK_PERIOD, STAT_TASK_PERIOD, 
		STAT_TASK_PERIOD / 50, 0, g_stat_task_stack, 
		sizeof(g_stat_task_stack), 
		"STATISTICS", 
		SYSTEM_TASK,
		&stat_tcb, 
		_OS_StatisticsFn, 0);
		
		if(stat_tcb != INVALID) 
		{
			g_stat_task = (OS_PeriodicTask *)&g_task_pool[stat_tcb];
		}
#endif

	// Call main from the kernel process which will create more processes
	// Note that main() should return in order for normal scheduling to start
	// This is a difference in the other OS and this OS.
	main(1, (char **)(&os_name_string));
}

///////////////////////////////////////////////////////////////////////////////
// IDLE Task
///////////////////////////////////////////////////////////////////////////////
static void _OS_idle_task(void * ptr)
{
	while(1)
	{
		// Wait for interrupt at lower power state
		_sysctl_wait_for_interrupt();
		
#if OS_ENABLE_CPU_STATS==1
		g_idle_count++;
#endif
	}
}

///////////////////////////////////////////////////////////////////////////////
// The following function schedules the first task from the ready queue 
///////////////////////////////////////////////////////////////////////////////
void _OS_ReSchedule()
{
	void * task;
	UINT32 intsts;

	OS_ENTER_CRITICAL(intsts);	// Enter critical section

	if(_OS_QueuePeek(&g_ready_q, (void**) &task, 0) != SUCCESS)
	{
		_OS_QueuePeek(&g_ap_ready_q, (void**) &task, 0);
	}

	_OS_SetNextTimeout();
	
	KlogStr(KLOG_CONTEXT_SWITCH, "ContextSW To - ", ((OS_AperiodicTask *)task)->name);
		
	OS_EXIT_CRITICAL(intsts);	// Exit critical section
	
	_OS_ContextSw(task);	// This has the affect of g_current_task = task;
}

///////////////////////////////////////////////////////////////////////////////
// This function is invoked when the timer expires. This function should calculate
// and set the new delay required for the next timer expiry in number of
// microseconds.
///////////////////////////////////////////////////////////////////////////////
void _OS_Timer0ISRHandler(void *arg)
{
	OS_PeriodicTask * task;
	UINT64 new_time = 0;
	
#if ENABLE_SYNC_TIMER==1	
	if(!g_sync_expected)
#endif	// ENABLE_SYNC_TIMER
	{
		KlogStr(KLOG_OS_TIMER_ISR, "OS Timer ISR - ", ((OS_AperiodicTask *)arg)->name);
				
		// Acknowledge the interrupt
		_OS_TimerInterrupt(TIMER0);
	}
	
#if ENABLE_SYNC_TIMER==1	
	g_sync_expected = FALSE;
#endif	// ENABLE_SYNC_TIMER
	
	g_global_time = g_next_wakeup_time;	
	g_next_wakeup_time = (UINT64)-1;
	
	// The argument in this case is the g_current_task before the interrupt
	task = (OS_PeriodicTask *)arg;
	if(IS_PERIODIC_TASK(task->attributes))
	{
		// Adjust the remaining & accumulated budgets
		task->accumulated_budget += g_current_timeout;
		task->remaining_budget -= g_current_timeout;

		// If the remaining_budget == 0, there was a TBE exception.
		if(task->remaining_budget == 0)
		{			
			KlogStr(KLOG_TBE_EXCEPTION, "TBE Exception = ", task->name);
			
			// Count the number of TBEs
			task->TBE_count++;

			// TODO: Raise the flag saying that the thread has exceeded its
			// budget
		
			// Suspend the current task. 
			_OS_QueueDelete(&g_ready_q, task);

			// We need to re-insert this task based on the alarm_time into the wait queue
			if(task->deadline == task->period)
			{
				_OS_SetAlarm(task, task->alarm_time, FALSE, FALSE);		
			}
			else
			{
				_OS_SetAlarm(task, (task->alarm_time + task->period - task->deadline), FALSE, FALSE);
			}			
		}
	}

	while(_OS_QueuePeek(&g_ready_q, (void**) &task, &new_time) == SUCCESS)
	{
		// First Check the deadline expiry for each job in the ready queue
		if(new_time <= g_global_time)
		{
			// Now get the front task from the queue.
			_OS_QueueGet(&g_ready_q, (void**) &task, 0);

			// Deadline has expired
			// TODO: Take necessary action for deadline miss
			task->dline_miss_count ++;

			// If the relative deadline and the period are same, then reintroduce a job
			// in the ready queue because the next period for the task is to begin now
			if(task->deadline == task->period)
			{
				// ReSet the remaining budget to full budget
				task->remaining_budget = task->budget;
				_OS_SetAlarm(task, g_global_time + task->period, TRUE, FALSE);
			}
			// Else put it in the wait_q so that we can reintroduce the task to ready
			// when the next period begins some time later.
			else
			{
				_OS_SetAlarm(task, g_global_time + task->period - task->deadline, FALSE, FALSE);
			}

			continue;
		}
		else
		{
			break;
		}
	}

	// Now consider new jobs to be introduced from the wait queue
	while(_OS_QueuePeek(&g_wait_q, (void**) &task, &new_time) == SUCCESS)
	{
		// First Check the deadline expiry for each job in the ready queue
		if(new_time <= g_global_time)
		{
			// Now get the new task from the queue.
			_OS_QueueGet(&g_wait_q, (void**) &task, 0);
			
			// ReSet the remaining budget to full budget
			task->remaining_budget = task->budget;
			_OS_SetAlarm(task, g_global_time + task->deadline, TRUE, FALSE);
			continue;
		}
		else
		{
			break;				
		}
	}

	// Select the next task to schedule. First from the periodic ready queue
	// then from the Aperiodic ready queue
	if(_OS_QueuePeek(&g_ready_q, (void**) &task, 0) != SUCCESS)
	{
		_OS_QueuePeek(&g_ap_ready_q, (void**) &task, 0);
	}

	// Update timeout based on the periodic ready/wait queue and the ready task's remaining budget
	_OS_SetNextTimeout();

	KlogStr(KLOG_CONTEXT_SWITCH, "ContextSW To : ", task->name);
	
	_OS_ContextRestore(task);	// This has the affect of g_current_task = task;
}

///////////////////////////////////////////////////////////////////////////////
// This function puts the current thread to sleep and sets up an alarm for
// waking up at a specified time instant (relative to the starting time).
// ASSUMPTION: The interrupts are disabled when this function is invoked
///////////////////////////////////////////////////////////////////////////////
void _OS_SetAlarm(OS_PeriodicTask *task, 
	UINT64 abs_time_in_us, 
	BOOL is_new_job,
	BOOL update_timer)
{
	// If it was a past time, then return immediately.
	if(g_global_time >= abs_time_in_us) 
	{
		return;
	}

	// Insert the task into the g_ready_q / g_wait_q
	task->alarm_time = abs_time_in_us;
	_OS_QueueInsert(((is_new_job) ? &g_ready_q : &g_wait_q), (void *) task, abs_time_in_us);
	
	if(update_timer) 
	{
		_OS_SetNextTimeout();
	}
}

///////////////////////////////////////////////////////////////////////////////
// Set the timer to next earliest timeout requested in either the
// ready queue or the wait queue. This function should be called with interrupts disabled
// This function takes a task parameter (optional) which is the next task to be scheduled.
///////////////////////////////////////////////////////////////////////////////
void _OS_SetNextTimeout(void)
{
	UINT64 t1 = (UINT64)-1;
	UINT64 t2 = (UINT64)-1;
	OS_PeriodicTask *task;

	_OS_QueuePeek(&g_ready_q, (void**) &task, &t1);
	_OS_QueuePeek(&g_wait_q, 0, &t2);
	
	// Get the minimum of t1, t2 & task->remaining_budget
	UINT64 timeout_in_us = (t1 < t2) ? t1 : t2;
	UINT64 budget_timeout = (task) ? (g_global_time + task->remaining_budget) : (UINT64)-1;

	if(budget_timeout < timeout_in_us)
	{
		timeout_in_us = budget_timeout;
	}
	
	UINT32 budget_spent = 0;
	
	// If the requested timeout is crossing g_next_sync_time, then we need to adjust the
	// requested timeout in order to guarantee that we schedule an interrupt at the SYNC time
#if ENABLE_SYNC_TIMER==1	
	if((timeout_in_us >= g_next_sync_time) && (g_next_sync_time > g_global_time))
	{
		if(!g_sync_expected)	// If we haven't already setup for SYNC interrupt
		{
			// When the next scheduled timeout is the SYNC timeout, we will not set TIMER 0.
			// We will wait for SYNC timer interrupt.
			g_current_timeout = (UINT32)(g_next_sync_time - g_global_time);
			g_next_wakeup_time = g_next_sync_time;
			
			// Disable OS timer. Wait for SYNC timer interrupt.
			budget_spent = _OS_UpdateTimer(NULL);
			
			// Note that we are expecting a SYNC interrupt. When we actually get the SYNC interrupt,
			// if this variable is not set, then it indicates a scheduling problem.
			g_sync_expected = TRUE;
		}
		
		return;
	}
#endif // ENABLE_SYNC_TIMER

	// Check if we want a shorter timeout than the one currently set
	if((timeout_in_us > g_global_time) && \
		(timeout_in_us < g_next_wakeup_time))	
	{
		g_current_timeout = (UINT32)(timeout_in_us - g_global_time);
		budget_spent = _OS_UpdateTimer(&g_current_timeout);	// Note that _OS_UpdateTimer may choose a shorter timeout
		g_next_wakeup_time = g_global_time + g_current_timeout;
#if ENABLE_SYNC_TIMER==1			
		g_sync_expected = FALSE;
#endif // ENABLE_SYNC_TIMER
	}
	
	// Check if we need to adjust the budget spent for the old task
	if(budget_spent > 0)
	{
		OS_PeriodicTask * cur_task = (OS_PeriodicTask *)g_current_task;
		if(IS_PERIODIC_TASK(cur_task->attributes))
		{
			if(task != cur_task)
			{
				cur_task->accumulated_budget += budget_spent;
				cur_task->remaining_budget -= budget_spent;
			}
			else
			{
				panic("task == cur_task: Unexpected condition");
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Timer 1 ISR
///////////////////////////////////////////////////////////////////////////////
#if ENABLE_SYNC_TIMER==1
void _OS_Timer1ISRHandler(void *arg)
{
	KlogStr(KLOG_SYNC_TIMER_ISR, "SYNC Timer ISR", " entered");
	
	// Acknowledge the interrupt
	_OS_TimerInterrupt(TIMER1);

	// Time of next SYNC
	g_next_sync_time += SYNC_TIMER_INTERVAL;

	if(!g_sync_expected)
	{
#if OS_ENABLE_CPU_STATS==1	
		sync_timer_miss_counter++;
#endif // OS_ENABLE_CPU_STATS
	
		Syslog32("KERNEL WARNING: SYNC interrupt not expected - ", g_current_timeout);		
	
		// The SYNC timer triggered before we expected it. So just yield and let the 
		// OS Timer do its work
		_OS_ContextRestore(arg);
	}
		
	// Handle the interrupt as if this is regular OS timer interrupt
	_OS_Timer0ISRHandler(arg);
	
}
#endif	// ENABLE_SYNC_TIMER

///////////////////////////////////////////////////////////////////////////////
// The below function, gets the total elapsed time since the beginning
// of the system in microseconds.
///////////////////////////////////////////////////////////////////////////////
UINT64 OS_GetElapsedTime()
{
	UINT64 elapsed_time, old_global_time;

	// NOTE: The below loop for GetElapsedTime is very important. The design 
	// for this function is due to:
	// 1. We want to ensure that there is no interruption b/w reading
	// g_global_time and _OS_GetTimerValue_us. Otherwise we will have old g_global_time 
	// and latest timer count, which is not correct. 
	// 2. We cannot use disable/enable interrupts to avoid looping. This is because
	// the timer is designed to keep running in the background even if we have
	// disabled the interrupts. So if we disable the interrupts, it is possible
	// that the timer has reached the terminal value and then it reloads 0 again
	// and starts running. And since we have disabled the interrupts, we would get
	// older g_global_time and newer timer count, which is not correct.
	do
	{
		old_global_time = g_global_time;
		elapsed_time = g_global_time + _OS_GetTimerValue_us();
	} 
	while(old_global_time != g_global_time); // To ensure that the timer has not expired since we have read both g_global_time and OSW_GetTime

	return elapsed_time; 
}
