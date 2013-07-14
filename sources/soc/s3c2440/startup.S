//------------------------------------------------------------------------------
//
//						Copyright 2012 xxxxxxx, xxxxxxx
//	File:	startup.S
//	Author: Bala B. (bhat.balasubramanya//gmail.com)
//	Description: Processor Initialization code
//
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// Memory control
//-------------------------------------------------------------------------------
BWSCON  	=  0x48000000     // Bus width & wait status
BANKCON0	=  0x48000004     // Boot ROM control
BANKCON1	=  0x48000008     // BANK1 control
BANKCON2	=  0x4800000c     // BANK2 control
BANKCON3	=  0x48000010     // BANK3 control
BANKCON4	=  0x48000014     // BANK4 control
BANKCON5	=  0x48000018     // BANK5 control
BANKCON6	=  0x4800001c     // BANK6 control
BANKCON7	=  0x48000020     // BANK7 control
REFRESH 	=  0x48000024     // DRAM/SDRAM refresh
BANKSIZE	=  0x48000028     // Flexible Bank Size
MRSRB6  	=  0x4800002c     // Mode register set for SDRAM Bank6
MRSRB7  	=  0x48000030     // Mode register set for SDRAM Bank7

//-------------------------------------------------------------------------------
// CLOCK & POWER MANAGEMENT
//-------------------------------------------------------------------------------
LOCKTIME	=  0x4c000000     // PLL lock time counter
MPLLCON 	=  0x4c000004     // MPLL Control
UPLLCON 	=  0x4c000008     // UPLL Control
CLKCON  	=  0x4c00000c     // Clock generator control
CLKSLOW 	=  0x4c000010     // Slow clock control
CLKDIVN 	=  0x4c000014     // Clock divider control
CAMDIVN		=  0x4C000018	  // Camera clock divider register 

//-------------------------------------------------------------------------------
// INTERRUPT
//-------------------------------------------------------------------------------
SRCPND   	=  0x4a000000    // Interrupt r=est status
INTMOD   	=  0x4a000004    // Interrupt mode control
INTMSK   	=  0x4a000008    // Interrupt mask control
PRIORITY 	=  0x4a00000c    // IRQ priority control
INTPND   	=  0x4a000010    // Interrupt request status
INTOFFSET	=  0x4a000014    // Interruot request source offset
SUSSRCPND	=  0x4a000018    // Sub source pending
INTSUBMSK	=  0x4a00001c    // Interrupt sub mask

//-------------------------------------------------------------------------------
// I/O PORT for LED
//-------------------------------------------------------------------------------
GPFCON  	=  0x56000050     // Port F control
GPFDAT  	=  0x56000054     // Port F data
GPFUP   	=  0x56000058     // Pull-up control F

GPBCON  	=  0x56000010     // Port F control
GPBDAT  	=  0x56000014     // Port F data

//-------------------------------------------------------------------------------
//Miscellaneous registers
//-------------------------------------------------------------------------------
MISCCR  	=  0x56000080     // Miscellaneous control
DCKCON  	=  0x56000084     // DCLK0/1 control
EXTINT0 	=  0x56000088     // External interrupt control register 0
EXTINT1 	=  0x5600008c     // External interrupt control register 1
EXTINT2 	=  0x56000090     // External interrupt control register 2
EINTFLT0	=  0x56000094     // Reserved
EINTFLT1	=  0x56000098     // Reserved
EINTFLT2	=  0x5600009c     // External interrupt filter control register 2
EINTFLT3	=  0x560000a0     // External interrupt filter control register 3
EINTMASK	=  0x560000a4     // External interrupt mask
EINTPEND	=  0x560000a8     // External interrupt pending
GSTATUS0	=  0x560000ac     // External pin status
GSTATUS1	=  0x560000b0     // Chip ID(0x32440000)
GSTATUS2	=  0x560000b4     // Reset type
GSTATUS3	=  0x560000b8     // Saved data0(32-bit) before entering POWER_OFF mode
GSTATUS4	=  0x560000bc     // Saved data1(32-bit) before entering POWER_OFF mode
MSLCON		=  0x560000cc     // Memory sleep control register

//-------------------------------------------------------------------------------
// WATCHDOG TIMER
//-------------------------------------------------------------------------------
WTCON 		=  0x53000000       // Watch-dog timer mode
WTDAT 		=  0x53000004       // Watch-dog timer data
WTCNT 		=  0x53000008       // Eatch-dog timer count

//-------------------------------------------------------------------------------
// NAND FLASH
//-------------------------------------------------------------------------------
NFCONF  	=  0x4e000000
NECONT  	=  0x4e000004

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

//-------------------------------------------------------------------------------
// Frequency selector
//-------------------------------------------------------------------------------

// Fclk:Hclk:Pclk Ratio
// 0(1:1:1), 1(1:1:2), 2(1:2:2), 3(1:2:4), 4(1:4:4), 5(1:4:8), 6(1:3:3), 7(1:3:6).
CLKDIV_VAL = 7					// (1:3:6)

// Fin=12MHz Fout=405MHz
U_MDIV		=	0x7f
U_PDIV		=	0x2
U_SDIV		=	0x1

//-------------------------------------------------------------------------------
// MMU Cache/TLB/etc on/off functions 
//-------------------------------------------------------------------------------
R1_I    	=	(1<<12) 
R1_C		=	(1<<2) 
R1_A		=	(1<<1) 
R1_M		=	(1) 
R1_iA		=	(1<<31) 
R1_nF		=	(1<<30)


   	.section .text.startup
   	.code 32

   	.global _OS_Init
	.global _start	
	.global	_SVC_StackTop_
	.global _reset_handler

//---------------------------------------------------------------------
//  Start routine
//---------------------------------------------------------------------
_start:
_reset_handler:

    ldr	r0,=WTCON       // watch dog disable
    ldr	r1,=0x0
    str	r1,[r0]

    ldr	r0,=INTMSK
    ldr	r1,=0xffffffff  // all interrupt disable
    str	r1,[r0]

    ldr	r0,=INTSUBMSK
    ldr	r1,=0x7fff			// all sub interrupt disable
    str	r1,[r0]

	bl	InitPLL				// Initialize PLL
	
    bl  InitStacks          // Initialize Stack
    
    bl	_sysctl_init		// Initialize System Control Coprocessor
    
    bl	_set_svc_stack_top	// Initialize _SVC_STACK_TOP_

	// Cleear BSS section
    bl 	InitVariables               // Initialize Global Variables

    ldr	pc, =_OS_Init				// Call _OS_Init

    b     	.						// We should never return from _OS_Init

	.LTORG   
	
//---------------------------------------------------------------------
//  Initialize Stack
//---------------------------------------------------------------------
InitStacks:
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
InitVariables:
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
