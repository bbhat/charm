///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	os_timer.c
//	Author: Bala B.
//	Description: OS Timer implementation
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_timer.h"
#include "os_core.h"
#include "os_stat.h"
#include "soc.h"
#include "target.h"

//The S3C2440A has five 16-bit timers. Timer 0, 1, 2, and 3 have Pulse Width Modulation (PWM) function. 
// Timer 4 has an internal timer only with no output pins. The timer 0 has a dead-zone generator, 
// which is used with a large current device.
// For the purpose of OS, lets use Timer 0 & 1

// Similarly S5PV210 has 5 32-bit timers. We use timers 0 and 1 for the OS

// TODO: I need to make below calculations extremely efficient.
// Use 64 bit calculation for accuracy and always round up
#define CONVERT_us_TO_TICKS(us)		((((UINT64)TIMER0_TICK_FREQ * (us)) + (1000000-1)) / 1000000)
#define CONVERT_TICKS_TO_us(tick)	((((tick) * 1000000ull) + (TIMER0_TICK_FREQ - 1)) / TIMER0_TICK_FREQ)

#define TIMER0_START		0x001
#define TIMER0_RUNNING		0x001
#define TIMER0_UPDATE		0x002
#define TIMER0_AUTORELOAD	0x008

#define TIMER1_START		0x100
#define TIMER1_UPDATE		0x200
#define TIMER1_AUTORELOAD	0x800

static UINT32 timer0_count_buffer;

void _OS_Timer0ISRHandler(void *arg);
void _OS_Timer1ISRHandler(void *arg);

///////////////////////////////////////////////////////////////////////////////
// Function to initialize the timers 0 & 1
///////////////////////////////////////////////////////////////////////////////
void _OS_InitTimer ()
{
	static BOOL initialized = 0;

	if(!initialized)
	{
		// Place the timers 0 & 1 on hold 
		rTCON &= ~0xfff;
		
		// Clear any pending interrupts
		ACK_TIMER_INTERRUPT(TIMER0_INTERRUPT_INDEX);
		ACK_TIMER_INTERRUPT(TIMER1_INTERRUPT_INDEX);
		
		timer0_count_buffer = 0;
				
		// Configure the two prescalars
		rTCFG0 = (rTCFG0 & 0xffffff00) | TIMER_PRESCALAR_0;	 // Set the Pre-scaler 0
		rTCFG1 = (rTCFG1 & 0xffffff00) | (TIMER1_DIVIDER << 4) | TIMER0_DIVIDER;		// Set the divider
		
		// Set the interrupt handlers. This also unmasks that interrupt
		OS_SetInterruptVector(_OS_Timer0ISRHandler, TIMER0_INTERRUPT_INDEX);
		
#if ENABLE_SYNC_TIMER==1		// Setup SYNC timer
		OS_SetInterruptVector(_OS_Timer1ISRHandler, TIMER1_INTERRUPT_INDEX);		
#endif // ENABLE_SYNC_TIMER

		// Set the initialized flag
		initialized = 1;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Start the SYNC timer
// This function sets the first interrupt at SYNC_TIMER_INTERVAL + OS_FIRST_SCHED_DELAY
// Subsequent interrupts would come at SYNC_TIMER_INTERVAL
///////////////////////////////////////////////////////////////////////////////
#if ENABLE_SYNC_TIMER==1
void _OS_StartSyncTimer()
{
	// Below expression will be evaluated at compile time
	rTCNTB1 = ((((UINT64)TIMER1_TICK_FREQ * (SYNC_TIMER_INTERVAL + OS_FIRST_SCHED_DELAY)) 
				+ (1000000-1)) / 1000000);

	// Inform that Timer 1 Buffer has changed by updating manual update bit
	rTCON = (rTCON & (~0xf00)) | TIMER1_UPDATE;
	
	// Start timer 1
	rTCON = (rTCON & (~0xf00)) | (TIMER1_START | TIMER1_AUTORELOAD); 		
	
	// Now that the timer is running, subsequent interrupts should occur every SYNC_TIMER_INTERVAL
	// Below expression will be evaluated at compile time
	rTCNTB1 = ((((UINT64)TIMER1_TICK_FREQ * SYNC_TIMER_INTERVAL) + (1000000-1)) / 1000000);
}
#endif // ENABLE_SYNC_TIMER

///////////////////////////////////////////////////////////////////////////////
// Timer 0 & 1 Interrupt handler
///////////////////////////////////////////////////////////////////////////////
void _OS_TimerInterrupt(UINT32 timer)
{
	// We should keep the running without it generating an interrupt. 
	// Otherwise there will be a drift in the clock. 
	// Hence comment following code
	//rTCON &= ~(0x0f << (timer << 3));
	
	// Acknowledge the interrupt
	ACK_TIMER_INTERRUPT(TIMER0_INTERRUPT_INDEX + timer);

	// Since there has been an interrupt, the timer must have reloaded MAX_TIMER_COUNT
	// rTCNTB register. So reload the right value to use.
	switch(timer)
	{
		case TIMER0:
			timer0_count_buffer = rTCNTB0;
#if OS_ENABLE_CPU_STATS==1				
			sched_intr_counter++;
#endif	// OS_ENABLE_CPU_STATS
			break;
#if ENABLE_SYNC_TIMER==1
		case TIMER1:
			if(!(rTCON & TIMER0_RUNNING))	// Touch Timer0 only if it is not running
			{
				// As part of SYNC timer interrupt handling, we will start OS Timer with
				// highest timeout. This is done so that we can count for the elapsed time
				// later when we setup os_timer
				rTCNTB0 = MAX_TIMER_COUNT;
				timer0_count_buffer = MAX_TIMER_COUNT;
				rTCON = (rTCON & (~0x0f)) | TIMER0_UPDATE;
				rTCON = (rTCON & (~0x0f)) | (TIMER0_START | TIMER0_AUTORELOAD); 		
			}			
			// Keep track of number of SYNC interrupts
#if OS_ENABLE_CPU_STATS==1				
			sync_intr_counter++;
#endif	// OS_ENABLE_CPU_STATS
			break;
#endif // ENABLE_SYNC_TIMER
		default:
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Function to update the period in the timer
// The following function sets up the OS timer.
// Input:
// 		delay_in_us: Is the delay requested
// Output:
//		delay_in_us: Is the actual delay setup. Since every timer has a maximum interval,
//		the delay setup may be less than the delay requested.
// Return Value:
//		The budget spent if applicable
///////////////////////////////////////////////////////////////////////////////
UINT32 _OS_UpdateTimer(UINT32 * delay_in_us)
{
	UINT32 elapsed_count;
	UINT32 budget_spent_us;
	UINT32 req_count;
	
	if(delay_in_us)
	{	
		// Each timer has a maximum interval. Adjust the budget timeout accordingly
		if(*delay_in_us > MAX_TIMER0_INTERVAL_uS)
		{
			*delay_in_us = MAX_TIMER0_INTERVAL_uS;
		}

		req_count = CONVERT_us_TO_TICKS(*delay_in_us);
		Klog32(KLOG_OS_TIMER_SET, "OS Timer Set (us) - ", *delay_in_us);
	}
	else
	{
		req_count = 0;
		Klog32(KLOG_OS_TIMER_SET, "OS Timer Set (disabling) - ", 0);
	}
	
	// Calculate the elapsed time. There are following 4 cases:
	// 1. The OS timer has expired and the ISR has finished.
	//		In this case, timer0_count_buffer = MAX_TIMER_COUNT
	// 2. The OS timer has expired and but the ISR has not finished yet
	//		In this case, timer0_count_buffer < MAX_TIMER_COUNT but rTCNTO0 is > timer0_count_buffer
	// 3. The timer is still running	(not expired)
	// 4. The budget timer may have been disabled in the last period. 
	if(timer0_count_buffer == MAX_TIMER_COUNT)
	{
		// Case 1
		// Take this time from the new task, extra time is accounted for in the new task
		elapsed_count = (MAX_TIMER_COUNT - rTCNTO0) + OS_TIMER_ELAPSED_TIME_ADJ;	
		budget_spent_us = 0;							
	}
	else if(timer0_count_buffer > 0)
	{
		UINT32 tcount = rTCNTO0;
		if(tcount > timer0_count_buffer)
		{
			// Case 2
			// Take this time from the new task, extra time is accounted for in the new task
			elapsed_count = (MAX_TIMER_COUNT - tcount) + OS_TIMER_ELAPSED_TIME_ADJ;		
			budget_spent_us = 0;
		}
		else
		{
			// Case 3. We must have requested shorter timeout.
			// Extra time is accounted for in the old task. The time spent till now should be accounted for in the old task.
			elapsed_count = 0;								
			tcount = (timer0_count_buffer - tcount) + OS_TIMER_ELAPSED_TIME_ADJ;
			budget_spent_us = CONVERT_TICKS_TO_us(tcount);	// Convert it to us right away.
		}
	}
	else
	{
		// Case 4. The previous task must have been an Aperiodic task
		elapsed_count = 0;
		budget_spent_us = 0;
	}

	if(!delay_in_us || (*delay_in_us == 0))
	{
		// Disable the timer. 
		rTCON &= ~0x0f;
		rTCNTB0 = timer0_count_buffer = 0; 
	}
	else
	{
		if(req_count > elapsed_count) 
		{
			// The requested timeout is in the future. Update the terminal Count
			// and just resume counting
			rTCNTB0 = timer0_count_buffer = (req_count - elapsed_count);
			
			Klog32(KLOG_OS_TIMER_SET, "OS Timer Set - ", timer0_count_buffer);
		}
		else
		{
			// Highly undesirable situation. May need to adjust the task timings
			// In order to recover from this bad situation, give some non-zero time for the timer
			rTCNTB0 = timer0_count_buffer = 10;
			
#if OS_ENABLE_CPU_STATS==1			
			scheduler_miss_counter ++;
#endif	// OS_ENABLE_CPU_STATS

			Syslog32("KERNEL WARNING: Requested timeout is in the past ", elapsed_count);
		}
		
		// Inform that Timer 0 Buffer has changed by updating manual update bit
		rTCON = (rTCON & (~0x0f)) | TIMER0_UPDATE;
		rTCON = (rTCON & (~0x0f)) | (TIMER0_START | TIMER0_AUTORELOAD); 
		
		// Once manual update is done, we can change rTCNTB without affecting the current
		// timeout. We want the next cycle to start from the max value so that it is 
		// easy to compute elapsed time later on. This value will be loaded during auto-reload
		// after the counter reaches zero
		rTCNTB0 = MAX_TIMER_COUNT;		
	}
	
	// Keep track of max_scheduler_elapsed_time
#if OS_ENABLE_CPU_STATS==1	
	if((elapsed_count > 0) && (max_scheduler_elapsed_time < elapsed_count))
	{
		max_scheduler_elapsed_time = elapsed_count;
	}
#endif	// OS_ENABLE_CPU_STATS
	
	return budget_spent_us;
}

///////////////////////////////////////////////////////////////////////////////
// Converts the current timer count into micro seconds and returns.
///////////////////////////////////////////////////////////////////////////////
UINT32 _OS_GetTimerValue_us()
{
	return CONVERT_TICKS_TO_us(timer0_count_buffer - rTCNTO0);
}
