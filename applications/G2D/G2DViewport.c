/*******************************************************************************
 *
 *						Copyright 2014 xxxxxxx, xxxxxxx
 *	File:	G2D.c
 *	Author: Bala B.
 *	Description: G2D Viewport functions
 *
 ******************************************************************************/

#include "G2D.h"
#include "G2DViewport.h"
#include "printf.h"

G2D_Viewport viewports [MAX_VIEWPORT_COUNT];
UINT32 viewport_alloc_mask = ~((1 << MAX_VIEWPORT_COUNT) - 1);

G2D_ColorDepth gColorDepthMap[] = {
	G2D_COLOR_DEPTH_XRGB8888,
	G2D_COLOR_DEPTH_ARGB8888,
	G2D_COLOR_DEPTH_RGB565,
	G2D_COLOR_DEPTH_XRGB1555,
	G2D_COLOR_DEPTH_ARGB1555,
	G2D_COLOR_DEPTH_XRGB4444,
	G2D_COLOR_DEPTH_ARGB4444,
	G2D_COLOR_DEPTH_PRGB888
};

Viewport_t g2d_create_viewport(	OS_Process_t owner,
								UINT16 x,
								UINT16 y,
								UINT16 w,
								UINT16 h,
								COLOR fg_color,
								COLOR bg_color)
{
	Viewport_t handle = INVALID;
	
	do
	{
		// Validate input parameters
		if((w < MIN_VIEWPORT_WIDTH) ||(h < MIN_VIEWPORT_HEIGHT)) {
			break;
		}
		
		handle = GetFreeResIndex(&viewport_alloc_mask, MAX_VIEWPORT_COUNT);
		if(handle < 0) {
			break;
		}
		
		// Set the resource status to busy
		SetResourceStatus(&viewport_alloc_mask, handle, FALSE);
	
		// Initialize the Viewport
		G2D_Viewport *vp = &viewports[handle];
		vp->x = x;
		vp->y = y;
		vp->w = w;
		vp->h = h;
		vp->text_x = 0;
		vp->text_y = 0;
		
		// Initialize default FG and BG colors
		vp->bg_color = bg_color;
		vp->fg_color = fg_color;
		
		vp->owner = owner;
		
	} while(0);
	
	return handle;
}

void g2d_set_dest_coordinates(G2D_Viewport *vp, UINT16 x, UINT16 y, UINT16 w, UINT16 h)
{
	ASSERT(vp);
	
	// Set Destination Left Top and Right Bottom Coordinate Registers
	const UINT32 lt_x = (vp->x + x) & 0x1FFF;
	const UINT32 lt_y = (vp->y + y) & 0x1FFF;
	const UINT32 rb_x = (lt_x + w) & 0x1FFF;
	const UINT32 rb_y = (lt_y + h) & 0x1FFF;
	REG_WR(DST_LEFT_TOP_REG, lt_x | (lt_y << 16));
	REG_WR(DST_RIGHT_BOTTOM_REG, rb_x | (rb_y << 16));
}

OS_Return g2d_activate_viewport(G2D_Viewport *vp)
{
	ASSERT(vp);
	
	/***** Set parameters for color for the viewport *****/
	
	// Foreground / Background color Registers
	REG_WR(FG_COLOR_REG, vp->fg_color);
	REG_WR(BG_COLOR_REG, vp->bg_color);
	REG_WR(BS_COLOR_REG, vp->bg_color);
	
	/***** Set Destination properties	*****/
	
	// Destination Image Selection Register 
	REG_WR(DST_SELECT_REG, G2D_SELECT_MODE_NORMAL);

	// Clipping registers
	const UINT32 lt_x = vp->x & 0x1FFF;
	const UINT32 lt_y = vp->y & 0x1FFF;
	const UINT32 rb_x = lt_x + (vp->w & 0x1FFF);
	const UINT32 rb_y = lt_y + (vp->h & 0x1FFF);
	REG_WR(CW_LT_REG, lt_x | (lt_y << 16));
	REG_WR(CW_RB_REG, rb_x | (rb_y << 16));

	return SUCCESS;
}

// Fill viewport with a color
void viewport_fill (Viewport_t handle, COLOR color)
{
	OS_Return res;
	
	do
	{
		// Validate the input handle
		if(!g2d_viewport_valid(handle)) break;
		
		// Get the viewport object
		G2D_Viewport * vp = &viewports[handle];
		
		// Wait for the previous command to finish
		while(g2d_isbusy());
		
		// Activate viewport
		res = g2d_activate_viewport(vp);
		if(res != SUCCESS) break;

		// Set Destination Left Top and Right Bottom Coordinate Registers
		g2d_set_dest_coordinates(vp, 0, 0, vp->w, vp->h);

		// Override the Foreground color Registers
		REG_WR(FG_COLOR_REG, color);

		// Source Image Selection Register 
		REG_WR(SRC_SELECT_REG, G2D_SELECT_MODE_FGCOLOR);

		// Source Image Color Mode Register
		REG_WR(SRC_COLOR_MODE_REG, G2D_COLOR_FMT_XRGB8888 | G2D_ORDER_AXRGB);

		// Set ROP4 register
		REG_WR(ROP4_REG, ROP4_COPY);
	
		// Start the BitBLT Command Register
		REG_WR(BITBLT_COMMAND_REG, 0);
		REG_WR(BITBLT_START_REG, 1);
				
	} while(0);		
}

// Clear viewport
void viewport_clear (Viewport_t handle)
{
	OS_Return res;
	
	do
	{
		// Validate the input handle
		if(!g2d_viewport_valid(handle)) break;
		
		// Get the viewport object
		G2D_Viewport * vp = &viewports[handle];
		
		// Wait for the previous command to finish
		while(g2d_isbusy());
		
		// Activate viewport
		res = g2d_activate_viewport(vp);
		if(res != SUCCESS) break;

		// Set Destination Left Top and Right Bottom Coordinate Registers
		g2d_set_dest_coordinates(vp, 0, 0, vp->w, vp->h);

		// Source Image Selection Register 
		REG_WR(SRC_SELECT_REG, G2D_SELECT_MODE_BGCOLOR);

		// Source Image Color Mode Register
		REG_WR(SRC_COLOR_MODE_REG, G2D_COLOR_FMT_XRGB8888 | G2D_ORDER_AXRGB);

		// Set ROP4 register
		REG_WR(ROP4_REG, 0x00);
	
		// Start the BitBLT Command Register
		REG_WR(BITBLT_COMMAND_REG, 0);
		REG_WR(BITBLT_START_REG, 1);
		
		// Clear rest of Viewport states
		vp->text_x = 0;
		vp->text_y = 0;
		
	} while(0);
}

void viewport_scroll_up(Viewport_t handle, UINT16 pixels)
{
	OS_Return res;	
	
	do
	{
		// Validate the input handle
		if(!g2d_viewport_valid(handle)) break;
		
		// Get the viewport object
		G2D_Viewport * vp = &viewports[handle];
		
		if(pixels >= vp->h) {
			viewport_clear(handle);
			break;
		}
		
		// Wait for the previous command to finish
		while(g2d_isbusy());
		
		// Activate viewport
		res = g2d_activate_viewport(vp);
		if(res != SUCCESS) break;

		// Set Destination Left Top and Right Bottom Coordinate Registers
		g2d_set_dest_coordinates(vp, 0, 0, vp->w, (vp->h - pixels));
		
		// Src buffer clear (Automatically set to 0b after a cycle)
		REG_WR(CACHECTL_REG, G2D_FLUSH_SRC_BUFFER);

		// Source properties
		REG_WR(SRC_SELECT_REG, G2D_SELECT_MODE_NORMAL);							// Source Image Selection Register 
		REG_WR(SRC_COLOR_MODE_REG, G2D_COLOR_FMT_XRGB8888 | G2D_ORDER_AXRGB);	// Source Image Color Mode Register
		REG_WR(SRC_STRIDE_REG, (fb_width * gColorDepthMap[G2D_COLOR_FMT_XRGB8888]) >> 3);	// Set Source Stride Register 
		REG_WR(SRC_BASE_ADDR_REG, fb_addr);
		REG_WR(SRC_LEFT_TOP_REG, vp->x | ((vp->y + pixels) << 16));
		REG_WR(SRC_RIGHT_BOTTOM_REG, (vp->x + vp->w) | ((vp->y + vp->h) << 16));
		
		// Set ROP4 register
		REG_WR(ROP4_REG, ROP4_COPY);
	
		// Start the BitBLT Command Register
		REG_WR(BITBLT_COMMAND_REG, 0);
		REG_WR(BITBLT_START_REG, 1);

		// Wait for the previous command to finish
		while(g2d_isbusy());
			
		/******** Now clear the remaining portion of the viewport *******/

		// Set Destination Left Top and Right Bottom Coordinate Registers
		g2d_set_dest_coordinates(vp, 0, vp->h - pixels, vp->w, pixels);

		// Source Image Selection Register 
		REG_WR(SRC_SELECT_REG, G2D_SELECT_MODE_BGCOLOR);

		// Set ROP4 register
		REG_WR(ROP4_REG, 0x00);
	
		// Start the BitBLT Command Register
		REG_WR(BITBLT_COMMAND_REG, 0);
		REG_WR(BITBLT_START_REG, 1);

	} while(0);	
}
