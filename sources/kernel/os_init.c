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
#include "soc.h"

// External functions used in here
extern void _OS_InitTimer();
extern void _OS_PlatformInit();
extern OS_Error ramdisk_init(void * addr);
extern _OS_Queue g_ready_q;
extern _OS_Queue g_wait_q;
extern _OS_Queue g_ap_ready_q;
extern _OS_Queue g_block_q;

// Following variable are derived from the linker script file.
// They are used to create memory maps for the kernel process
extern UINT32 __ramdisk_start__;
extern UINT32 __ramdisk_length__;
extern UINT32 __EX_system_area_start__;
extern UINT32 __EX_system_area_end__;
extern UINT32 __RO_system_area_start__;
extern UINT32 __RO_system_area_end__;
extern UINT32 __RW_system_area_start__;
extern UINT32 __RW_system_area_end__;
extern UINT32 __page_table_area_start__;
extern UINT32 __page_table_area_length__;

OS_ProcessCB	* g_kernel_process;	// Kernel process

void _OS_Init(void);
void _OS_Start(void);
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
	
	// Initialize free resource pools
	_OS_InitFreeResources();
	
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

	// Initialize the global queue for timer waiting
	_OS_QueueInit(&g_ready_q); 
	_OS_QueueInit(&g_wait_q);
	_OS_QueueInit(&g_ap_ready_q);
	_OS_QueueInit(&g_block_q);
	
	// Initialize the Kernel process
	OS_CreateProcess(&kernel_pcb, "kernel", (SYSTEM_PROCESS | ADMIN_PROCESS), &kernel_process_entry, NULL);	
	g_kernel_process = &g_process_pool[kernel_pcb];
	
	// Initialize debug UART
	Uart_Init(UART0);	
		
	// Start the scheduling timer
	_OS_InitTimer();

	// Target Initialization
	_OS_TargetInit();	

    KlogStr(KLOG_OS_STARTUP, "Calling - ", "_OS_Start");
	
	// This will start the scheuling
	_OS_Start();
	
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
		|= ~((1ull << (32 - (MAX_TASK_COUNT & 0x1f))) - 1);	
	g_process_usage_mask[((MAX_PROCESS_COUNT + 31) >> 5) - 1] 
		|= ~((1ull << (32 - (MAX_PROCESS_COUNT & 0x1f))) - 1);	
	g_rdfile_usage_mask[((MAX_OPEN_FILES + 31) >> 5) - 1] 
		|= ~((1ull << (32 - (MAX_OPEN_FILES & 0x1f))) - 1);			
}

#if ENABLE_MMU

#if KERNEL_PAGE_SIZE==1024
#define KERNEL_VA_TO_PA_MAP_FUNCTION	_MMU_add_l1_va_to_pa_map
#elif KERNEL_PAGE_SIZE==64
#define KERNEL_VA_TO_PA_MAP_FUNCTION	_MMU_add_l2_large_page_va_to_pa_map
#elif KERNEL_PAGE_SIZE==4
#define KERNEL_VA_TO_PA_MAP_FUNCTION	_MMU_add_l2_small_page_va_to_pa_map
#else
#error "KERNEL_PAGE_SIZE should be either 1024 / 64 / 4"
#endif

void _OS_create_kernel_memory_map(_MMU_L1_PageTable * ptable)
{
	UINT32 length = (UINT32) ((UINT32)&__EX_system_area_end__ - (UINT32)&__EX_system_area_start__);
	
	// Create Map for Executable Kernel Sections
	KERNEL_VA_TO_PA_MAP_FUNCTION(ptable, 
			(VADDR) &__EX_system_area_start__, (PADDR) &__EX_system_area_start__, 
			length, KERNEL_EX_USER_NA, TRUE, TRUE);

	// Create Map for Read Only Kernel Sections
	length = (UINT32) ((UINT32)&__RO_system_area_end__ - (UINT32)&__RO_system_area_start__);
	KERNEL_VA_TO_PA_MAP_FUNCTION(ptable, 
			(VADDR) &__RO_system_area_start__, (PADDR) &__RO_system_area_start__, 
			length, KERNEL_RO_USER_NA, TRUE, TRUE);
				
	// Create Map for Read/Write Kernel Sections
	length = (UINT32) ((UINT32)&__RW_system_area_end__ - (UINT32)&__RW_system_area_start__);
	KERNEL_VA_TO_PA_MAP_FUNCTION(ptable, 
			(VADDR) &__RW_system_area_start__, (PADDR) &__RW_system_area_start__, 
			length, KERNEL_RW_USER_NA, TRUE, TRUE);
	
	// Create Map for Ramdisk space. Kernel will have read/write permissions
	KERNEL_VA_TO_PA_MAP_FUNCTION(ptable, 
			(VADDR) &__ramdisk_start__, (PADDR) &__ramdisk_start__, 
			(UINT32) &__ramdisk_length__, KERNEL_RW_USER_NA, TRUE, TRUE);

	// Create Map for Page Table space. Kernel will have read/write permissions
	// This space should not be cacheable / buffer-able
	KERNEL_VA_TO_PA_MAP_FUNCTION(ptable,
			(VADDR) &__page_table_area_start__, (PADDR) &__page_table_area_start__, 
			(UINT32) &__page_table_area_length__, KERNEL_RW_USER_NA, FALSE, FALSE);	
			
	//------------------------- Timer ---------------------------------
	// Create IO mappings for the kernel task before we access timer registers
	// Disable caching and write buffer for this region
	KERNEL_VA_TO_PA_MAP_FUNCTION(ptable, 
			(VADDR) ELFIN_TIMER_BASE, (PADDR) ELFIN_TIMER_BASE, 
			(UINT32) ONE_MB, KERNEL_RW_USER_NA, FALSE, FALSE);


	//------------------------- UART ---------------------------------
	// Create IO mappings for the kernel task before we access UART registers
	// Disable caching and write buffer for this region
	KERNEL_VA_TO_PA_MAP_FUNCTION(ptable, 
			(VADDR) ELFIN_UART_BASE, (PADDR) ELFIN_UART_BASE, 
			(UINT32) ONE_MB, KERNEL_RW_USER_NA, FALSE, FALSE);

	//------------------------- VIC ---------------------------------
	// Create IO mappings for the kernel task before we access VIC registers
	// Disable caching and write buffer for this region
	KERNEL_VA_TO_PA_MAP_FUNCTION(ptable, 
			(VADDR) ELFIN_VIC0_BASE_ADDR, (PADDR) ELFIN_VIC0_BASE_ADDR, 
			(UINT32) ONE_MB, KERNEL_RW_USER_NA, FALSE, FALSE);
	KERNEL_VA_TO_PA_MAP_FUNCTION(ptable, 
			(VADDR) ELFIN_VIC1_BASE_ADDR, (PADDR) ELFIN_VIC1_BASE_ADDR, 
			(UINT32) ONE_MB, KERNEL_RW_USER_NA, FALSE, FALSE);
	KERNEL_VA_TO_PA_MAP_FUNCTION(ptable, 
			(VADDR) ELFIN_VIC2_BASE_ADDR, (PADDR) ELFIN_VIC2_BASE_ADDR, 
			(UINT32) ONE_MB, KERNEL_RW_USER_NA, FALSE, FALSE);
	KERNEL_VA_TO_PA_MAP_FUNCTION(ptable, 
			(VADDR) ELFIN_VIC3_BASE_ADDR, (PADDR) ELFIN_VIC3_BASE_ADDR, 
			(UINT32) ONE_MB, KERNEL_RW_USER_NA, FALSE, FALSE);

	//------------------------- GPIO ---------------------------------
	// Create IO mappings for the kernel task before we access GPIO registers
	// Disable caching and write buffer for this region
	KERNEL_VA_TO_PA_MAP_FUNCTION(ptable, 
			(VADDR) ELFIN_GPIO_BASE, (PADDR) ELFIN_GPIO_BASE, 
			(UINT32) ONE_MB, KERNEL_RW_USER_NA, FALSE, FALSE);
}
#endif	// ENABLE_MMU
