//------------------------------------------------------------------------------
//
//						Copyright 2012-2013 xxxxxxx, xxxxxxx
//	File:	start.S
//	Author: Bala B. (bhat.balasubramanya//gmail.com)
//	Description: Boot for Mini210s target. Current version simply initializes
//		the hardware and waits for debugger to connect.
//
//-------------------------------------------------------------------------------

#include "s5pv210.h"

//-------------------------------------------------------------------------------
//				Memory Map
//	Exception Vector Table		0xD003_7400
//	Global Variable				0xD003_7480
//	Signature					0xD003_7580
//	SVC Stack					0xD003_7780
//	IRQ Stack					0xD003_7D80
//	BL1 Code Begin				0xD002_0010
//	SRAM Begin					0xD002_0000
//-------------------------------------------------------------------------------

#define REG_LED	0xE0200284
#define LED0	0xe
#define LED1	0xd
#define LED2	0xb
#define LED3	0x7

//-------------------------------------------------------------------------------
// Pre-defined constants
//-------------------------------------------------------------------------------
	USERMODE    = 0x10
	FIQMODE     = 0x11
	IRQMODE     = 0x12
	SVCMODE     = 0x13
	ABORTMODE   = 0x17
	UNDEFMODE   = 0x1b
	SYSMODE     = 0x1f
	MODEMASK    = 0x1f
	INTMASK     = 0xc0

	.section	.startup

	.global _start
_start:

	// Disable the WDT
	ldr	r0, =ELFIN_WATCHDOG_BASE			
	mov	r1, #0
	str	r1, [r0]

	// Reset all LEDs	
	mov r0, #0
	bl set_led

	// Initialize SVC Stack
	bl stack_init
				
	// Init SDRAM
	bl SDRAM_init						

	// Set LED0 to indicate that this step in the boot is over	
	ldr r0, =LED0
	bl set_led
	
halt_cpu:
	// Wait for the debugger to connect.
	// The current version of boot just prepares the target for debugging.
	// We need a full fledged Boot to be implemented later.
	// Issue #20
	b halt_cpu
	
	.section	.text
	
//---------------------------------------------------------------------
//  Initialize Stack
//---------------------------------------------------------------------
stack_init:
   mrs      r0, cpsr
   bic      r0, r0, #MODEMASK

   orr      r1, r0, #SVCMODE
   msr      cpsr_c, r1                      // Supervisor Mode
   ldr      sp, =__STACK_SVC_END__

   mov      pc, lr

//---------------------------------------------------------------------
//  Initialize Stack
//	r0 - should have leds to be set or reset
//---------------------------------------------------------------------
set_led:

	ldr r1, =REG_LED
	str r0, [r1]
	
	mov      pc, lr
	
//------------------------------------------------------------------------------
// The location for stacks
//------------------------------------------------------------------------------

	.section .stack
	
	.space   0x600
	
__STACK_SVC_END__:
