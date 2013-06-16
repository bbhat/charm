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
	.global	_IRQHandler_
	.global	_SVC_StackTop_
	.global _platform_clock_init

//---------------------------------------------------------------------
//  Start routine
//---------------------------------------------------------------------

   	.section .text.startup
   	.code 32

_start:
ResetHandler:

	// Disable the WDT
	ldr	r0, =ELFIN_WATCHDOG_BASE			
	mov	r1, #0
	str	r1, [r0]

	bl	_mask_interrupts			// Mask all interrupts

	bl	_platform_clock_init		// Initialize PLL
	
    bl  _init_stacks          		// Initialize Stack

    bl 	_init_bss               	// Clear BSS section

    ldr	pc, =_OS_Init				// Call _OS_Init

    b     	.						// We should never return from _OS_Init

	.LTORG   

//---------------------------------------------------------------------
//  Function to mask all interrupts
//---------------------------------------------------------------------
_mask_interrupts:

	// Mask all interrupts in VIC0
    ldr	r0,=ELFIN_VIC0_BASE_ADDR
    ldr	r1,=0xFFFFFFFF
    str	r1,[r0, #oINTMSK]
    str	r1,[r0, #oINTSUBMSK]

	// Mask all interrupts in VIC1
    ldr	r0,=ELFIN_VIC1_BASE_ADDR
    ldr	r1,=0xFFFFFFFF
    str	r1,[r0, #oINTMSK]
    str	r1,[r0, #oINTSUBMSK]

	// Mask all interrupts in VIC2
    ldr	r0,=ELFIN_VIC2_BASE_ADDR
    ldr	r1,=0xFFFFFFFF
    str	r1,[r0, #oINTMSK]
    str	r1,[r0, #oINTSUBMSK]
    
    // Mask all interrupts in VIC3
    ldr	r0,=ELFIN_VIC3_BASE_ADDR
    ldr	r1,=0xFFFFFFFF
    str	r1,[r0, #oINTMSK]
    str	r1,[r0, #oINTSUBMSK]

	mov pc, lr
	
    .LTORG 
	
//---------------------------------------------------------------------
//  Function to set custom IRQ Interrupt vector
// OS_InterruptVector OS_SetInterruptVector(OS_InterruptVector isr, UINT32 index)
//---------------------------------------------------------------------
	.global OS_SetInterruptVector
OS_SetInterruptVector:

	ldr		r2,=HandleEINT0		// Load the first IRQ handler address
	add		r2, r1, LSL #2
	swp		r0, r0, [r2]		// Read the old interrupt vector and replace with the new one
	
	mov		r3, #1
	mov		r3, r3, LSL r1
	ldr		r2,=INTMSK			// Clear appropriate bit in INTMSK
	ldr		r1, [r2]			// Read INTMSK
	bic		r1, r1, r3
	str		r1, [r2]
		
	mov      pc, lr
	
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
//  Function to initialize PLL
//---------------------------------------------------------------------
InitPLL:

    // To reduce PLL lock time, adjust the LOCKTIME register.
    ldr		r0,=LOCKTIME
    ldr		r1,=0xffffff
    str		r1,[r0]

    // Setting CAMDIVN to zero
    ldr		r0,=CAMDIVN
    mov		r1,#0
    str		r1,[r0]

    // Setting value Fclk:Hclk:Pclk
    ldr		r0,=CLKDIVN
    ldr		r1,=CLKDIV_VAL
    str		r1,[r0]

	// If HDIVN is not 0, the CPU bus mode has to be changed from the fast bus mode to the asynchronous
	// bus mode using following instructions(S3C2440 does not support synchronous bus mode).
	mrc 	p15,0,r0,c1,c0,0
	orr 	r0,r0,#(R1_nF | R1_iA)
	mcr 	p15,0,r0,c1,c0,0

    // Configure MPLL
    ldr		r0,=MPLLCON
    ldr 	r1,=((U_MDIV<<12) | (U_PDIV<<4) | (U_SDIV))		// Fin=12.00MHz	Fout=405.00 MHz
    str		r1,[r0]

	mov   	pc, lr
	
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
// Some initializations
// TODO: More initializations if needed
//-------------------------------------------------------------------------------

	.global _OS_SystemInit
_OS_SystemInit:

	// Update _SVC_STACK_TOP_
	ldr		r2,=_SVC_STACK_TOP_
	ldr		r3,=__STACK_SVC_END__
	str		r3, [r2]
	
	mov   	pc, lr

	.LTORG   

//------------------------------------------------------------------------------
// Interrupt vector in DRAM. We get control into this section from the 
// Vector table in the flash at address zero
//------------------------------------------------------------------------------
	.section .text.vector       // AREA ???, CODE, READONLY
	.code 32             		// CODE32
	
HandleReset:  					// Reset  
	b		ResetHandler
	.space  28		
HandleUndef:         			// Undefined Instruction Exception
	b		.
	.space  28		
HandleSWI:      				// Software Interrupt Exception
	b		.
	.space  28		
HandlePabort:   				// Prefetch Abort Exception
	b		.
	.space  28		
HandleDabort:   				// Data Abort Exception
	b		.
	.space  28		
HandleReserved: 				// Reserved
	b		.
	.space  28		
HandleIRQ:      				// IRQ Exception
	b		_IRQHandler_
	.space  28		
HandleFIQ:      				// FIQ Exception
	b		.
	.space  28		

// IRQ Interrupts

	.global		_IRQ_TABLE_
_IRQ_TABLE_:

HandleEINT0:	mov    pc, lr
HandleEINT1:	mov    pc, lr
HandleEINT2:	mov    pc, lr
HandleEINT3:	mov    pc, lr
HandleEINT4_7:	mov    pc, lr
HandleEINT8_23:	mov    pc, lr
HandleCAM:		mov    pc, lr		// Added for 2440.
HandleBATFLT:	mov    pc, lr
HandleTICK:		mov    pc, lr
HandleWDT:		mov    pc, lr
HandleTIMER0: 	mov    pc, lr
HandleTIMER1: 	mov    pc, lr
HandleTIMER2: 	mov    pc, lr
HandleTIMER3: 	mov    pc, lr
HandleTIMER4: 	mov    pc, lr
HandleUART2:  	mov    pc, lr
HandleLCD: 		mov    pc, lr
HandleDMA0:		mov    pc, lr
HandleDMA1:		mov    pc, lr
HandleDMA2:		mov    pc, lr
HandleDMA3:		mov    pc, lr
HandleMMC:		mov    pc, lr
HandleSPI0:		mov    pc, lr
HandleUART1:	mov    pc, lr
HandleNFCON:	mov    pc, lr		// Added for 2440.
HandleUSBD:		mov    pc, lr
HandleUSBH:		mov    pc, lr
HandleIIC:		mov    pc, lr
HandleUART0: 	mov    pc, lr
HandleSPI1: 	mov    pc, lr
HandleRTC: 		mov    pc, lr
HandleADC: 		mov    pc, lr
VectorTableEnd:	mov    pc, lr


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
