///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	target.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Board specific functions source file
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_config.h"
#include "target.h"
#include "soc.h"
#include "rtc.h"
#include "serial.h"

extern UINT32 __frame_buffer_start__;
extern UINT32 __frame_buffer_length__;
void * FB_ADDR = (void *)&__frame_buffer_start__;
const UINT32 FB_SIZE = (UINT32)&__frame_buffer_length__;

void _OS_TargetInit(void)
{
	// Configure the LEDs
	configure_user_led(0);
	configure_user_led(1);
	configure_user_led(2);
	configure_user_led(3);
	
#if TARGET_HAS_LCD && ENABLE_LCD
	lcd_init();
#endif

#if SERIAL_DRIVER_ENABLE	
	// Initialize the RTC driver
	_OS_DriverInit((OS_Driver *) &g_serial_driver, "Serial", _Serial_DriverInit, 16);
#endif

#if ENABLE_RTC	
	// Initialize the RTC driver
	_OS_DriverInit((OS_Driver *) &g_rtc_driver, "RTC Driver", _RTC_DriverInit, 0);	
#endif
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

#if ENABLE_LCD

// Initialize the LCD
void lcd_init (void)
{
	// Function configuration pin for LCD
	GPF0CON = 0x22222222;
	GPF1CON = 0x22222222;
	GPF2CON = 0x22222222;
	GPF3CON = 0x22222222;

	// Turn on/off backlight
	GPD0CON &= ~(0xf << 4);
	GPD0CON |= (1 << 4);
#if BACKLIGHT_ON
	GPD0DAT |= (1 << 1);
#else
	GPD0DAT &= ~(1 << 1);
#endif

	// 10: RGB = FIMD I80 = FIMD ITU = FIMD
	DISPLAY_CONTROL = 2 << 0;

	// Bit [26 ~ 28]: using the RGB interface
	// Bit [18]: RGB parallel
	// Bit [2]: Select the clock source is HCLK_DSYS = 166MHz
	VIDCON0 &= ~((3 << 26) | (1 << 18) | (1 << 2));

	// Bit [1]: Enable lcd controller
	// Bit [0]: after the end of the current frame lcd controller enable
	VIDCON0 |= ((1 << 0) | (1 << 1));

	// Bit [6]: Select the desired Divide
	// Bit [6 ~ 13]: division factor of 15, that VCLK = 166M / (14 +1) = 11M
	VIDCON0 |= 14 << 6 | 1 << 4;


	// H43-HSD043I9W1.pdf (p13) Timing Diagram: VSYNC and HSYNC is low pulse
	// S5pv210 chip manual (p1207) Timing Diagram: VSYNC and HSYNC are active high pulse, so the need to reverse
	VIDCON1 |= (1 << 5) | (1 << 6);

	// Set Timing
	VIDTCON0 = VBPD << 16 | VFPD << 8 | VSPW << 0;
	VIDTCON1 = HBPD << 16 | HFPD << 8 | HSPW << 0;

	// Set the width
	VIDTCON2 = (LINEVAL << 11) | (HOZVAL << 0);

	// Set windows1
	// Bit [0]: Enable
	// Bit [2 ~ 5]: 24bpp
	WINCON0 |= 1 << 0;
	WINCON0 &= ~ (0xf << 2);
	WINCON0 |= (0xB << 2) | (1 << 15);

	// Set up and down about windows1
	VIDOSD0A = (LeftTopX << 11) | (LeftTopY << 0);
	VIDOSD0B = (RightBotX << 11) | (RightBotY << 0);
	VIDOSD0C = (LINEVAL + 1) * (HOZVAL + 1);


	// Set the address of fb
	VIDW00ADD0B0 = (UINT32) FB_ADDR;
	VIDW00ADD1B0 = (((HOZVAL + 1) * 4 + 0) * (LINEVAL + 1)) & (0xffffff);

	// Enable channel 0 transmit data
	SHADOWCON = 0x1;
}

#endif
