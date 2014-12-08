/*******************************************************************************
 *
 *						Copyright 2014 xxxxxxx, xxxxxxx
 *	File:	G2DImage.c
 *	Author: Bala B.
 *	Description: G2D Image Functions
 *
 ******************************************************************************/

#include "G2D.h"
#include "G2DViewport.h"
#include "printf.h"

void viewport_draw_image (Viewport_t handle, 
					G2D_Image * image, 
					UINT16 dst_x, 
					UINT16 dst_y, 
					UINT16 dst_w, 
					UINT16 dst_h)
{
	OS_Return res;	
	
	ASSERT(image);
	do
	{
		// Validate the input handle
		if(!g2d_viewport_valid(handle)) break;
		
		// The width and height should not be 0
		if(!dst_w || !dst_h) break;
		
		// The width and height of the image should not be 0
		if(!image->width || !image->height) break;
		
		// Get the viewport object
		G2D_Viewport * vp = &viewports[handle];
		
		// Wait for the previous command to finish
		while(g2d_isbusy());
		
		// Activate viewport
		res = g2d_activate_viewport(vp);
		if(res != SUCCESS) break;

		// Set Destination Left Top and Right Bottom Coordinate Registers
		g2d_set_dest_coordinates(vp, dst_x, dst_y, dst_w, dst_h);
		
		// Check if we need to stretch the image
		BOOL stretch = ((image->width != dst_w) || (image->height != dst_h));
		
		// Src buffer clear (Automatically set to 0b after a cycle)
		REG_WR(CACHECTL_REG, G2D_FLUSH_SRC_BUFFER);

		// Source properties
		REG_WR(SRC_SELECT_REG, G2D_SELECT_MODE_NORMAL);					// Source Image Selection Register 
		REG_WR(SRC_COLOR_MODE_REG, image->format | G2D_ORDER_AXRGB);	// Source Image Color Mode Register
		REG_WR(SRC_STRIDE_REG, (image->width * gColorDepthMap[image->format]) >> 3);	// Set Source Stride Register 
		REG_WR(SRC_BASE_ADDR_REG, (UINT32)image->buffer);
		REG_WR(SRC_LEFT_TOP_REG, 0);
		REG_WR(SRC_RIGHT_BOTTOM_REG, image->width | (image->height << 16));
		
		// Set ROP4 register
		REG_WR(ROP4_REG, ROP4_COPY);
	
		// Start the BitBLT Command Register
		REG_WR(BITBLT_COMMAND_REG, G2D_ENABLE_CLIPPING_WINDOW | (stretch ? G2D_ENABLE_STRETCH_MODE : 0));
		REG_WR(BITBLT_START_REG, 1);

	} while(0);	
}
