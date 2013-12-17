///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	main.c
//	Author:	Bala bhat (bhat.balasubramanya@gmail.com)
//	Description: main function
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_core.h"
#include "target.h"
#include "sysctl.h"
#include "mmu.h"

//#define TEST_KERNEL
#define TEST_OS
//#define TEST_SRT
//#define TEST_APERIODIC

OS_Process_t test_proc;
OS_Process_t test_proc1;
OS_Process_t test_proc2;

OS_Task_t task1, task2, task3, task4;

UINT32 stack1 [0x400];
UINT32 stack2 [0x400];
UINT32 stack3 [0x400];
UINT32 stack4 [0x400];

int a = 0;
int b = 1;
int c = 2;
int d = 3;

UINT32 _sysctl_query_l2_aux_control_reg(void);
	
void task_task(void * ptr)
{
	PFM_SetUserLED(*(LED_Number *)ptr, LED_TOGGLE);
}

void task_budget(void * ptr)
{
//	static int count = 0;
 	UINT32 dm = OS_GetTBECount();

	PFM_SetUserLED(*(LED_Number *)ptr, LED_TOGGLE);
//	Syslog32("task_budget - ", count++);

	while(dm == OS_GetTBECount());
	//	while(1);
}

int test_kernel()
{	
	SyslogStr("Calling - ",  __func__);
	
	// Test Casual
	OS_CreatePeriodicTask( 100000, 100000, 30000, 5000, stack1, sizeof(stack1), "LED1", &task1, task_task, &a);
	OS_CreatePeriodicTask( 120000, 120000, 20000, 10000, stack2, sizeof(stack2), "LED2", &task2, task_task, &b);
	OS_CreatePeriodicTask( 500000, 500000, 30000, 15000, stack3, sizeof(stack3), "LED3", &task3, task_task, &c);
	OS_CreatePeriodicTask( 200000, 200000, 40000, 20000, stack4, sizeof(stack4), "LED4", &task4, task_task, &d);

	// test_short_intervals
// 	OS_CreatePeriodicTask( 1000, 1000, 400, 5000, stack1, sizeof(stack1), "LED1", &task1, task_task, &a);
//  OS_CreatePeriodicTask( 2000, 2000, 500, 1000, stack2, sizeof(stack2), "LED2", &task2, task_task, &b);
//  OS_CreatePeriodicTask( 5000, 5000, 1000, 0, stack3, sizeof(stack3), "LED3", &task3, task_task, &c);
//  OS_CreatePeriodicTask(100000, 100000, 10000, 3000, stack4, sizeof(stack4), "LED4", &task4, task_task, &d);
	
	// Test Long Budget
// 	OS_CreatePeriodicTask( 100000, 100000, 10000, 0, stack1, sizeof(stack1), "LED1", &task1, task_budget, &a);
//  OS_CreatePeriodicTask( 120000, 120000, 12000, 0, stack2, sizeof(stack2), "LED2", &task2, task_budget, &b);
//  OS_CreatePeriodicTask( 500000, 500000, 15000, 0, stack3, sizeof(stack3), "LED3", &task3, task_budget, &c);
//  OS_CreatePeriodicTask( 200000, 200000, 20000, 0, stack4, sizeof(stack4), "LED4", &task4, task_budget, &d);

	// Test long intervals	
// 	OS_CreatePeriodicTask( 1000000, 1000000, 50000, 0, stack1, sizeof(stack1), "LED1", &task1, task_task, &a);
// 	OS_CreatePeriodicTask( 1200000, 1200000, 20000, 0, stack2, sizeof(stack2), "LED2", &task2, task_task, &b);
// 	OS_CreatePeriodicTask( 5000000, 5000000, 30000, 0, stack3, sizeof(stack3), "LED3", &task3, task_task, &c);
// 	OS_CreatePeriodicTask( 2000000, 2000000, 90000, 0, stack4, sizeof(stack4), "LED4", &task4, task_task, &d);

	// Test TBE
// 	OS_CreatePeriodicTask( 1000, 1000, 300, 1000, stack1, sizeof(stack1), "LED1", &task1, task_budget, &a);
// 	OS_CreatePeriodicTask( 2000, 2000, 400, 2000, stack2, sizeof(stack2), "LED2", &task2, task_budget, &b);
// 	OS_CreatePeriodicTask( 5000, 5000, 500, 3000, stack3, sizeof(stack3), "LED3", &task3, task_budget, &c);
// 	OS_CreatePeriodicTask(400000, 400000, 40000, 10000, stack4, sizeof(stack4), "LED4", &task4, task_budget, &d);

	return 0;
}

void process_entry(void * pdata)
{
	test_kernel();
}

///////////////////////////////////////////////////////////////////////////////
// main function for the Kernel
// Note: This is the place where we should create all the required processes
// and kernel tasks (for example driver related tasks) using APIs in os_core.h
// Use - OS_CreateProcess to create processes.
// Use - OS_CreatePeriodicTask / OS_CreateAperiodicTask to create kernel tasks
//			that runs in the kernel space
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
	SyslogStr("Calling - ",  __func__);
	
	// Print some general info about the platform
	Klog32(KLOG_GENERAL_INFO, "icache line len - ", _icache_line_size);
	Klog32(KLOG_GENERAL_INFO, "icache set num - ", _icache_set_count);
	Klog32(KLOG_GENERAL_INFO, "icache ways - ", _icache_ways_count);
	
	Klog32(KLOG_GENERAL_INFO, "dcache line len - ", _dcache_line_size);
	Klog32(KLOG_GENERAL_INFO, "dcache set num - ", _dcache_set_count);
	Klog32(KLOG_GENERAL_INFO, "dcache ways - ", _dcache_ways_count);

#if ENABLE_L2_CACHE
	Klog32(KLOG_GENERAL_INFO, "l2 cache line len - ", _l2_cache_line_size);
	Klog32(KLOG_GENERAL_INFO, "l2 cache set num - ", _l2_cache_set_count);
	Klog32(KLOG_GENERAL_INFO, "l2 cache ways - ", _l2_cache_ways_count);
	Klog32(KLOG_GENERAL_INFO, "l2 cache control register - ", _sysctl_query_l2_aux_control_reg());
#endif
	
#if OS_ENABLE_CPU_STATS==1
	// Create IO mappings for the kernel task before we access timer registers
	// Disable caching and write buffer for this region
	OS_CreateProcessFromFile(&test_proc1, "STAT", ADMIN_PROCESS, "applications/bin/SystemMonitor.elf", NULL);	
#endif

#if defined(TEST_KERNEL)
	OS_CreateProcess(&test_proc, "test_kernel", 0, process_entry, NULL);
#else
	// Ensure that ramdisk is enabled
	#if ENABLE_RAMDISK==0
		#error "In order to load external processes (non-kernel), please set ENABLE_RAMDISK in OS Configuration file"
	#endif

#if defined(TEST_OS)	
	OS_CreateProcessFromFile(&test_proc1, "test_os", 0, "applications/bin/test_os.elf", NULL);
#endif

#if defined(TEST_SRT)
	OS_CreateProcessFromFile(&test_proc2, "srt", 0, "applications/bin/srt.elf", NULL);
#endif

#if defined(TEST_APERIODIC)
	OS_CreateProcessFromFile(&test_proc2, "test_aperiodic", 0, "applications/bin/test_aperiodic.elf", NULL);
#endif

#endif
	return 0;
}
