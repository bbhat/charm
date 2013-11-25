///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	os_timer.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: OS Timer implementation
//	
///////////////////////////////////////////////////////////////////////////////

#ifndef _OS_TIMER_H_
#define _OS_TIMER_H_

#include "os_core.h"
#include "os_types.h"

#if defined(SOC_S5PV210)
	#include "vic.h"
#endif

enum
{
    PERIODIC_TIMER = 0,
    TIMER0 = 0,
	BUDGET_TIMER = 1,
    TIMER1 = 1
};

#if defined(SOC_S5PV210)
	#define MAX_TIMER_COUNT		0xffffffff
#elif defined(SOC_S3C2440)
	#define MAX_TIMER_COUNT		0xffff
#endif

// Initializes both timer 0 & 1
void _OS_InitTimer ();

// Function to start the SYNC timer
void _OS_StartSyncTimer();

#if defined(SOC_S3C2440)

	#define TIMER0_INTERRUPT_INDEX		10
	#define TIMER1_INTERRUPT_INDEX		11
	
	#define ACK_TIMER_INTERRUPT(timer)	{ rSRCPND = rINTPND = (1 << (TIMER0_INTERRUPT_INDEX + (timer))); }

#elif defined(SOC_S5PV210)

	#define TIMER0_INTERRUPT_INDEX		21
	#define TIMER1_INTERRUPT_INDEX		22

	#define ACK_TIMER_INTERRUPT(timer)	{ \
			_vic_ack_irq(TIMER0_INTERRUPT_INDEX + (timer)); \
			rTINT_CSTAT |= (0x20 << timer); \
		}

#endif

// TODO: I need to make below calculations extremely efficient.
// Use 64 bit calculation for accuracy and always round up
#define CONVERT_TMR0_us_TO_TICKS(us)	((UINT32)(TIMER0_TICK_PER_us * (us)))
#define CONVERT_TMR1_us_TO_TICKS(us)	((UINT32)(TIMER1_TICK_PER_us * (us)))
#define CONVERT_TMR0_TICKS_TO_us(tick)	((UINT32)((tick) * TIMER0_us_PER_TICK + 0.5))
#define CONVERT_TMR1_TICKS_TO_us(tick)	((UINT32)((tick) * TIMER1_us_PER_TICK + 0.5))

// Timer functions
void _OS_Timer_AckInterrupt(UINT32 timer);
void _OS_Timer_PeriodicTimerStart(UINT32 interval_us);
void _OS_Timer_SetTimeout_us(UINT32 timer);
void _OS_Timer_SetMaxTimeout(void);
void _OS_Timer_Disable(UINT32 timer);
UINT32 _OS_Timer_GetTimeElapsed_us(UINT32 timer);
UINT32 _OS_Timer_GetCount(UINT32 timer);
UINT32 _OS_Timer_GetMaxCount(UINT32 timer);

// Timer ISR
void _OS_PeriodicTimerISR(void *arg);
void _OS_BudgetTimerISR(void *arg);

#endif // _OS_TIMER_H_
