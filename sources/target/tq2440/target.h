///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File: target.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Board specific functions header file for TQ2440
//	
///////////////////////////////////////////////////////////////////////////////

#ifndef _TARGET_H_
#define _TARGET_H_

#include "os_types.h"

#define FIN 	(12000000)				// 12MHz Crystal

#define FCLK (405000000)				// Main Processor clock	405 MHz
#define HCLK (FCLK/3)					// AHB Clock	135 MHz
#define PCLK (HCLK/2)					// APB Clock	67.5 MHz
#define UCLK (48000000)					// USB Clock

#define	TIMER_PRESCALAR_0	(0x3f)		// PCLK/64
#define	TIMER0_DIVIDER		(0x00)		// PCLK/PRESCALAR0/2
#define	TIMER1_DIVIDER		(0x00)		// PCLK/PRESCALAR0/2
#define	TIMER0_TICK_FREQ	(527344)	// (PCLK/(TIMER_PRESCALAR_0+1)/2) - Resolution 1.8963 uSec per tick
#define	TIMER1_TICK_FREQ	(527344)	// (PCLK/(TIMER_PRESCALAR_0+1)/2) - Resolution 1.8963 uSec per tick

// (65535 * 1000000) / TIMER0_TICK_FREQ = 124273.78 uSec. Lets use 100ms for this.
// This way there will be at least one interrupt every 100ms
#define	MAX_TIMER0_INTERVAL_uS		100000		

// (65535 * 1000000) / TIMER1_TICK_FREQ = 124273.78 uSec. Lets use 100ms for this.
#define	MAX_TIMER1_INTERVAL_uS		100000

// Architecture related macros
#define _ARM_ARCH			4
#define _ARM_ARCH_v4

void _OS_TargetInit(void);

void configure_user_led(UINT32 led);
void user_led_on(UINT32 led);
void user_led_off(UINT32 led);
void user_led_toggle(UINT32 led);

#endif // _TARGET_H_
