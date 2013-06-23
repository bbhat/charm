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

typedef enum {
	USER_LED0 = 0,
	USER_LED1,
	USER_LED2,
	USER_LED3
	} User_led_type;

void configure_user_led(User_led_type led);
void user_led_on(User_led_type led);
void user_led_off(User_led_type led);
void user_led_toggle(User_led_type led);

#endif // _TARGET_H_
