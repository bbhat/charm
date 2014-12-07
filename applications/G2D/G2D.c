/*******************************************************************************
 *
 *						Copyright 2014 xxxxxxx, xxxxxxx
 *	File:	G2D.c
 *	Author: Bala B.
 *	Description: G2D User space driver
 *
 ******************************************************************************/

#include "G2D.h"
#include "printf.h"
#include "bitmaps.h"

/*
 * Global Variables
 */
static OS_Task_t render_task;
static UINT32 render_stack[0x800];
static OS_Task_t periodic_task;
static UINT32 periodic_stack[0x400];
OS_Sem_t sem;
volatile UINT32 * g2d_regs;		// Register map address
static Viewport_t scr_handle;
static UINT32 led = 0;

/*
 * Frame buffer
 */
static void * fb_addr;
static UINT32 fb_width;
static UINT32 fb_height;
static UINT32 fb_size;

#define DEBUG_PRINT	0

#ifdef DEBUG_PRINT
#define dprintf(...) printf (__VA_ARGS__)
#else
#define dprintf(...)
#endif


/*******************************************************************************
 * Local Functions
 *******************************************************************************/ 
static void g2d_init();
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
	REG_WR(DST_STRIDE_REG, (fb_width * gColorDepthMap[G2D_COLOR_FMT_XRGB8888]) >> 3);
}

BOOL g2d_isbusy()		
{ 
	return ((REG_RD(FIFO_STAT_REG) & 0x01) == 0);	
}

BOOL isvalid(viewport)	
{ 
	return ((viewport < MAX_VIEWPORT_COUNT) && IsResourceBusy(&viewport_alloc_mask, viewport));	
}

void task_render(void * ptr)
{
	Viewport_t vp_handle = (Viewport_t) ptr;
	G2D_Image image;
	UINT32 half_w;
	UINT32 half_h;
	UINT32 w = 2;
	UINT32 h = 2;
	float step_x;
	float step_y;
	UINT32 count = 0;
	
	dprintf("Entered task_render for viewport %d Screen width %d height %d\n", vp_handle, fb_width, fb_height);
	
	do
	{
		if(!isvalid(vp_handle)) break;
		
		// Get the viewport object
		G2D_Viewport * vp = &viewports[vp_handle];
		
		g2d_init();
		dprintf("Finished FIMG2D initialization\n");
		
		dprintf("Calling clear screen\n");
		viewport_clear(scr_handle);
		
		// Prepare the viewport image
		image.width = 480;
		image.height = 270;
		image.format = G2D_COLOR_FMT_PRGB888;
		image.buffer = (UINT8 *)&gImage_bmp;
		
		half_w = vp->w >> 1;
		half_h = vp->h >> 1;
		
		step_x = half_w / 50.0;
		step_y = half_h / 50.0;

		while(1)
		{
			OS_SemWait(sem);			
			viewport_draw_image(vp_handle, &image, half_w - (w >> 1), half_h - (h >> 1), w, h);
			count++;
			w = count * step_x;
			h = count * step_y;
			if((w >= vp->w) || (h >= vp->h))
			{
				int sleep = 80;
				while(sleep--) 
					OS_SemWait(sem);			
				viewport_fill(vp_handle, DEFAULT_BG_COLOR);
				w = 2;
				h = 2;	
				count = 0;
			}
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
		g2d_regs = OS_MapPhysicalMemory(process, FIMG2D_REG_BASE, FIMG2D_REG_PAGE_SIZE, 
									MMAP_PROT_READ_WRITE | MMAP_NONCACHEABLE | MMAP_WRITE_BUFFER_DISABLE, 
									&ret);							
		if(!g2d_regs || ret != SUCCESS) break;
		
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
											0, 
											0, 
											fb_width, 
											fb_height, 
											DEFAULT_FG_COLOR, 
											DEFAULT_BG_COLOR);
		if(win_handle < 0) break;

		OS_SemAlloc(&sem, 0, 0);

		ret = OS_CreatePeriodicTask(25000, 25000, 2000, 0, 
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
