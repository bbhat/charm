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
	TIMER0 = 0,
	TIMER1 = 1
};

#if defined(SOC_S5PV210)
	#define MAX_TIMER_COUNT		0xffffffff
#elif defined(SOC_S3C2440)
	#define MAX_TIMER_COUNT		0xffff
#endif

// Initializes both timer 0 & 1
void _OS_InitTimer ();

#if ENABLE_SYNC_TIMER==1

// Function to start the SYNC timer
void _OS_StartSyncTimer();

#endif	// ENABLE_SYNC_TIMER

#if defined(SOC_S3C2440)

	#define TIMER0_INTERRUPT_INDEX		10
	#define TIMER1_INTERRUPT_INDEX		11
	
	#define ACK_TIMER_INTERRUPT(intr_index)	{ rSRCPND = rINTPND = (1 << (intr_index)); }

#elif defined(SOC_S5PV210)

	#define TIMER0_INTERRUPT_INDEX		21
	#define TIMER1_INTERRUPT_INDEX		22

	#define ACK_TIMER_INTERRUPT(intr_index)	{ _vic_ack_irq(intr_index); }

#endif

// The following function sets up the OS timer.
// Input:
// 		delay_in_us: Is the delay requested
// Output:
//		delay_in_us: Is the actual delay setup. Since every timer has a maximum interval,
//		the delay setup may be less than the delay requested.
// Return Value:
//		The budget spent if applicable
UINT32 _OS_UpdateTimer(UINT32 * delay_in_us);

// This function should be called whenever the timer o/1 interrupt happens.
// This function clears the interrupt bits
// Input:
//		timer: 0/1
void _OS_TimerInterrupt(UINT32 timer);

// Read the timer value in uSec
UINT32 _OS_GetTimerValue_us(void);

#endif // _OS_TIMER_H_