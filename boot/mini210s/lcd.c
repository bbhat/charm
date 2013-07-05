//-----------------------------------------------------------------------
// LCD related functions
//-----------------------------------------------------------------------
#define GPF0CON			(*(volatile unsigned long *)0xE0200120)
#define GPF1CON			(*(volatile unsigned long *)0xE0200140)
#define GPF2CON			(*(volatile unsigned long *)0xE0200160)
#define GPF3CON			(*(volatile unsigned long *)0xE0200180)

#define GPD0CON			(*(volatile unsigned long *)0xE02000A0)
#define GPD0DAT			(*(volatile unsigned long *)0xE02000A4)

#define CLK_SRC1		(*(volatile unsigned long *)0xe0100204)
#define CLK_DIV1		(*(volatile unsigned long *)0xe0100304)
#define DISPLAY_CONTROL	(*(volatile unsigned long *)0xe0107008)

#define VIDCON0			(*(volatile unsigned long *)0xF8000000)
#define VIDCON1			(*(volatile unsigned long *)0xF8000004)
#define VIDTCON2		(*(volatile unsigned long *)0xF8000018)
#define WINCON0 		(*(volatile unsigned long *)0xF8000020)
#define WINCON2 		(*(volatile unsigned long *)0xF8000028)
#define SHADOWCON 		(*(volatile unsigned long *)0xF8000034)
#define VIDOSD0A 		(*(volatile unsigned long *)0xF8000040)
#define VIDOSD0B 		(*(volatile unsigned long *)0xF8000044)
#define VIDOSD0C 		(*(volatile unsigned long *)0xF8000048)

#define VIDW00ADD0B0 	(*(volatile unsigned long *)0xF80000A0)
#define VIDW00ADD1B0 	(*(volatile unsigned long *)0xF80000D0)

#define VIDTCON0 (* (volatile unsigned long *) 0xF8000010)
#define VIDTCON1 (* (volatile unsigned long *) 0xF8000014)

#define HSPW (0)
#define HBPD (40 - 1)
#define HFPD (5 - 1)
#define VSPW (0)
#define VBPD (8 - 1)
#define VFPD (8 - 1)

// Frame Buffer Address
#define FB_ADDR (0x23000000)
#define ROW (272)
#define COL (480)
#define HOZVAL (COL-1)
#define LINEVAL (ROW-1)

#define LeftTopX 0
#define LeftTopY 0
#define RightBotX (COL-1)
#define RightBotY (ROW-1)

#define BACKLIGHT_ON	0

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
#if BACKLIGHT_ON==1
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
	VIDW00ADD0B0 = FB_ADDR;
	VIDW00ADD1B0 = (((HOZVAL + 1) * 4 + 0) * (LINEVAL + 1)) & (0xffffff);

	// Enable channel 0 transmit data
	SHADOWCON = 0x1;
}

// Clear the screen
void lcd_clear_screen (unsigned int color)
{
	unsigned long * ptr = (unsigned long *) FB_ADDR;
	unsigned int i;
		
	for (i = 0; i < (ROW * COL); i++) {
		ptr[i] = color;
	}
}
