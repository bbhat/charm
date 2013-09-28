///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	os_init.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: OS Initialization routines
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_core.h"
#include "os_queue.h"
#include "os_process.h"
#include "target.h"
#include "cache.h"
#include "uart.h"

// External functions used in here
extern void _OS_InitTimer();
extern void _OS_PlatformInit();
extern OS_Error ramdisk_init(void * addr);
extern _OS_Queue g_ready_q;
extern _OS_Queue g_wait_q;
extern _OS_Queue g_ap_ready_q;
extern _OS_Queue g_block_q;

extern UINT32 __ramdisk_start__;

OS_ProcessCB	* g_kernel_process;	// Kernel process

void _OS_Init(void);
void _OS_Exit(void);
static void _OS_InitFreeResources(void);
void kernel_process_entry(void * pdata);

///////////////////////////////////////////////////////////////////////////////
// Initialization function for the OS
///////////////////////////////////////////////////////////////////////////////
void _OS_Init()
{
	OS_Process kernel_pcb;
	
	// Call system initialization routine
	_OS_PlatformInit();
	
	// Target Initialization
	_OS_TargetInit();
	
	// Initialize free resource pools
	_OS_InitFreeResources();
	
	// Initialize debug UART
	Uart_Init(UART0);	
		
#if ENABLE_RAMDISK==1	
	if(ramdisk_init((void *)&__ramdisk_start__) != SUCCESS) {
		panic("ramdisk_init failed\n");
	}
#endif
	
	// Initialize Instruction and Data Caches
#if ENABLE_INSTRUCTION_CACHE == 1
	_OS_InvalidateICache();
	_OS_EnableICache();
#endif

#if ENABLE_DATA_CACHE == 1
	_OS_InvalidateDCache();
	_OS_EnableDCache();
#endif

#if ENABLE_L2_CACHE == 1
	_OS_Enable_L2_Cache();
#endif

	// Start the scheduling timer
	_OS_InitTimer();

	// Initialize the global queue for timer waiting
	_OS_QueueInit(&g_ready_q); 
	_OS_QueueInit(&g_wait_q);
	_OS_QueueInit(&g_ap_ready_q);
	_OS_QueueInit(&g_block_q);
	
    KlogStr(KLOG_OS_STARTUP, "Creating process - ", "kernel");
	
	// Initialize the Kernel process
	OS_CreateProcess(&kernel_pcb, "kernel", &kernel_process_entry, NULL);	
	g_kernel_process = &g_process_pool[kernel_pcb];

    KlogStr(KLOG_OS_STARTUP, "Calling - ", "OS_Start");
	
	// This will start the scheuling
	OS_Start();
	
	// The main function should never return if it called OS_Start. 
	// If it does not call we may return from main.
	_OS_Exit();
}

///////////////////////////////////////////////////////////////////////////////
// This function terminates the OS and shuts down the system
///////////////////////////////////////////////////////////////////////////////
void _OS_Exit()
{
	// TODO: Stop scheduling & shutdown the system
	panic("Shutdown");
}

///////////////////////////////////////////////////////////////////////////////
// Initialization of Interrupts processing
///////////////////////////////////////////////////////////////////////////////
void _OS_InitInterrupts()
{
	UINT32 intsts = 0;
	
	// Enable IRQ and FIQ interrupts
	_enable_interrupt(intsts);
}

///////////////////////////////////////////////////////////////////////////////
// Initialization of free resource pools
///////////////////////////////////////////////////////////////////////////////
static void _OS_InitFreeResources(void)
{
	// The resource usage masks are already cleared when BSS section is initialized
	// Only mark the unused bits as busy
	g_task_usage_mask[((MAX_TASK_COUNT + 31) >> 5) - 1] 
		|= ~((1 << (32 - (MAX_TASK_COUNT & 0x1f))) - 1);	
	g_process_usage_mask[((MAX_PROCESS_COUNT + 31) >> 5) - 1] 
		|= ~((1 << (32 - (MAX_PROCESS_COUNT & 0x1f))) - 1);	
	g_rdfile_usage_mask[((MAX_OPEN_FILES + 31) >> 5) - 1] 
		|= ~((1 << (32 - (MAX_OPEN_FILES & 0x1f))) - 1);			
	
}
