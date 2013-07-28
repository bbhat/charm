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

void _OS_TargetInit(void)
{
	// Configure the LEDs
	configure_user_led(USER_LED0);
	configure_user_led(USER_LED1);
	configure_user_led(USER_LED2);
	configure_user_led(USER_LED3);	
}

// The LED0-3 are connected to GPB5..8
void configure_user_led(UINT32 led)
{
	// Configure corresponding GPB bit as output
	rGPBCON &= ~(0x3 << ((5 + led) << 1));	// Clear the bits
	rGPBCON |= (0x1 << ((5 + led) << 1)); 	// 01 for output pin
	
	// Disable Pullups
	rGPBUP |= (1 << (5 +led));
}

void user_led_on(UINT32 led)
{
	rGPBDAT |= (1 << (5 +led));
}

void user_led_off(UINT32 led)
{
	rGPBDAT &= ~(1 << (5 +led));	
}

void user_led_toggle(UINT32 led)
{
	rGPBDAT ^= (1 << (5 +led));		
}
