/*******************************************************************************
 *
 *						Copyright 2014 xxxxxxx, xxxxxxx
 *	File:	G2D.c
 *	Author: Bala B.
 *	Description: OS Statistics related functions
 *
 ******************************************************************************/

#include "G2D.h"
#include "printf.h"
#include "utils.h"

/*
 * Global Variables
 */
static OS_Task_t render_task;
static UINT32 render_stack[0x800];
static volatile UINT32 * regs;		// Register map address
static G2D_Viewport viewports [MAX_VIEWPORT_COUNT];
static UINT32 viewport_alloc_mask = ~((1 << MAX_VIEWPORT_COUNT) - 1);

/*
 * Frame buffer
 */
static void * fb_addr;
static UINT32 fb_width;
static UINT32 fb_height;
static UINT32 fb_size;

/*
 * Global Macros
 */
#define REG_RD(reg)			(regs[(reg) >> 2])
#define REG_WR(reg, val)	(regs[(reg) >> 2] = (val))

#define DEBUG	1

#ifdef DEBUG
#define dprintf(...) printf (__VA_ARGS__)
#else
#define dprintf(...)
#endif

#define isvalid(viewport)	((viewport < MAX_VIEWPORT_COUNT) && IsResourceBusy(&viewport_alloc_mask, viewport))

void g2d_init()
{
	// Setting bit #0 results in a one-cycle reset signal to FIMG2D graphics engine.
	dprintf("Reset FIMG2D\n");
	REG_WR(SOFT_RESET_REG, 0x01);
}

Viewport_t g2d_create_viewport(	OS_Process_t owner,
								UINT16 x,
								UINT16 y,
								UINT16 w,
								UINT16 h)
{
	Viewport_t handle = INVALID;
	
	do
	{
		handle = GetFreeResIndex(&viewport_alloc_mask, MAX_VIEWPORT_COUNT);
		if(handle) {
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
		
		// Initialize default FG and BG colors
		vp->bg_color = 0x4321;
		vp->fg_color = 0x1234;
		
		vp->owner = owner;
		
	} while(0);
	
	return handle;
}

// Clear viewport
void viewport_clear (Viewport_t handle)
{
	unsigned long * ptr = (unsigned long *) fb_addr;
	unsigned int i;
	
	do
	{
		if(!isvalid(handle)) break;
		
		G2D_Viewport * vp = &viewports[handle];
		for (i = 0; i < (vp->w * vp->h); i++) {
			ptr[i] = vp->bg_color;
		}		
		
	} while(0);		
}

void task_render(void * ptr)
{
	Viewport_t vp_handle = (Viewport_t) ptr;
	
	do
	{
		if(!isvalid(vp_handle)) break;
		
		dprintf("Calling viewport clear\n");
		
		// Start with a clear viewport
		viewport_clear(vp_handle);
		
		g2d_init();
		dprintf("Finished FIMG2D initialization\n");
		
	} while(0);	
}

int main(int argc, char *argv[])
{
	OS_Return ret;
	OS_Process_t process = OS_GetCurrentProcess();
	Viewport_t vp_handle;
	
	do
	{
		// First map the G2D register space into this application
		regs = OS_MapPhysicalMemory(process, FIMG2D_REG_BASE, FIMG2D_REG_PAGE_SIZE, 
									MMAP_PROT_READ_WRITE | MMAP_NONCACHEABLE | MMAP_WRITE_BUFFER_DISABLE, 
									&ret);							
		if(!regs || ret != SUCCESS) break;
		
		fb_addr = OS_GetDisplayFrameBuffer(&fb_width, &fb_height, &fb_size);
		if(!fb_addr) break;
		
		vp_handle = g2d_create_viewport(process, 0, 0, fb_width, fb_height);
		if(vp_handle < 0) break;

		ret = OS_CreateAperiodicTask(TASK_PRIORITY_HIGH, render_stack, 
									sizeof(render_stack), "G2D Render Task", &render_task, 
									task_render, (void *) vp_handle);
		if(ret != SUCCESS) break;
		
	} while(0);
		
	return ret;
}