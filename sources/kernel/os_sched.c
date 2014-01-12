///////////////////////////////////////////////////////////////////////////////
//    
//                        Copyright 2009-2013 xxxxxxx, xxxxxxx
//    File:    os_queue.c
//    Author: Bala B.
//    Description: Main Scheduling function implementation
//    
///////////////////////////////////////////////////////////////////////////////

#include "os_core.h"
#include "os_queue.h"
#include "os_timer.h"
#include "util.h"
#include "sysctl.h"

// The PERIODIC_TIMER_INTERVAL is same as MIN_TASK_PERIOD
#define PERIODIC_TIMER_INTERVAL     MIN_TASK_PERIOD

_OS_Queue g_ready_q;
_OS_Queue g_wait_q;
_OS_Queue g_ap_ready_q;
_OS_Queue g_block_q;

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

// Semaphores which have periodic tasks blocking on them needs to be managed
// by the scheduler. The scheduler needs to keep updating the queue as and when
// their deadlines get expired. The following mask indicates all such semaphores.
// We don't need such special treatment for blocked Aperiodic tasks
extern UINT32 g_semaphore_active_periodic_queue_mask[];
extern OS_SemaphoreCB * g_semaphore_pool[];

// Periodic timer ISR
void _OS_PeriodicTimerISR(void *arg);

// Budget timer ISR
void _OS_BudgetTimerISR(void *arg);

// Some more function prototypes
void _OS_InitInterrupts();
void _OS_ContextRestore(void *new_task);
void _OS_ContextSw(void * new_task);
void _OS_Schedule(void);
void _OS_Exit(void);
void _OS_Timer_AckInterrupt(UINT32 timer);

void _OS_SetAlarm(OS_Task *task,
                  UINT64 abs_time_in_us,
                  BOOL ready);

void kernel_process_entry(void * pdata);
void main(int argc, char **argv);

// local methods
static void CheckTaskBudgetDline(OS_Task * task);
static void UpdateSemaphoreWaitQueue(void);
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

        // Call reschedule. 
        _OS_Schedule();
        
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
    // - Serial task
    
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
    
    // Update all semaphore wait queues which may have expiring deadlines for periodic tasks
    UpdateSemaphoreWaitQueue();
    
    // Consider new jobs to be introduced from the wait queue
    while(_OS_QueuePeek(&g_wait_q, NULL, &new_time) == SUCCESS)
    {
        if(new_time > g_current_period_us) break;
		
        // Dequeue the new task from the queue.
        _OS_QueueGet(&g_wait_q, (void**) &task, NULL);
        
		ASSERT(g_current_period_us == task->job_release_time);
        
        // Reset the remaining budget to full
        task->periodic.remaining_budget = task->periodic.budget;
        
        // Insert into ready queue with deadline as the key. This is where the EDF scheduler
        // is coming into picture
        _OS_SetAlarm(task, g_current_period_us + task->periodic.deadline, TRUE);
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
    
    // Call the OS Scheduler function to schedule the next task
    _OS_Schedule();
}

static void UpdateSemaphoreWaitQueue(void)
{
	UINT32 i;
	UINT64 new_time = 0;
	OS_Task * task;
	const UINT32 word_count = (MAX_SEMAPHORE_COUNT + 31) >> 5;
	
	for(i = 0; i < word_count; i++)
	{
		UINT32 mask = g_semaphore_active_periodic_queue_mask[i];
		if(mask)
		{
			const UINT32 index = clz(mask);
			mask &= ~(1 << index);

			OS_SemaphoreCB * sem = g_semaphore_pool[(i << 5) + (31 - index)];
			ASSERT(sem);
			
			// Check if the first element in the blocked periodic task queue has exceeded
    		while(_OS_QueuePeek(&sem->periodic_task_queue, NULL, &new_time) == SUCCESS)
		    {
				if(new_time > g_current_period_us) break;
		
				// Now get the front task from the queue.
				_OS_QueueGet(&sem->periodic_task_queue, (void**) &task, NULL);
		
				// Deadline has expired
				// TODO: Take necessary action for deadline miss
				task->periodic.dline_miss_count ++;
		
				KlogStr(KLOG_DEADLINE_MISS, "Deadline Miss (Blocked) - ", task->name);
		
				// We are done for the current period. Update the next job_release_time.
				task->periodic.job_release_time += task->periodic.period;
		
				// Re-insert the task with the new deadline
				task->periodic.alarm_time = task->periodic.job_release_time + task->periodic.deadline;
				_OS_QueueInsert(&sem->periodic_task_queue, (void *) task, task->periodic.alarm_time);
			}
		}
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
        if(budget_spent > task->periodic.remaining_budget)
			budget_spent = task->periodic.remaining_budget;

        // Adjust the remaining & accumulated budgets
		task->periodic.remaining_budget -= budget_spent;
        
        // If the remaining_budget == 0, there was a TBE exception.
        if(task->periodic.remaining_budget == 0)
        {
            KlogStr(KLOG_TBE_EXCEPTION, "TBE Exception = ", task->name);
            
            // Count the number of TBEs
            task->periodic.TBE_count++;
            task->periodic.exec_count++;
            
            // TODO: Raise the flag saying that the thread has exceeded its budget
            
            // Take the current task out of ready queue
            _OS_QueueGet(&g_ready_q, NULL, NULL);
            
            // We are done for the current period. Update the next job_release_time.
            task->periodic.job_release_time += task->periodic.period;
            
			// If we are going to put the task into ready queue, then the alarm time
			// should be the deadline. Or else, it should be the next release time
			if(task->periodic.job_release_time == g_current_period_us)
				_OS_SetAlarm(task, task->periodic.job_release_time + task->periodic.deadline, TRUE);
			else
				_OS_SetAlarm(task, task->periodic.job_release_time, FALSE);
        }
    }
    
    // Check if anyone in the ready queue exceeded the deadline
    while(_OS_QueuePeek(&g_ready_q, NULL, &new_time) == SUCCESS)
    {
        if(new_time > (g_current_period_us + g_current_period_offset_us)) break;
        
        // Now get the front task from the queue.
        _OS_QueueGet(&g_ready_q, (void**) &task, NULL);
        
        // Deadline has expired
        // TODO: Take necessary action for deadline miss
        task->periodic.dline_miss_count ++;
        task->periodic.exec_count++;
        
        KlogStr(KLOG_DEADLINE_MISS, "Deadline Miss - ", task->name);
        
        // We are done for the current period. Update the next job_release_time.
        task->periodic.job_release_time += task->periodic.period;
        
		// If we are going to put the task into ready queue, then the alarm time
		// should be the deadline. Or else, it should be the next release time
		if(task->periodic.job_release_time == g_current_period_us)
			_OS_SetAlarm(task, task->periodic.job_release_time + task->periodic.deadline, TRUE);
		else
			_OS_SetAlarm(task, task->periodic.job_release_time, FALSE);
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
    task->periodic.alarm_time = abs_time_in_us;
    _OS_QueueInsert((ready ? &g_ready_q : &g_wait_q), (void *) task, abs_time_in_us);
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
    if(_OS_QueuePeek(&g_ready_q, (void**) &task, 0) != SUCCESS)
    {
        _OS_QueuePeek(&g_ap_ready_q, (void**) &task, 0);
    }

    KlogStr(KLOG_CONTEXT_SWITCH, "ContextSW To - ", task->name);

    // For periodic task, set the next budget timeout we should use.
    if(IS_PERIODIC_TASK(task->attributes))
    {
        // The timeout to be used = MIN(task remaining budget, task next deadline)
        UINT64 now = g_current_period_us + g_current_period_offset_us;
        UINT64 abs_deadline_us = (task->periodic.job_release_time + task->periodic.deadline);
        UINT64 abs_budget_us = (now + task->periodic.remaining_budget);
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

            task->periodic.exec_count++;
            
            // Adjust the remaining & accumulated budgets
            ASSERT(budget_spent <= task->periodic.remaining_budget);
            task->periodic.remaining_budget -= budget_spent;
            
            // Take the current task out of ready queue
            _OS_QueueGet(&g_ready_q, NULL, NULL);
            
            // We are done for the current period. Update the next job_release_time.
            task->periodic.job_release_time += task->periodic.period;
            
			// If we are going to put the task into ready queue, then the alarm time
			// should be the deadline. Or else, it should be the next release time
			if(task->periodic.job_release_time == g_current_period_us)
				_OS_SetAlarm(task, task->periodic.job_release_time + task->periodic.deadline, TRUE);
			else
				_OS_SetAlarm(task, task->periodic.job_release_time, FALSE);
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
		_OS_QueueDelete(&g_ap_ready_q, task);

		// Insert into block q
		_OS_QueueInsert(&g_block_q, task, task->aperiodic.priority);

		// Note that the TCB resource for this task will not be freed.
		// This task will remain in the blocked queue permanently
	
		OS_EXIT_CRITICAL(intsts);
		
		ret = SUCCESS;
	}

	return ret;
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
