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
static OS_Task_t periodic_task;
static UINT32 periodic_stack[0x400];
OS_Sem_t sem;
static volatile UINT32 * regs;		// Register map address
static G2D_Viewport viewports [MAX_VIEWPORT_COUNT];
static UINT32 viewport_alloc_mask = ~((1 << MAX_VIEWPORT_COUNT) - 1);
static Viewport_t scr_handle;
static UINT32 led = 0;

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

#define DEBUG	0

#ifdef DEBUG
#define dprintf(...) printf (__VA_ARGS__)
#else
#define dprintf(...)
#endif

#define isvalid(viewport)	((viewport < MAX_VIEWPORT_COUNT) && IsResourceBusy(&viewport_alloc_mask, viewport))

#define DEFAULT_FG_COLOR	0xAAAAAA
#define DEFAULT_BG_COLOR	0x220000

/*******************************************************************************
 * Local Functions
 *******************************************************************************/ 
static void g2d_init();
static inline BOOL g2d_isbusy();
static OS_Return g2d_activate_viewport(G2D_Viewport *vp);
static inline void g2d_set_dest_coordinates(UINT16 x, UINT16 y, UINT16 w, UINT16 h);
static void task_periodic(void * ptr);

void g2d_init()
{
	// Setting bit #0 results in a one-cycle reset signal to FIMG2D graphics engine.
	dprintf("Reset FIMG2D\n");
	REG_WR(SOFT_RESET_REG, 0x01);
		
	// Set the AXI ID Mode Register
	REG_WR(AXI_MODE_REG, 0x01);			// Use out of order AXI Master Read	
	
	// Set Destination Image Base Address Register
	ASSERT(fb_addr);	// Make sure the frame buffer is set
	REG_WR(DST_BASE_ADDR_REG, (UINT32) fb_addr);

	// Destination Image Color Mode Register
	REG_WR(DST_COLOR_MODE_REG, G2D_COLOR_FMT_XRGB8888 | G2D_ORDER_AXRGB);
	
	// Set Destination Stride Register 
	REG_WR(DST_STRIDE_REG, (fb_width * G2D_COLOR_DEPTH_XRGB8888) >> 3);
}

inline BOOL g2d_isbusy()
{
	return ((REG_RD(FIFO_STAT_REG) & 0x01) == 0);
}

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
		
		// Initialize default FG and BG colors
		vp->bg_color = bg_color;
		vp->fg_color = fg_color;
		
		vp->owner = owner;
		
	} while(0);
	
	return handle;
}

void g2d_set_dest_coordinates(UINT16 x, UINT16 y, UINT16 w, UINT16 h)
{
	// Set Destination Left Top and Right Bottom Coordinate Registers
	const UINT32 lt_x = x & 0x1FFF;
	const UINT32 lt_y = y & 0x1FFF;
	const UINT32 rb_x = lt_x + (w & 0x1FFF);
	const UINT32 rb_y = lt_y + (h & 0x1FFF);
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
		if(!isvalid(handle)) break;
		
		// Get the viewport object
		G2D_Viewport * vp = &viewports[handle];
		
		// Wait for the previous command to finish
		while(g2d_isbusy());
		
		// Activate viewport
		res = g2d_activate_viewport(vp);
		if(res != SUCCESS) break;

		// Set Destination Left Top and Right Bottom Coordinate Registers
		g2d_set_dest_coordinates(vp->x, vp->y, vp->w, vp->h);

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
		if(!isvalid(handle)) break;
		
		// Get the viewport object
		G2D_Viewport * vp = &viewports[handle];
		
		// Wait for the previous command to finish
		while(g2d_isbusy());
		
		// Activate viewport
		res = g2d_activate_viewport(vp);
		if(res != SUCCESS) break;

		// Set Destination Left Top and Right Bottom Coordinate Registers
		g2d_set_dest_coordinates(vp->x, vp->y, vp->w, vp->h);

		// Source Image Selection Register 
		REG_WR(SRC_SELECT_REG, G2D_SELECT_MODE_BGCOLOR);

		// Source Image Color Mode Register
		REG_WR(SRC_COLOR_MODE_REG, G2D_COLOR_FMT_XRGB8888 | G2D_ORDER_AXRGB);

		// Set ROP4 register
		REG_WR(ROP4_REG, ROP4_COPY);
	
		// Start the BitBLT Command Register
		REG_WR(BITBLT_COMMAND_REG, 0);
		REG_WR(BITBLT_START_REG, 1);
		
	} while(0);
}

void task_render(void * ptr)
{
	Viewport_t vp_handle = (Viewport_t) ptr;
	static BOOL toggle = FALSE;
	
	dprintf("Entered task_render for viewport %d Screen width %d height %d\n", vp_handle, fb_width, fb_height);
	
	do
	{
		if(!isvalid(vp_handle)) break;
		
		g2d_init();
		dprintf("Finished FIMG2D initialization\n");
		
		dprintf("Calling clear screen\n");
		viewport_clear(scr_handle);
		
		while(1)
		{
			dprintf("Calling viewport fill\n");
			viewport_fill(vp_handle, toggle ? DEFAULT_FG_COLOR : DEFAULT_BG_COLOR);
			toggle = !toggle;
			OS_SemWait(sem);			
		}		
	} while(0);	
}

void task_periodic(void * ptr)
{
	int led = *(int *) ptr;
	PFM_SetUserLED(led, LED_TOGGLE);
	OS_SemPost(sem);
}

int main(int argc, char *argv[])
{
	OS_Return ret;
	OS_Process_t process = OS_GetCurrentProcess();
	Viewport_t win_handle;
	
	do
	{
		// First map the G2D register space into this application
		regs = OS_MapPhysicalMemory(process, FIMG2D_REG_BASE, FIMG2D_REG_PAGE_SIZE, 
									MMAP_PROT_READ_WRITE | MMAP_NONCACHEABLE | MMAP_WRITE_BUFFER_DISABLE, 
									&ret);							
		if(!regs || ret != SUCCESS) break;
		
		fb_addr = OS_GetDisplayFrameBuffer(&fb_width, &fb_height, &fb_size);
		if(!fb_addr) break;
		
		// Create a whole screen viewport
		scr_handle = g2d_create_viewport(process, 
											0, 
											0, 
											fb_width, 
											fb_height, 
											DEFAULT_FG_COLOR, 
											DEFAULT_BG_COLOR);
		if(scr_handle < 0) break;
		
		// Create a sub-Window
		win_handle = g2d_create_viewport(process, 
											fb_width >> 2, 
											fb_height >> 2, 
											fb_width >> 1, 
											fb_height >> 1, 
											DEFAULT_FG_COLOR, 
											DEFAULT_BG_COLOR);
		if(win_handle < 0) break;

		OS_SemAlloc(&sem, 0, 0);

		ret = OS_CreatePeriodicTask(1000000, 1000000, 100000, 0, 
									periodic_stack, sizeof(periodic_stack), 
									"G2D Periodic", &periodic_task, task_periodic, &led);
		if(ret != SUCCESS) break;
		
		ret = OS_CreateAperiodicTask(TASK_PRIORITY_HIGH, render_stack, 
									sizeof(render_stack), "G2D Render Task", &render_task, 
									task_render, (void *) win_handle);
		if(ret != SUCCESS) break;
		
	} while(0);
		
	return ret;
}
