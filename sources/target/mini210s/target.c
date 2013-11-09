///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	target.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Board specific functions source file
//	
///////////////////////////////////////////////////////////////////////////////

#include "target.h"
#include "soc.h"
#include "mmu.h"

void _OS_TargetInit(void)
{
#if ENABLE_MMU
	// Create IO mappings for the kernel task before we access GPIO registers
	// Disable caching and write buffer for this region
	_MMU_add_l1_va_to_pa_map(NULL, 
			(VADDR) ELFIN_GPIO_BASE, (PADDR) ELFIN_GPIO_BASE, 
			(UINT32) 0x100000, PRIVILEGED_RW_USER_NA, FALSE, FALSE);
#endif

	// Configure the LEDs
	configure_user_led(0);
	configure_user_led(1);
	configure_user_led(2);
	configure_user_led(3);
}

// The LED0-3 are connected to GPB5..8
void configure_user_led(UINT32 led)
{
	// Configure corresponding GPB bit as output
	rGPJ2CON &= ~(0xf << (led << 2));	// Clear the bits
	rGPJ2CON |= (0x1 << (led << 2)); 	// 01 for output pin	
}

void user_led_on(UINT32 led)
{
	rGPJ2DAT &= ~(1 << led);
}

void user_led_off(UINT32 led)
{
	rGPJ2DAT |= (1 << led);
}

void user_led_toggle(UINT32 led)
{
	rGPJ2DAT ^= (1 << led);
}
