///////////////////////////////////////////////////////////////////////////////
//    
//                        Copyright 2009-2013 xxxxxxx, xxxxxxx
//    File:    os_sched.h
//    Author: Bala B.
//    Description: Main Scheduling functions
//    
///////////////////////////////////////////////////////////////////////////////

#ifndef _OS_SCHED_H
#define _OS_SCHED_H

#include "os_core.h"
#include "os_queue.h"

extern _OS_Queue g_ready_q;
extern _OS_Queue g_wait_q;
extern _OS_Queue g_ap_ready_q;
extern _OS_Queue g_completed_task_q;

// The following queue has all the tasks that are blocked on resources such ASSERT
// Semaphores or IOs
extern _OS_Queue g_periodic_blocked_q;

// This global variable can be accessed from outside
extern BOOL _OS_IsRunning;

// This variable holds the beginning time (in us) of the current period
// It gets updated everytime the Periodic ISR is fired (NOT updated for budget timer)
extern UINT64 g_current_period_us;

extern OS_Task * g_current_task;
	
extern UINT32 g_periodic_timer_intr_counter;
extern UINT32 g_budget_timer_intr_counter;

#if OS_ENABLE_CPU_STATS==1
extern UINT32 g_max_scheduler_elapsed_count;
extern UINT32 g_sched_starting_counter_value;
extern UINT32 g_sched_ending_counter_value;
#endif

extern OS_Task * g_idle_task;  // A TCB for the idle task

extern OS_Process * g_kernel_process;	// Kernel process

// Some function prototypes
void _OS_InitInterrupts();
void _OS_ContextRestore(void *new_task);
void _OS_ContextSw(void * new_task);
void _OS_Schedule(void);
void _OS_Exit(void);
void _OS_Timer_AckInterrupt(UINT32 timer);

void _OS_SetAlarm(OS_Task *task,
                  UINT64 abs_time_in_us,
                  BOOL ready);
void _OS_SchedulerBlockCurrentTask();
void _OS_SchedulerUnblockTask(OS_Task * task);

void kernel_process_entry(void * pdata);

#endif // _OS_SCHED_H
