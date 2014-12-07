///////////////////////////////////////////////////////////////////////////////
//
//						Copyright 2014 xxxxxxx, xxxxxxx
//	File:	G2DViewport.h
//	Author: Bala B.
//	Description: 2D viewport header file
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _G2DVIEWPORT_H_
#define _G2DVIEWPORT_H_

#include "fimg2d_regs.h"
#include "utils.h"

enum {
	G2D_SELECT_MODE_NORMAL	= (0 << 0),
	G2D_SELECT_MODE_FGCOLOR = (1 << 0),
	G2D_SELECT_MODE_BGCOLOR = (2 << 0)
};

typedef enum {
	// Depth in terms of number of bits per pixel
	G2D_COLOR_DEPTH_XRGB8888 = 32,
	G2D_COLOR_DEPTH_ARGB8888 = 32,
	G2D_COLOR_DEPTH_RGB565 = 16,
	G2D_COLOR_DEPTH_XRGB1555 = 16,
	G2D_COLOR_DEPTH_ARGB1555 = 16,
	G2D_COLOR_DEPTH_XRGB4444 = 16,
	G2D_COLOR_DEPTH_ARGB4444 = 16,
	G2D_COLOR_DEPTH_PRGB888 = 24
	
} G2D_ColorDepth;

typedef enum
{
	// COLOR ORDER
	G2D_ORDER_AXRGB		= (0 << 4),
	G2D_ORDER_RGBAX		= (1 << 4),
	G2D_ORDER_AXBGR		= (2 << 4),
	G2D_ORDER_BGRAX		= (3 << 4)
	
} G2D_ColorOrder;

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

typedef struct
{
	UINT16 x;				// Starting coordinate-X
	UINT16 y;				// Starting coordinate-Y
	UINT16 w;				// Width
	UINT16 h; 				// Height
	
	COLOR fg_color;
	COLOR bg_color;
	
	// When used to output text, this points to the current cursor location 
	// within the current viewport
	UINT16 text_x;			
	UINT16 text_y;
	
	OS_Process_t owner;		// Process handle owning this viewport
	
} G2D_Viewport;

#define REG_RD(reg)			(g2d_regs[(reg) >> 2])
#define REG_WR(reg, val)	(g2d_regs[(reg) >> 2] = (val))

extern volatile UINT32 * g2d_regs;		// Register map address
extern G2D_ColorDepth gColorDepthMap[];
extern UINT32 viewport_alloc_mask;
extern G2D_Viewport viewports [MAX_VIEWPORT_COUNT];

// Internal functions
BOOL g2d_isbusy();
BOOL isvalid(viewport);
OS_Return g2d_activate_viewport(G2D_Viewport *vp);
void g2d_set_dest_coordinates(G2D_Viewport *vp, UINT16 x, UINT16 y, UINT16 w, UINT16 h);

#endif /* _G2DVIEWPORT_H_ */
