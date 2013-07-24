//------------------------------------------------------------------------------
//
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	startup.s
//	Author: Bala B. (bhat.balasubramanya//gmail.com)
//	Description: Processor Initialization code
//
//-------------------------------------------------------------------------------

#include "soc.h"

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

NOINT  		= 0xc0

   	.global _OS_Init
	.global _start
	.global	_SVC_StackTop_
	.global _platform_clock_init
	.global _reset_handler

//---------------------------------------------------------------------
//  Start routine
//---------------------------------------------------------------------

   	.section .text
   	.code 32

_start:
_reset_handler:

	// Disable the WDT
	ldr	r0, =ELFIN_WATCHDOG_BASE			
	mov	r1, #0
	str	r1, [r0]
	
	// Mask all interrupts
	bl	_mask_interrupts			// Mask all interrupts

#ifndef DEBUG

	// While debugging, we cannot change the clock. 
	// Since this is already done in the boot it is not necessary to do it any way
	bl	_platform_clock_init		// Initialize PLL
	
#endif
	
    bl  _init_stacks          		// Initialize Stack
    
    bl	_sysctl_init				// Initialize System Control Coprocessor
    
    bl	_sysctl_disable_icache		// Disable instruction cache. iROM enables it at reset
	
    bl	_set_svc_stack_top			// Initialize _SVC_STACK_TOP_

    bl 	_init_bss               	// Clear BSS section

    ldr	pc, =_OS_Init				// Call _OS_Init

    b     .							// We should never return from _OS_Init

	.LTORG   

//---------------------------------------------------------------------
//  Function to mask all interrupts
//---------------------------------------------------------------------
_mask_interrupts:

	// Mask all interrupts in VIC0
    ldr		r0,=ELFIN_VIC0_BASE_ADDR
    ldr		r1,=0xFFFFFFFF
    str		r1,[r0, #oINTMSK]
    str		r1,[r0, #oINTSUBMSK]

	// Mask all interrupts in VIC1
    ldr		r0,=ELFIN_VIC1_BASE_ADDR
    ldr		r1,=0xFFFFFFFF
    str		r1,[r0, #oINTMSK]
    str		r1,[r0, #oINTSUBMSK]

	// Mask all interrupts in VIC2
    ldr		r0,=ELFIN_VIC2_BASE_ADDR
    ldr		r1,=0xFFFFFFFF
    str		r1,[r0, #oINTMSK]
    str		r1,[r0, #oINTSUBMSK]
    
    // Mask all interrupts in VIC3
    ldr		r0,=ELFIN_VIC3_BASE_ADDR
    ldr		r1,=0xFFFFFFFF
    str		r1,[r0, #oINTMSK]
    str		r1,[r0, #oINTSUBMSK]

	mov 	pc, lr
	
    .LTORG 
	
//---------------------------------------------------------------------
//  Initialize Stack
//---------------------------------------------------------------------
_init_stacks:
   mrs      r0, cpsr
   bic      r0, r0, #MODEMASK
   orr      r1, r0, #IRQMODE
   msr      cpsr_c, r1                      // IRQ Mode
   ldr      sp, =__STACK_IRQ_END__

   orr      r1, r0, #FIQMODE
   msr      cpsr_c, r1                      // FIQ Mode
   ldr      sp, =__STACK_FIQ_END__

   orr      r1, r0, #UNDEFMODE
   msr      cpsr_c, r1                      // UND Mode
   ldr      sp, =__STACK_UNDEF_END__

   orr      r1, r0, #SVCMODE
   msr      cpsr_c, r1                      // Supervisor Mode
   ldr      sp, =__STACK_SVC_END__

   mov      pc, lr

   .LTORG   

//---------------------------------------------------------------------
//  Initialize Data Region
//---------------------------------------------------------------------
_init_bss:
   	// zero bss
_Lzero_bss:
   	ldr   	r0, =__bss_start__
   	ldr   	r1, =__bss_end__
   	mov   	r2, #0
_Lzero_words:
   	cmp   	r0, r1
   	strne   r2, [r0], #0x4
   	blt   	_Lzero_words

   	mov   	pc, lr

   	.LTORG   

//---------------------------------------------------------------------
//  OS Helper function: _disable_interrupt
//  UINT32 _disable_interrupt(void)
//---------------------------------------------------------------------
	.global _disable_interrupt
_disable_interrupt:
	mrs 	r0,cpsr
	orr 	r1,r0,#NOINT
	msr 	cpsr_c,r1
	and		r0, r0,#INTMASK		// Return old interrupt bits
	mov		pc, lr
	
	.LTORG   	

//---------------------------------------------------------------------
//  OS Helper function: _disable_interrupt
//  void _enable_interrupt(UINT32 arg)
//  Here the argument is the value returned by _disable_interrupt
//---------------------------------------------------------------------
	.global _enable_interrupt
_enable_interrupt:
	mrs 	r1,cpsr
	bic 	r1,r1,#INTMASK
	orr		r1,r1,r0		// Mask 
	msr 	cpsr_c,r1
	mov   	pc, lr
	
	.LTORG   

//-------------------------------------------------------------------------------
// Initialize _SVC_STACK_TOP_
//-------------------------------------------------------------------------------

	.global _set_svc_stack_top
_set_svc_stack_top:

	// Update _SVC_STACK_TOP_ variable
	ldr		r2,=_SVC_STACK_TOP_
	ldr		r3,=__STACK_SVC_END__
	str		r3, [r2]
		
	mov   	pc, lr

	.LTORG   

//------------------------------------------------------------------------------
// Some global variables
//------------------------------------------------------------------------------
	.section .data

	.global	_SVC_STACK_TOP_
_SVC_STACK_TOP_:
	.space		4		// Space for SVC stack top pointer
	
	
//------------------------------------------------------------------------------
// The location for stacks
//------------------------------------------------------------------------------
	.section .stack
	.space   0x1000
	
_SYSTEM_STACK:

// Assign Start address of each stacks

__STACK_SVC_END__	=	(_SYSTEM_STACK-0x500)	
__STACK_UNDEF_END__	=	(_SYSTEM_STACK-0x300)	
__STACK_ABORT_END__	=	(_SYSTEM_STACK-0x200)	
__STACK_IRQ_END__	=	(_SYSTEM_STACK-0x100)	
__STACK_FIQ_END__	=	(_SYSTEM_STACK-0x0)	

   .END
