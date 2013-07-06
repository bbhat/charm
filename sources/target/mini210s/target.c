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

// The LED0-3 are connected to GPB5..8
void configure_user_led(User_led_type led)
{
	// Configure corresponding GPB bit as output
	rGPJ2CON &= ~(0xf << (led << 2));	// Clear the bits
	rGPJ2CON |= (0x1 << (led << 2)); 	// 01 for output pin	
}

void user_led_on(User_led_type led)
{
	rGPJ2DAT &= ~(1 << led);
}

void user_led_off(User_led_type led)
{
	rGPJ2DAT |= (1 << led);
}

void user_led_toggle(User_led_type led)
{
	rGPJ2DAT ^= (1 << led);
}
