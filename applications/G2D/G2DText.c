/*******************************************************************************
 *
 *						Copyright 2014 xxxxxxx, xxxxxxx
 *	File:	G2D.c
 *	Author: Bala B.
 *	Description: G2D Text Functions
 *
 ******************************************************************************/

#include "G2D.h"
#include "G2DViewport.h"
#include "printf.h"
#include "font_8x16.h"

void viewport_draw_char(Viewport_t handle, UINT8 ch)
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
		g2d_set_dest_coordinates(vp, vp->text_x, vp->text_y, DEFAULT_TEXT_WIDTH, DEFAULT_TEXT_HEIGHT);
		
		// Source properties
		REG_WR(SRC_SELECT_REG, G2D_SELECT_MODE_FGCOLOR);								// Source Image Selection Register 
		REG_WR(SRC_COLOR_MODE_REG, G2D_COLOR_FMT_XRGB8888 | G2D_ORDER_AXRGB);			// Source Image Color Mode Register

		// Mask buffer clear (Automatically set to 0b after a cycle)
		REG_WR(CACHECTL_REG, G2D_FLUSH_MASK_BUFFER);
		
		// Select Mask Image
		REG_WR(MASK_BASE_ADDR_REG, &fontdata_8x16[((UINT16)ch) << 4]);
		REG_WR(MASK_STRIDE_REG, (DEFAULT_TEXT_WIDTH >> 3));								// 1 bit per pixel, 8 pixels per char
				
		// Set ROP4 register
		REG_WR(ROP4_REG, ROP4_MASKED_COPY);
	
		// Start the BitBLT Command Register
		REG_WR(BITBLT_COMMAND_REG, G2D_ENABLE_MASK_OP);
		REG_WR(BITBLT_START_REG, 1);

		// Advance the cursor
		vp->text_x += DEFAULT_TEXT_WIDTH;
		if((vp->text_x + DEFAULT_TEXT_WIDTH) > vp->w)
		{
			vp->text_x = 0;
			vp->text_y += DEFAULT_TEXT_HEIGHT;
			if((vp->text_y + DEFAULT_TEXT_HEIGHT) > vp->h)
			{
				viewport_scroll_up(handle, DEFAULT_TEXT_HEIGHT);
				vp->text_y -= DEFAULT_TEXT_HEIGHT;
			}
		}
		
	} while(0);	
}

void viewport_draw_string(Viewport_t handle, const INT8 * string)
{
}
