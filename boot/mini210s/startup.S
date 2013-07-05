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

#define ELFIN_VIC0_BASE_ADDR		(0xF2000000)
#define ELFIN_VIC1_BASE_ADDR		(0xF2100000)
#define ELFIN_VIC2_BASE_ADDR		(0xF2200000)
#define ELFIN_VIC3_BASE_ADDR		(0xF2300000)

#define GPJ2CON						(0xE0200280)
#define GPD0CON						(0xE02000A0)
#define GPD0DAT						(0xE02000A4)

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
// Global Symbols
//-------------------------------------------------------------------------------
	.global lcd_init
	.global lcd_clear_screen
	.global SDRAM_init
	.global _platform_clock_init

//-------------------------------------------------------------------------------
// Startup code
// NOTE: that iROM has alredy done lot of work. Refer to S5PV210 RISC Microprocessor
// IROM(Internal ROM) Booting Application Note
//
// iROM(BL0) boot-up sequence (Refer 2.3 V210 boot-up diagram)
// 1. Disable the Watch-Dog Timer
// 2. Initialize the instruction cache
// 3. Initialize the stack region (see “memory map” on chap 2.5)
// 4. Initialize the heap region. (see “memory map” on chap 2.5)
// 5. Initialize the Block Device Copy Function. (see “Device Copy Function” on chap 2.7)
// 6. Initialize the PLL and Set system clock. (see “clock configuration” on chap 2.11)
// 7. Copy the BL1 to the internal SRAM region (see “Device Copy Function” on chap 2.7)
// 8. Verify the checksum of BL1.
// If checksum fails, iROM will try the second boot up. (SD/MMC channel 2)
// 9. Check if it is secure-boot mode or not. In Mini210s board, it is NOT secure boot.
// 		If the security key value is written in S5PV210, It’s secure-boot mode.
// 		If it is secure-boot mode, verify the integrity of BL1.
// 10. Jump to the start address of BL1
//-------------------------------------------------------------------------------

	.section	.startup
	
//-------------------------------------------------------------------------------
// Start function:
// Here we should not repeat the iROM functionality
// Note that stack is alredy setup in iROM
//-------------------------------------------------------------------------------
	.global _start
_start:

	// Disable the WDT
	ldr	r0, =ELFIN_WATCHDOG_BASE			
	mov	r1, #0
	str	r1, [r0]

	// Configure GPIO for LEDs
	bl _configure_leds
	
	// Reset all LEDs	
	mov r0, #0
	bl _set_leds
	
	// Initialize SVC Stack
	bl _stack_init
	
	// Mask all interrupts
	bl _mask_interrupts

	// Disable Interrupts
	bl _disable_interrupt
		
	// Initialize clocks
	bl _platform_clock_init		
		
	// Init SDRAM
	bl SDRAM_init			

	// Shut off Buzzer
	bl _shutup_buzzer
	
	// Clear the screen
	ldr r0, =0x1234
	bl lcd_clear_screen

	// Handle LCD
	bl lcd_init
	
	// Set LED0 to indicate that this step in the boot is over	
	ldr r0, =LED0
	bl _set_leds
	
	// The current version of boot just prepares the target for debugging.
	// We need a full fledged Boot to be implemented later.
	// Issue #20

halt_cpu:

	// Wait for interrupt or debugger to connect. This wakes up CPU even if the interrupt is masked.
	wfe
	
	// Just in case	
	b halt_cpu
	
		
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
	
//---------------------------------------------------------------------
// Helper function: _shutup_buzzer
// Unless we configure the GPIO to output 0, Buzzer is ON
//---------------------------------------------------------------------
_shutup_buzzer:
		
	// Configure GPIO as output
	ldr		r1, =GPD0CON
	ldr		r0, [r1]
	orr		r0, r0, #1
	str		r0, [r1]

	ldr		r1, =GPD0DAT
	ldr		r0, [r1]
	bic		r0, r0, #1
	str		r0, [r1]
	
    mov 	pc, lr

//---------------------------------------------------------------------
//  Initialize Stack
//---------------------------------------------------------------------
_stack_init:
	mrs     r0, cpsr
	bic     r0, r0, #MODEMASK
	orr     r1, r0, #SVCMODE
	msr     cpsr_c, r1                      // Supervisor Mode
	ldr     sp, =__STACK_SVC_END__

	mov		pc, lr


//---------------------------------------------------------------------
// Helper function: _shutdown_lcd
// By default LCD turns ON
//---------------------------------------------------------------------
_shutdown_lcd:
		
	
    mov 	pc, lr
    
//---------------------------------------------------------------------
//  Helper function: _disable_interrupt
//  UINT32 _disable_interrupt(void)
//---------------------------------------------------------------------
	.global _disable_interrupt
_disable_interrupt:
	mrs 	r0,cpsr
	orr 	r1,r0,#NOINT
	msr 	cpsr_c,r1
	and		r0, r0,#INTMASK		// Return old interrupt bits
	mov		pc, lr
	
//---------------------------------------------------------------------
//  Helper function: _disable_interrupt
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
	
//---------------------------------------------------------------------
//	configure LEDs
//---------------------------------------------------------------------
_configure_leds:
	ldr		r1, =GPJ2CON
	ldr		r0, =0x00001111		// Configure as output pins
	str		r0, [r1]
	
	mov     pc, lr

//---------------------------------------------------------------------
//	r0 - should have leds to be set or reset
//---------------------------------------------------------------------
_set_leds:

	ldr r1, =REG_LED
	str r0, [r1]
	
	mov      pc, lr

//------------------------------------------------------------------------------
// The location for stacks
//------------------------------------------------------------------------------
	.section .stack

	.space   0x600
	
__STACK_SVC_END__:
