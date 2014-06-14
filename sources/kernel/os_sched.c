///////////////////////////////////////////////////////////////////////////////
//    
//                        Copyright 2009-2013 xxxxxxx, xxxxxxx
//    File:    os_sched.c
//    Author: Bala B.
//    Description: Main Scheduling function implementation
//    
///////////////////////////////////////////////////////////////////////////////

#include "os_sched.h"
#include "os_timer.h"
#include "util.h"
#include "sysctl.h"

// The PERIODIC_TIMER_INTERVAL is same as MIN_TASK_PERIOD
#define PERIODIC_TIMER_INTERVAL     MIN_TASK_PERIOD

_OS_Queue g_ready_q;
_OS_Queue g_wait_q;
_OS_Queue g_ap_ready_q;
_OS_Queue g_completed_task_q;

// The following queue has all the tasks that are blocked on resources such ASSERT
// Semaphores or IOs
_OS_Queue g_periodic_blocked_q;

// This global variable can be accessed from outside
BOOL _OS_IsRunning = FALSE;

// This variable holds the beginning time (in us) of the current period
// It gets updated everytime the Periodic ISR is fired (NOT updated for budget timer)
UINT64 g_current_period_us;

// This variable holds the starting time of next period (in us)
UINT64 g_next_period_us;

// Amount of time elapsed since the beginning of the period. This is updated whenever
// a budget timer interrupt happens or when the task yields
UINT32 g_current_period_offset_us;

OS_Task * g_current_task;
	
UINT32 g_periodic_timer_intr_counter;
UINT32 g_budget_timer_intr_counter;

#if OS_ENABLE_CPU_STATS==1
UINT32 g_max_scheduler_elapsed_count;
UINT32 g_sched_starting_counter_value;
UINT32 g_sched_ending_counter_value;
#endif
static char os_name_string [] = { OS_NAME_STRING };

OS_Task * g_idle_task;  // A TCB for the idle task
static UINT32 g_idle_task_stack [OS_IDLE_TASK_STACK_SIZE];

extern OS_Process * g_kernel_process;	// Kernel process


// local methods
void main(int argc, char **argv);
static void CheckTaskBudgetDline(OS_Task * task);
static void UpdatePeriodicBlockedQueue(void);
static void _OS_idle_task(void * ptr);

#define MIN(a, b)   (((a) > (b)) ? (b) : (a))

static __inline__ UINT32 clz(UINT32 input)
{
	unsigned int result;
	
	__asm__ volatile("clz %0, %1" : "=r" (result) : "r" (input));
	
	return result;
}

///////////////////////////////////////////////////////////////////////////////
// The following funcstion starts the OS scheduling
// Note that this function never returns
///////////////////////////////////////////////////////////////////////////////
void _OS_Start()
{
    // Check if the OS is already running
    if(!_OS_IsRunning)
    {
        // Reset the global timer variables
        g_current_period_us = 0;
        g_next_period_us = 0;
        g_current_period_offset_us = 0;

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

#if ENABLE_MMU		
		// We need to set permissions in the domain access register before we enable MMU
		// DOMAIN_ACCESS_CLIENT specifies that the permission bits should be used from the Page Tables
		_sysctl_set_domain_rights(
			(DOMAIN_ACCESS_CLIENT << KERNEL_DOMAIN), 
			(DOMAIN_ACCESS_MASK << KERNEL_DOMAIN));
								  
		// Before we enable MMU, we need to flush TLB
		_sysctl_flush_tlb();
		
		// Before enabling MMU, set the page table address in SYSCTL register
		_sysctl_set_ptable((PADDR)g_kernel_process->ptable);

		// Start the MMU and Virtual Memory
		_sysctl_enable_mmu();	
#endif


        // Start the Periodic timer
        _OS_Timer_PeriodicTimerStart(PERIODIC_TIMER_INTERVAL);

#if OS_ENABLE_CPU_STATS==1
        Syslog32("Max periodic timer count = ", _OS_Timer_GetMaxCount(PERIODIC_TIMER));
#endif

        _OS_IsRunning = TRUE;

		// Switch to idle task explicitly until the actual scheduling begins with the 
		// first periodic timer interrupt 
		_OS_ContextRestore(g_idle_task);
        
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
    OS_Task_t idle_tcb;
    
    // Create all kernel tasks. Currently there are:
    // - Idle task
    
    // Create the IDLE task 
    _OS_CreateAperiodicTask(MIN_PRIORITY + 1,
        g_idle_task_stack, 
        OS_IDLE_TASK_STACK_SIZE << 2,    // In Bytes
        "idle",
        SYSTEM_TASK,
        &idle_tcb,
        _OS_idle_task,
        NULL);
        
        if(idle_tcb != INVALID) 
        {
            g_idle_task = (OS_Task *)&g_task_pool[idle_tcb];
        }
        
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
    }
}

///////////////////////////////////////////////////////////////////////////////
// Periodic Timer ISR
// This interrupt comes every PERIODIC_TIMER_INTERVAL
// The argument in this case is the g_current_task at the time of interrupt
///////////////////////////////////////////////////////////////////////////////
void _OS_PeriodicTimerISR(void *arg)
{
    UINT64 new_time = 0;
    OS_Task * task = (OS_Task *)arg;

    KlogStr(KLOG_PERIODIC_TIMER_ISR, "Periodic ISR - ", task->name);
                
    // Acknowledge the timer interrupt
    _OS_Timer_AckInterrupt(PERIODIC_TIMER);
    
    // Update timer variables
    g_current_period_us = g_next_period_us;
    g_next_period_us += PERIODIC_TIMER_INTERVAL;
    g_current_period_offset_us = 0;    
    
#if OS_ENABLE_CPU_STATS==1
    g_sched_starting_counter_value = _OS_Timer_GetMaxCount(PERIODIC_TIMER);
    g_periodic_timer_intr_counter++;
#endif
    
    // Check if any ready task exceeded budget or deadline
    CheckTaskBudgetDline(task);
    
    // Update global periodic blocked queue which may have expiring deadlines 
    // for periodic tasks
    UpdatePeriodicBlockedQueue();
    
    // Consider new jobs to be introduced from the wait queue
    while(_OS_QueuePeekWithKey(&g_wait_q, NULL, &new_time))
    {
        if(new_time > g_current_period_us) break;
		
        // Dequeue the new task from the queue.
        _OS_PQueueGet(&g_wait_q, (_OS_TaskQNode**) &task);
        
		ASSERT(g_current_period_us == task->p.job_release_time);
        
        // Reset the remaining budget to full
        task->p.remaining_budget = task->p.budget;
        
        // Insert into ready queue with deadline as the key. This is where the EDF scheduler
        // is coming into picture
        _OS_SetAlarm(task, g_current_period_us + task->p.deadline, TRUE);
    }
    
    // Call the OS Scheduler function to schedule the next task
    _OS_Schedule();
}

///////////////////////////////////////////////////////////////////////////////
// Budget Timer ISR
///////////////////////////////////////////////////////////////////////////////
void _OS_BudgetTimerISR(void *arg)
{
    OS_Task * task = (OS_Task *)arg;
    
    KlogStr(KLOG_BUDGET_TIMER_ISR, "Budget/Dline ISR - ", task->name);

    // Acknowledge the timer interrupt
    _OS_Timer_AckInterrupt(BUDGET_TIMER);

#if OS_ENABLE_CPU_STATS==1
    g_sched_starting_counter_value = _OS_Timer_GetCount(PERIODIC_TIMER);
    g_budget_timer_intr_counter++;
#endif
    
    // Get the time elapsed since the beginning of the period
    g_current_period_offset_us = _OS_Timer_GetTimeElapsed_us(PERIODIC_TIMER);
        
    // Some ready task must have exceeded its budget or deadline
    // Do the necessary handling
    CheckTaskBudgetDline(task);
    
    // Update global periodic blocked queue which may have expiring deadlines 
    // for periodic tasks
    UpdatePeriodicBlockedQueue();
    
    // Call the OS Scheduler function to schedule the next task
    _OS_Schedule();
}

///////////////////////////////////////////////////////////////////////////////
// UpdatePeriodicBlockedQueue
///////////////////////////////////////////////////////////////////////////////
static void UpdatePeriodicBlockedQueue(void)
{
	UINT64 abs_deadline;
	const UINT64 curtime = (g_current_period_us + g_current_period_offset_us);
	OS_Task * task;
	
    while(_OS_QueuePeekWithKey(&g_periodic_blocked_q, (_OS_TaskQNode **) &task, &abs_deadline))
    {
        if(abs_deadline > curtime) break;
		
        // Dequeue the new task from the queue.
        _OS_PQueueGet(&g_periodic_blocked_q, NULL);
                
		// Deadline has expired
		// TODO: Take necessary action for deadline miss
		task->p.dline_miss_count ++;

		KlogStr(KLOG_DEADLINE_MISS, "Deadline Miss (Blocked) - ", task->name);

        // Reset the remaining budget to full
        task->p.remaining_budget = task->p.budget;

		// We are done for the current period. Update the next job_release_time.
		task->p.job_release_time += task->p.period;

		// Re-insert the task with the new deadline
		_OS_PQueueInsertWithKey(&g_periodic_blocked_q, (_OS_TaskQNode *) task, 
								task->p.job_release_time + task->p.deadline);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Check task budget & deadline
///////////////////////////////////////////////////////////////////////////////
static void CheckTaskBudgetDline(OS_Task * task)
{	
	UINT64 new_time = 0;
	UINT32 budget_spent = _OS_Timer_GetTimeElapsed_us(BUDGET_TIMER);
    task->accumulated_budget += budget_spent;
    
    if(IS_PERIODIC_TASK(task->attributes))
    {
		// The budget spent should always be < remaining_budget. However because of
		// inaccuracies in the tick <-> us conversions, it is better to limit the budget_spent
        if(budget_spent > task->p.remaining_budget)
			budget_spent = task->p.remaining_budget;

        // Adjust the remaining & accumulated budgets
		task->p.remaining_budget -= budget_spent;
        
        // If the remaining_budget == 0, there was a TBE exception.
        if(task->p.remaining_budget == 0)
        {
            KlogStr(KLOG_TBE_EXCEPTION, "TBE Exception = ", task->name);
            
            // Count the number of TBEs
            task->p.TBE_count++;
            task->p.exec_count++;
            
            // TODO: Raise the flag saying that the thread has exceeded its budget
            
            // Take the current task out of ready queue
            _OS_PQueueGet(&g_ready_q, NULL);
            
            // We are done for the current period. Update the next job_release_time.
            task->p.job_release_time += task->p.period;
            
			// If we are going to put the task into ready queue, then the alarm time
			// should be the deadline. Or else, it should be the next release time
			if(task->p.job_release_time == g_current_period_us)
				_OS_SetAlarm(task, task->p.job_release_time + task->p.deadline, TRUE);
			else
				_OS_SetAlarm(task, task->p.job_release_time, FALSE);
        }
    }
    
    // Check if anyone in the ready queue exceeded the deadline
    while(_OS_QueuePeekWithKey(&g_ready_q, NULL, &new_time))
    {
        if(new_time > (g_current_period_us + g_current_period_offset_us)) break;
        
        // Now get the front task from the queue.
        _OS_PQueueGet(&g_ready_q, (_OS_TaskQNode**) &task);
        
        // Deadline has expired
        // TODO: Take necessary action for deadline miss
        task->p.dline_miss_count ++;
        task->p.exec_count++;
        
        KlogStr(KLOG_DEADLINE_MISS, "Deadline Miss - ", task->name);
        
        // We are done for the current period. Update the next job_release_time.
        task->p.job_release_time += task->p.period;
        
		// If we are going to put the task into ready queue, then the alarm time
		// should be the deadline. Or else, it should be the next release time
		if(task->p.job_release_time == g_current_period_us)
			_OS_SetAlarm(task, task->p.job_release_time + task->p.deadline, TRUE);
		else
			_OS_SetAlarm(task, task->p.job_release_time, FALSE);
    }
}

///////////////////////////////////////////////////////////////////////////////
// This function sets up an alarm at a specified instant for the given task.
// The parameter 'ready' indicates if the task should be put into the ready queue
// or the wait queue
///////////////////////////////////////////////////////////////////////////////
void _OS_SetAlarm(OS_Task *task,
                  UINT64 abs_time_in_us,
                  BOOL ready)
{
    // Ensure that the timeout is in the future
    ASSERT(abs_time_in_us > g_current_period_us)
        
    // Insert the task into the g_ready_q / g_wait_q
    task->p.alarm_time() = abs_time_in_us;
    _OS_PQueueInsertWithKey((ready ? &g_ready_q : &g_wait_q), (_OS_TaskQNode *) task, abs_time_in_us);
}

///////////////////////////////////////////////////////////////////////////////
// The following function schedules the first task from the ready queue
// It also sets appropriate timeout in the Budget timer (for periodic tasks)
// It also sets the virtual table to the new task's process
///////////////////////////////////////////////////////////////////////////////
void _OS_Schedule()
{
    OS_Task * task;
    
    // Check if there is any ready task in the periodic ready queue
    // Or else check the Aperiodic ready queue
    if(!_OS_QueuePeek(&g_ready_q, (_OS_TaskQNode**) &task))
    {
        _OS_QueuePeek(&g_ap_ready_q, (_OS_TaskQNode**) &task);
    }

    KlogStr(KLOG_CONTEXT_SWITCH, "ContextSW To - ", task->name);

    // For periodic task, set the next budget timeout we should use.
    if(IS_PERIODIC_TASK(task->attributes))
    {
        // The timeout to be used = MIN(task remaining budget, task next deadline)
        UINT64 now = g_current_period_us + g_current_period_offset_us;
        UINT64 abs_deadline_us = (task->p.job_release_time + task->p.deadline);
        UINT64 abs_budget_us = (now + task->p.remaining_budget);
        UINT64 abs_timeout_us = MIN(abs_deadline_us, abs_budget_us);
        
		ASSERT(abs_timeout_us > now);
            
		// Set the budget timeout. Even though there may be a periodic interrupt before
		// the next budget timeout, it is helpful to keep the timer running so that
		// we can accurately measure the budget spent
		_OS_Timer_SetTimeout_us(abs_timeout_us - now);
    }
    else
    {
        // If this is a Aperiodic task, keep the timer running so that we can calculate the budget used
        _OS_Timer_SetMaxTimeout();
    }
    
#if ENABLE_MMU
	// Before we change the ptable, we need to flush TLB so that older process's maps are discarded
	_sysctl_flush_tlb();

	// Set the page table address in SYSCTL register to the new task's process
	_sysctl_set_ptable((PADDR)(task->owner_process->ptable));
#endif
    
#if OS_ENABLE_CPU_STATS==1
	g_sched_ending_counter_value = _OS_Timer_GetCount(PERIODIC_TIMER);
	
	// Since timer is downcounting, the end value will be smaller than starting value
	if(g_sched_ending_counter_value < g_sched_starting_counter_value)
	{
		UINT32 diff_count = (g_sched_starting_counter_value - g_sched_ending_counter_value);
		if(g_max_scheduler_elapsed_count < diff_count) 
		{
			g_max_scheduler_elapsed_count = diff_count;
		}
	}
	
	g_sched_starting_counter_value = 0;
#endif

    // It is OK to context switch to another task with interrupts disabled
    _OS_ContextRestore(task);    // This has the affect of g_current_task = task;
}

///////////////////////////////////////////////////////////////////////////////
// Function to yield from a task
// Can be used with both Periodic / Aperiodic Tasks
// For periodic tasks, this means that we are done for the current period
///////////////////////////////////////////////////////////////////////////////
void _OS_TaskYield()
{
    if(g_current_task)
    {
		UINT32 budget_spent = _OS_Timer_GetTimeElapsed_us(BUDGET_TIMER);
        g_current_task->accumulated_budget += budget_spent;
		
        if(IS_PERIODIC_TASK(g_current_task->attributes))
        {
#if OS_ENABLE_CPU_STATS==1
	    	g_sched_starting_counter_value = _OS_Timer_GetCount(PERIODIC_TIMER);
#endif
            OS_Task * task = (OS_Task *)g_current_task;

            task->p.exec_count++;
            
            // Adjust the remaining & accumulated budgets
            ASSERT(budget_spent <= task->p.remaining_budget);
            task->p.remaining_budget -= budget_spent;
            
            // Take the current task out of ready queue
            _OS_PQueueGet(&g_ready_q, NULL);
            
            // We are done for the current period. Update the next job_release_time.
            task->p.job_release_time += task->p.period;
            
			// If we are going to put the task into ready queue, then the alarm time
			// should be the deadline. Or else, it should be the next release time
			if(task->p.job_release_time == g_current_period_us)
				_OS_SetAlarm(task, task->p.job_release_time + task->p.deadline, TRUE);
			else
				_OS_SetAlarm(task, task->p.job_release_time, FALSE);
        }

        // Before calling _OS_Schedule, update g_current_period_offset_us
        g_current_period_offset_us = _OS_Timer_GetTimeElapsed_us(PERIODIC_TIMER);

        // Call reschedule
        _OS_Schedule();
    }
}

///////////////////////////////////////////////////////////////////////////////
// Function to be called when an Aperiodic task finishes so that it is no more included
// in scheduling. Only Aperiodic tasks are allowed to complete
///////////////////////////////////////////////////////////////////////////////
OS_Return _OS_CompleteAperiodicTask()
{
	OS_Return ret = INVALID_TASK;
	UINT32 intsts;
	
	if(IS_APERIODIC_TASK(g_current_task->attributes))
	{
		OS_Task * task = (OS_Task *) g_current_task;

		// Ensure that we are in the critical section as some call paths are not thread safe.
		OS_ENTER_CRITICAL(intsts);		

		// If this function ever returns, just block this task by adding it to
		// block q
		_OS_PQueueDelete(&g_ap_ready_q, (_OS_TaskQNode *)task);

		// Insert into block q
		_OS_NPQueueInsert(&g_completed_task_q, (_OS_TaskQNode *)task);

		// Note that the TCB resource for this task will not be freed.
		// This task will remain in the blocked queue permanently
	
		OS_EXIT_CRITICAL(intsts);
		
		ret = SUCCESS;
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////
// Remove current task from the scheduler ready queue and insert it into blocked queue
// It is the responsibility of the caller to queue this task somewhere else
///////////////////////////////////////////////////////////////////////////////
void _OS_SchedulerBlockCurrentTask()
{
	UINT32 intsts;
		
	OS_ENTER_CRITICAL(intsts);
		
	if(IS_PERIODIC_TASK(g_current_task->attributes)) {
	
		// Delete the current task from ready tasks queue
		_OS_PQueueDelete(&g_ready_q, (_OS_TaskQNode *)g_current_task); 
		
		// Add this task to the global blocked periodic queue so that scheduler can
		// continuously update its deadlines and readiness
		_OS_PQueueInsertWithKey(&g_periodic_blocked_q, (_OS_TaskQNode *)g_current_task,
			g_current_task->p.alarm_time());
	}
	else {
		// Delete the current task from ready tasks queue
		_OS_PQueueDelete(&g_ap_ready_q, (_OS_TaskQNode *)g_current_task); 
	}
		
	OS_EXIT_CRITICAL(intsts);
}

///////////////////////////////////////////////////////////////////////////////
// Reinsert a task into scheduler queue. Depending on the current time,
// this function decides which scheduler queue should this task be inserted
// This function does not validate if the task given is really blocked or not
// It is the responsibility of the caller to ensure this
///////////////////////////////////////////////////////////////////////////////
void _OS_SchedulerUnblockTask(OS_Task * task)
{
	UINT32 intsts;
	
	ASSERT(task);
	
	OS_ENTER_CRITICAL(intsts);
	
	if(IS_PERIODIC_TASK(task->attributes)) {
	
		// Delete this task from the g_periodic_blocked_q
		_OS_PQueueDelete(&g_periodic_blocked_q, (_OS_TaskQNode *) task);
		
		// Insert this into the periodic ready / wait queue
		if(task->p.job_release_time <= g_current_period_us) {
		
			// We have a job waiting to complete. So insert this into ready queue
			_OS_PQueueInsertWithKey(&g_ready_q, (_OS_TaskQNode *)task, 
									task->p.job_release_time + task->p.deadline);
		}
		else {
			// There is no active job. So insert this into wait queue
			_OS_PQueueInsertWithKey(&g_wait_q, (_OS_TaskQNode *)task, 
									task->p.job_release_time);
		}
	}
	else {
	
		// Insert this task into Aperiodic ready queue
		_OS_PQueueInsertWithKey(&g_ap_ready_q,(_OS_TaskQNode *)task, task->ap.priority());
	}
	
	OS_EXIT_CRITICAL(intsts);
}

///////////////////////////////////////////////////////////////////////////////
// This function updates the accumulated budget and remaining budget for the
// currently running task
///////////////////////////////////////////////////////////////////////////////
void _OS_UpdateCurrentTaskBudget()
{
	UINT32 intsts;
		
	OS_ENTER_CRITICAL(intsts);

    // Get the task execution time
    UINT32 budget_spent = _OS_Timer_GetTimeElapsed_us(BUDGET_TIMER);
    g_current_task->accumulated_budget += budget_spent;
	
	if(IS_PERIODIC_TASK(g_current_task->attributes)) {
	
	    // Adjust the remaining  budget for the current task
	    ASSERT(budget_spent <= g_current_task->p.remaining_budget);
	    g_current_task->p.remaining_budget -= budget_spent;		
	}
	
	OS_EXIT_CRITICAL(intsts);
}

///////////////////////////////////////////////////////////////////////////////
// The below function, gets the total elapsed time since the beginning
// of the system in microseconds.
///////////////////////////////////////////////////////////////////////////////
UINT64 _OS_GetElapsedTime()
{
    UINT64 elapsed_time, old_global_time;

    // NOTE: The below loop for GetElapsedTime is very important. The design 
    // for this function is due to:
    // 1. We want to ensure that there is no interruption b/w reading
    // g_current_period_us and _OS_Timer_GetTimeElapsed_us. Otherwise we will have old g_current_period_us
    // and latest timer count, which is not correct. 
    // 2. We cannot use disable/enable interrupts to avoid looping. This is because
    // the timer is designed to keep running in the background even if we have
    // disabled the interrupts. So if we disable the interrupts, it is possible
    // that the timer has reached the terminal value and then it reloads 0 again
    // and starts running. And since we have disabled the interrupts, we would get
    // older g_current_period_us and newer timer count, which is not correct.
    do
    {
        old_global_time = g_current_period_us;
        elapsed_time = g_current_period_us + _OS_Timer_GetTimeElapsed_us(PERIODIC_TIMER);
    }
    // To ensure that the timer has not expired since we have read both g_current_period_us and _OS_Timer_GetTimeElapsed_us
    while(old_global_time != g_current_period_us);
    
    return elapsed_time;
}
