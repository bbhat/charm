///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File: target.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Board specific functions header file for mini210s
//	
///////////////////////////////////////////////////////////////////////////////

#ifndef _TARGET_H_
#define _TARGET_H_

#include "os_types.h"

#if defined(TARGET_MINI210S)

	#define FIN 	(24000000)				// 24MHz Crystal
	
	// PLL 
	#define APLL						(1000000000)
	#define MPLL						(667000000)
	
	// Clock Dividers for best performance
	#define DIVAPLL				0
	#define DIVA2M				4
	#define DIVHCLKM			4
	#define DIVPCLKM			1
	
	#define DIVHCLKD			3
	#define DIVPCLKD			1
	
	#define DIVHCLKP			4
	#define DIVPCLKP			1

	// MSYS Domain will use APLL
	#define ARMCLK				(APLL / (DIVAPLL+1))						// 1GHz
	#define HCLK_MSYS			((APLL / (DIVAPLL+1)) / (DIVHCLKM+1))		// 200 MHz
	#define PCLK_MSYS 			(HCLK_MSYS / (DIVPCLKM+1))					// 100 MHz

	// DSYS Domain will use MPLL
	#define HCLK_DSYS			(MPLL / (DIVHCLKD+1))							// 166.75 MHz
	#define PCLK_DSYS 			(HCLK_DSYS / (DIVPCLKD+1))				// 83.375 MHz

	// PSYS Domain will use MPLL	
	#define HCLK_PSYS			(MPLL / (DIVHCLKP+1))							// 133.4 MHz
	#define PCLK_PSYS 			(HCLK_PSYS / (DIVPCLKP+1))					// 66.7 MHz

	#define	TIMER_PRESCALAR_0	(0x07)		// PCLK/8
	#define	TIMER0_DIVIDER		(0x02)		// PCLK/PRESCALAR0/4
	#define	TIMER1_DIVIDER		(0x02)		// PCLK/PRESCALAR0/4
	#define	TIMER0_TICK_FREQ	(2084375)	// (PCLK/(TIMER_PRESCALAR_0+1)/4) - Resolution 0.47976 uSec per tick
	#define	TIMER1_TICK_FREQ	(2084375)	// (PCLK/(TIMER_PRESCALAR_0+1)/4) - Resolution 0.47976 uSec per tick

	// (0xffffffff * 1000000) / TIMER0_TICK_FREQ = 2060.554 Seconds. Lets use 1 second for this.
	#define	MAX_TIMER0_INTERVAL_uS		1000000

	// (0xffffffff * 1000000) / TIMER1_TICK_FREQ = 2060.554 Seconds. Lets use 1 second for this.
	#define	MAX_TIMER1_INTERVAL_uS		1000000

#endif

void _OS_TargetInit(void);

void configure_user_led(UINT32 led);
void user_led_on(UINT32 led);
void user_led_off(UINT32 led);
void user_led_toggle(UINT32 led);

#endif // _TARGET_H_
