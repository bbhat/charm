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
volatile UINT32 * regs;

void task_render(void * ptr)
{
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
									MMAP_PROT_READ_WRITE | MMAP_NONCACHEABLE, &ret);
		if(!regs || ret != SUCCESS) break;
		
		ret = OS_CreateAperiodicTask(TASK_PRIORITY_HIGH, render_stack, 
									sizeof(render_stack), "G2D Render Task", &render_task, 
									task_render, NULL);
		if(ret != SUCCESS) break;
		
	} while(0);
		
	return ret;
}
