///////////////////////////////////////////////////////////////////////////////
//
//						Copyright 2014 xxxxxxx, xxxxxxx
//	File:	G2D.h
//	Author: Bala B.
//	Description: 2D Graphics driver
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _G2D_H_
#define _G2D_H_

#include "os_api.h"

typedef UINT32 COLOR;
typedef INT32 Viewport_t;	// Typedef for viewport handle

typedef enum {
	// COLOR FORMAT
	G2D_COLOR_FMT_XRGB8888 = 0,
	G2D_COLOR_FMT_ARGB8888 = 1,
	G2D_COLOR_FMT_RGB565 = 2,
	G2D_COLOR_FMT_XRGB1555 = 3,
	G2D_COLOR_FMT_ARGB1555 = 4,
	G2D_COLOR_FMT_XRGB4444 = 5,
	G2D_COLOR_FMT_ARGB4444 = 6,
	G2D_COLOR_FMT_PRGB888 = 7,

} G2D_ColorFormat;

typedef struct
{
	UINT16 width;				// Width
	UINT16 height; 				// Height
	G2D_ColorFormat format;
	UINT8 * buffer;
} G2D_Image;

#define MAX_VIEWPORT_COUNT		(12)

#define DEFAULT_FG_COLOR	(0xAAAAAA)
#define DEFAULT_BG_COLOR	(0x220000)
#define MIN_VIEWPORT_WIDTH	(16)
#define MIN_VIEWPORT_HEIGHT	(16)
#define DEFAULT_TEXT_WIDTH	(8)				// Character width in pixels
#define DEFAULT_TEXT_HEIGHT	(16)			// Character height in pixels

Viewport_t g2d_create_viewport(	OS_Process_t owner,
								UINT16 x,
								UINT16 y,
								UINT16 w,
								UINT16 h,
								COLOR fg_color,
								COLOR bg_color);
								
void viewport_clear (Viewport_t handle);

void viewport_fill (Viewport_t handle, COLOR color);

void viewport_draw_image (Viewport_t handle, 
								G2D_Image * image, 
								UINT16 dst_x, 
								UINT16 dst_y, 
								UINT16 dst_w, 
								UINT16 dst_h);

void viewport_scroll_up(Viewport_t handle, UINT16 pixels);
void viewport_draw_string(Viewport_t handle, const INT8 * string);
void viewport_draw_char(Viewport_t handle, UINT8 ch);


#endif /* _G2D_H_ */
