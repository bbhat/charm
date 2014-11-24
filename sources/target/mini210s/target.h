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

#define FIN 	(24000000)				// 24MHz Crystal

// PLL 
#define APLL				(1000000000)
#define MPLL				(667000000)

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
#define HCLK_MSYS			(ARMCLK / (DIVHCLKM+1))	                    // 200 MHz
#define PCLK_MSYS 			(HCLK_MSYS / (DIVPCLKM+1))					// 100 MHz

// DSYS Domain will use MPLL
#define HCLK_DSYS			(MPLL / (DIVHCLKD+1))						// 166.75 MHz
#define PCLK_DSYS 			(HCLK_DSYS / (DIVPCLKD+1))				    // 83.375 MHz

// PSYS Domain will use MPLL	
#define HCLK_PSYS			(MPLL / (DIVHCLKP+1))						// 133.4 MHz
#define PCLK_PSYS 			(HCLK_PSYS / (DIVPCLKP+1))					// 66.7 MHz

#define	TIMER_PRESCALAR_0	(0x01)		// PCLK/2
#define	TIMER0_DIVIDER		(0x01)		// PCLK/PRESCALAR0/2
#define	TIMER1_DIVIDER		(0x01)		// PCLK/PRESCALAR0/2
#define	TIMER0_TICK_PER_us	(16.675)	// (PCLK/(TIMER_PRESCALAR_0+1)/2) - Resolution 0.05997 uSec per tick
#define	TIMER1_TICK_PER_us	(16.675)	// (PCLK/(TIMER_PRESCALAR_0+1)/2) - Resolution 0.05997 uSec per tick
#define	TIMER0_us_PER_TICK	(0.05997)	// (PCLK/(TIMER_PRESCALAR_0+1)/2) - Resolution 0.05997 uSec per tick
#define	TIMER1_us_PER_TICK	(0.05997)	// (PCLK/(TIMER_PRESCALAR_0+1)/2) - Resolution 0.05997 uSec per tick

// (0xffffffff * 1000000) / TIMER0_TICK_FREQ. Lets use 1 second for this.
#define	MAX_TIMER0_INTERVAL_uS		1000000

// (0xffffffff * 1000000) / TIMER1_TICK_FREQ. Lets use 1 second for this.
#define	MAX_TIMER1_INTERVAL_uS		1000000

//-----------------------------------------------------------------------
// LCD related defines
//-----------------------------------------------------------------------
#define GPF0CON			(*(volatile unsigned long *)0xE0200120)
#define GPF1CON			(*(volatile unsigned long *)0xE0200140)
#define GPF2CON			(*(volatile unsigned long *)0xE0200160)
#define GPF3CON			(*(volatile unsigned long *)0xE0200180)

#define GPD0CON			(*(volatile unsigned long *)0xE02000A0)
#define GPD0DAT			(*(volatile unsigned long *)0xE02000A4)

#define CLK_SRC1		(*(volatile unsigned long *)0xe0100204)
#define CLK_DIV1		(*(volatile unsigned long *)0xe0100304)
#define DISPLAY_CONTROL	(*(volatile unsigned long *)0xe0107008)

#define DISP_CONTROLLER_BASE_ADDR	0xF8000000
#define VIDCON0			(*(volatile unsigned long *)0xF8000000)
#define VIDCON1			(*(volatile unsigned long *)0xF8000004)
#define VIDTCON2		(*(volatile unsigned long *)0xF8000018)
#define WINCON0 		(*(volatile unsigned long *)0xF8000020)
#define WINCON2 		(*(volatile unsigned long *)0xF8000028)
#define SHADOWCON 		(*(volatile unsigned long *)0xF8000034)
#define VIDOSD0A 		(*(volatile unsigned long *)0xF8000040)
#define VIDOSD0B 		(*(volatile unsigned long *)0xF8000044)
#define VIDOSD0C 		(*(volatile unsigned long *)0xF8000048)

#define VIDW00ADD0B0 	(*(volatile unsigned long *)0xF80000A0)
#define VIDW00ADD1B0 	(*(volatile unsigned long *)0xF80000D0)

#define VIDTCON0 		(* (volatile unsigned long *) 0xF8000010)
#define VIDTCON1 		(* (volatile unsigned long *) 0xF8000014)

#define HSPW 		(0)
#define HBPD 		(40 - 1)
#define HFPD 		(5 - 1)
#define VSPW 		(0)
#define VBPD 		(8 - 1)
#define VFPD 		(8 - 1)

// Frame Buffer Address
extern void * FB_ADDR;
extern const UINT32 FB_SIZE;

#define FB_WIDTH 	(480)
#define FB_HEIGHT 	(272)
#define LINEVAL 	(FB_HEIGHT-1)
#define HOZVAL 		(FB_WIDTH-1)
#define LeftTopX 	(0)
#define LeftTopY 	(0)
#define RightBotX 	(FB_WIDTH-1)
#define RightBotY 	(FB_HEIGHT-1)

#define TARGET_HAS_LCD		1
#define BACKLIGHT_ON		1

//-----------------------------------------------------------------------
// Architecture related macros
//-----------------------------------------------------------------------
#define _ARM_ARCH			7
#define _ARM_ARCH_v7
#define _ARM_ARCH_v7a

//-----------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------
void _OS_TargetInit(void);

void configure_user_led(UINT32 led);
void user_led_on(UINT32 led);
void user_led_off(UINT32 led);
void user_led_toggle(UINT32 led);

void lcd_init (void);

#endif // _TARGET_H_
