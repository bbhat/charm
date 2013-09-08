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
#define CONVERT_us_TO_TICKS(us)		((((UINT64)TIMER1_TICK_FREQ * (us)) + (1000000-1)) / 1000000)
#define CONVERT_TMR0_TICKS_TO_us(tick)	((((tick) * 1000000ull) + (TIMER0_TICK_FREQ - 1)) / TIMER0_TICK_FREQ)
#define CONVERT_TMR1_TICKS_TO_us(tick)	((((tick) * 1000000ull) + (TIMER1_TICK_FREQ - 1)) / TIMER1_TICK_FREQ)

#define TIMER0_START		0x001
#define TIMER0_RUNNING		0x001
#define TIMER0_UPDATE		0x002
#define TIMER0_AUTORELOAD	0x008

#define TIMER1_START		0x100
#define TIMER1_UPDATE		0x200
#define TIMER1_AUTORELOAD	0x800

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
		ACK_TIMER_INTERRUPT(TIMER0);
		ACK_TIMER_INTERRUPT(TIMER1);
						
		// Configure the two prescalars
		rTCFG0 = (rTCFG0 & 0xffffff00) | TIMER_PRESCALAR_0;	 // Set the Pre-scaler 0
		rTCFG1 = (rTCFG1 & 0xffffff00) | (TIMER1_DIVIDER << 4) | TIMER0_DIVIDER;		// Set the divider
		
		// Set the interrupt handlers. This also unmasks that interrupt
		OS_SetInterruptVector(_OS_PeriodicTimerISR, TIMER0_INTERRUPT_INDEX);

#if defined(SOC_S5PV210)
		rTINT_CSTAT |= (1 << TIMER0);	// Enable Timer0 interrupt
#endif
		
		OS_SetInterruptVector(_OS_BudgetTimerISR, TIMER1_INTERRUPT_INDEX);
		
#if defined(SOC_S5PV210)
		rTINT_CSTAT |= (1 << TIMER1);	// Enable Timer1 interrupt
#endif
	
		// Set the initialized flag
		initialized = 1;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Start the PERIODIC timer. This should be a auto reloading timer
///////////////////////////////////////////////////////////////////////////////
void _OS_Timer_PeriodicTimerStart(UINT32 interval_us)
{
	// Below expression will be evaluated at compile time
	rTCNTB0 = ((((UINT64)TIMER0_TICK_FREQ * interval_us) + (1000000-1)) / 1000000);

	// Inform that Timer 0 Buffer has changed by updating manual update bit
	rTCON = (rTCON & (~0x0f)) | TIMER0_UPDATE;
	
	// Start timer 0
	rTCON = (rTCON & (~0xf)) | (TIMER0_START | TIMER0_AUTORELOAD);
}

///////////////////////////////////////////////////////////////////////////////
// Timer 0 & 1 Interrupt handler
///////////////////////////////////////////////////////////////////////////////
void _OS_Timer_AckInterrupt(UINT32 timer)
{
	// We should keep the running without it generating an interrupt. 
	// Otherwise there will be a drift in the clock. 
	// Hence comment following code
	//rTCON &= ~(0x0f << (timer << 3));
	
	// Acknowledge the interrupt
	ACK_TIMER_INTERRUPT(timer);
}

///////////////////////////////////////////////////////////////////////////////
// Function to update timer interval
// The following function sets up the OS timer.
// Input:
// 		delay_in_us: Is the delay requested
///////////////////////////////////////////////////////////////////////////////
void _OS_Timer_SetTimeout_us(UINT32 delay_in_us)
{
    UINT32 req_count;
    
    // Validate the input
    ASSERT(delay_in_us <= MAX_TIMER1_INTERVAL_uS);

    Klog32(KLOG_OS_TIMER_SET, "Budget Timer Set (us) - ", delay_in_us);
    
    rTCNTB0 = CONVERT_us_TO_TICKS(delay_in_us);
    
    // Inform that Timer 1 Buffer has changed by updating manual update bit
    // We are going to use Timer 1 as one shot timer.
    rTCON = (rTCON & (~0xf00)) | TIMER1_UPDATE;
    rTCON = (rTCON & (~0xf00)) | TIMER1_START;
}

void _OS_Timer_Disable(UINT32 timer)
{
    rTCON &= ~(0x0f << (timer << 3));
}

///////////////////////////////////////////////////////////////////////////////
// Converts the current timer count into micro seconds and returns.
///////////////////////////////////////////////////////////////////////////////
UINT32 _OS_Timer_GetCurTime_us(UINT32 timer)
{
    if(timer == PERIODIC_TIMER)
        return CONVERT_TMR0_TICKS_TO_us(rTCNTO0);
    else
        return CONVERT_TMR1_TICKS_TO_us(rTCNTO1);
}
