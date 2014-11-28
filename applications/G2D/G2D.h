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
#include "fimg2d_regs.h"

typedef UINT32 COLOR;

typedef struct
{
	UINT16 x;				// Starting coordinate-X
	UINT16 y;				// Starting coordinate-Y
	UINT16 w;				// Width
	UINT16 h; 				// Height
	
	COLOR fg_color;
	COLOR bg_color;
	
	OS_Process_t owner;		// Process handle owning this viewport
	
} G2D_Viewport;

enum {
	G2D_SELECT_MODE_NORMAL	= (0 << 0),
	G2D_SELECT_MODE_FGCOLOR = (1 << 0),
	G2D_SELECT_MODE_BGCOLOR = (2 << 0)
};

enum {
	// COLOR FORMAT
	G2D_COLOR_FMT_XRGB8888 = 0,
	G2D_COLOR_FMT_ARGB8888 = 1,
	G2D_COLOR_FMT_RGB565 = 2,
	G2D_COLOR_FMT_XRGB1555 = 3,
	G2D_COLOR_FMT_ARGB1555 = 4,
	G2D_COLOR_FMT_XRGB4444 = 5,
	G2D_COLOR_FMT_ARGB4444 = 6,
	G2D_COLOR_FMT_PRGB888 = 7,

	// Depth in terms of number of bits per pixel
	G2D_COLOR_DEPTH_XRGB8888 = 32,
	G2D_COLOR_DEPTH_ARGB8888 = 32,
	G2D_COLOR_DEPTH_RGB565 = 16,
	G2D_COLOR_DEPTH_XRGB1555 = 16,
	G2D_COLOR_DEPTH_ARGB1555 = 16,
	G2D_COLOR_DEPTH_XRGB4444 = 16,
	G2D_COLOR_DEPTH_ARGB4444 = 16,
	G2D_COLOR_DEPTH_PRGB888 = 24,

	// COLOR ORDER
	G2D_ORDER_AXRGB		= (0 << 4),
	G2D_ORDER_RGBAX		= (1 << 4),
	G2D_ORDER_AXBGR		= (2 << 4),
	G2D_ORDER_BGRAX		= (3 << 4)
};

enum
{
	G2D_ENABLE_MASK_OP = (1 << 0),
	G2D_ENABLE_STRETCH_MODE = (1 << 4),
	G2D_ENABLE_CLIPPING_WINDOW = (1 << 8),
	
	// Transparency mode
	G2D_TRANSPARENT_MODE_OPAQUE = (0 << 12),
	G2D_TRANSPARENT_MODE_TRANSPARENT = (1 << 12),
	G2D_TRANSPARENT_MODE_BLUESCREEN = (2 << 12),
	G2D_TRANSPARENT_MODE_MAX = (3 << 12),
	
	// Color Key mode
	G2D_COLORKEY_MODE_DISABLE	= (0 << 16),
	G2D_COLORKEY_MODE_SRC_RGBA	= (1 << 16),
	G2D_COLORKEY_MODE_DST_RGBA	= (2 << 16),
	G2D_COLORKEY_MODE_MASK		= (3 << 16),
	
	// Alpha blend_modes
	G2D_ALPHA_BLEND_MODE_DISABLE = (0 << 20),
	G2D_ALPHA_BLEND_MODE_ENABLE = (1 << 20),
	G2D_ALPHA_BLEND_MODE_FADING = (2 << 20),
	G2D_ALPHA_BLEND_MODE_MAX = (3 << 20),
	
	// Source SrcNonPreBlendMode(Non-premultiplied)
	G2D_ALPHA_BLEND_CONST_ALPHA = (1 << 22),
	G2D_ALPHA_BLEND_PERPIXEL_ALPHA = (2 << 22)
};

enum {
	// ROP4 operation values
	ROP4_COPY = 0xCCCC,
	ROP4_INVERT = 0x3333
};

typedef INT32 Viewport_t;	// Typedef for viewport handle

#define MAX_VIEWPORT_COUNT		(12)

Viewport_t g2d_create_viewport(	OS_Process_t owner,
								UINT16 x,
								UINT16 y,
								UINT16 w,
								UINT16 h,
								COLOR fg_color,
								COLOR bg_color);
								
void viewport_clear (Viewport_t handle);
void viewport_fill (Viewport_t handle, COLOR color);

#endif /* _G2D_H_ */
