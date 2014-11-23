///////////////////////////////////////////////////////////////////////////////
//
//						Copyright 2014 xxxxxxx, xxxxxxx
//	File:	G2D.c
//	Author: Bala B.
//	Description: OS Statistics related functions
//
///////////////////////////////////////////////////////////////////////////////

#include "G2D.h"
#include "printf.h"

OS_Task_t render_task;
UINT32 render_stack[0x800];

// Register map address
static volatile UINT32 * regs;

#define REG_RD(reg)			(regs[(reg) >> 2])
#define REG_WR(reg, val)	(regs[(reg) >> 2] = (val))

#define DEBUG	1

#ifdef DEBUG
#define dprintf(...) printf (__VA_ARGS__)
#else
#define dprintf(...)
#endif

void g2d_reset()
{
	// Setting bit #0 results in a one-cycle reset signal to FIMG2D graphics engine.
	dprintf("Reset FIMG2D\n");
	REG_WR(SOFT_RESET_REG, 0x01);
}

void task_render(void * ptr)
{
	// Start with a reset 
	g2d_reset();
	dprintf("Finished FIMG2D reset\n");
	
	while(1)
	{	
		OS_TaskYield();
	}
}

int main(int argc, char *argv[])
{
	OS_Return ret;
	OS_Process_t process = OS_GetCurrentProcess();
	
	do
	{
		// First map the G2D register space into this application
		regs = OS_MapPhysicalMemory(process, FIMG2D_REG_BASE, FIMG2D_REG_PAGE_SIZE, 
									MMAP_PROT_READ_WRITE | MMAP_NONCACHEABLE | MMAP_WRITE_BUFFER_DISABLE, 
									&ret);							
		if(!regs || ret != SUCCESS) break;
				
		ret = OS_CreateAperiodicTask(TASK_PRIORITY_HIGH, render_stack, 
									sizeof(render_stack), "G2D Render Task", &render_task, 
									task_render, NULL);
		if(ret != SUCCESS) break;
		
	} while(0);
		
	return ret;
}
