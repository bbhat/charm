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

typedef struct
{
	UINT16 x;				// Starting coordinate-X
	UINT16 y;				// Starting coordinate-Y
	UINT16 w;				// Width
	UINT16 h; 				// Height
	
	UINT32 fg_color;
	UINT32 bg_color;
	
	OS_Process_t owner;		// Process handle owning this viewport
	
} G2D_Viewport;

typedef INT32 Viewport_t;	// Typedef for viewport handle

#define MAX_VIEWPORT_COUNT		(12)

void g2d_init();
Viewport_t g2d_create_viewport(	OS_Process_t owner,
								UINT16 x,
								UINT16 y,
								UINT16 w,
								UINT16 h);
								


#endif /* _G2D_H_ */
