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

#define TEST_KERNEL		1

OS_Process test_proc;
OS_PeriodicTask task1, task2, task3, task4;

UINT32 stack1 [0x400];
UINT32 stack2 [0x400];
UINT32 stack3 [0x400];
UINT32 stack4 [0x400];

int a = 0;
int b = 1;
int c = 2;
int d = 3;

void task_task(void * ptr)
{
	user_led_toggle(*(int *)ptr);
}

void task_budget(void * ptr)
{
	static int count = 0;
 	UINT32 dm = OS_GetTBECount();

	user_led_toggle(*(int *)ptr);
	Syslog32("task_budget - ", count++);

 	while(dm == OS_GetTBECount());
}

int test_kernel()
{	
	SyslogStr("Calling - ",  __func__);
	
	// Configure the LEDs
	configure_user_led(USER_LED0);
	configure_user_led(USER_LED1);
	configure_user_led(USER_LED2);
	configure_user_led(USER_LED3);

	// Test Casual
	OS_CreatePeriodicTask( 100000, 100000, 30000, 5000, stack1, sizeof(stack1), "LED1", &task1, task_task, &a);
  	OS_CreatePeriodicTask( 120000, 120000, 20000, 10000, stack2, sizeof(stack2), "LED2", &task2, task_task, &b);
  	OS_CreatePeriodicTask( 500000, 500000, 30000, 15000, stack3, sizeof(stack3), "LED3", &task3, task_task, &c);
  	OS_CreatePeriodicTask( 200000, 200000, 40000, 20000, stack4, sizeof(stack4), "LED4", &task4, task_task, &d);

	// test_short_intervals
// 	OS_CreatePeriodicTask( 100, 100, 50, 50, stack1, sizeof(stack1), "LED1", &task1, task_task, &a);
//  	OS_CreatePeriodicTask( 1200, 1200, 100, 400, stack2, sizeof(stack2), "LED2", &task2, task_task, &b);
//  	OS_CreatePeriodicTask( 5000, 5000, 500, 2500, stack3, sizeof(stack3), "LED3", &task3, task_task, &c);
//  	OS_CreatePeriodicTask(20000, 20000, 1000, 3500, stack4, sizeof(stack4), "LED4", &task4, task_task, &d);
	
	// Test Long Budget
// 	OS_CreatePeriodicTask( 1000000, 1000000, 300000, 0, stack1, sizeof(stack1), "LED1", &task1, task_budget, &a);
//   	OS_CreatePeriodicTask( 1200000, 1200000, 200000, 0, stack2, sizeof(stack2), "LED2", &task2, task_budget, &b);
//   	OS_CreatePeriodicTask( 5000000, 5000000, 300000, 0, stack3, sizeof(stack3), "LED3", &task3, task_budget, &c);
//   	OS_CreatePeriodicTask( 2000000, 2000000, 400000, 0, stack4, sizeof(stack4), "LED4", &task4, task_budget, &d);

	// Test long intervals	
// 	OS_CreatePeriodicTask( 1000000, 1000000, 50000, 0, stack1, sizeof(stack1), "LED1", &task1, task_task, &a);
//  	OS_CreatePeriodicTask( 1200000, 1200000, 20000, 0, stack2, sizeof(stack2), "LED2", &task2, task_task, &b);
//  	OS_CreatePeriodicTask( 5000000, 5000000, 30000, 0, stack3, sizeof(stack3), "LED3", &task3, task_task, &c);
//  	OS_CreatePeriodicTask( 2000000, 2000000, 90000, 0, stack4, sizeof(stack4), "LED4", &task4, task_task, &d);

	// Test TBE
// 	OS_CreatePeriodicTask( 1000, 1000, 300, 100, stack1, sizeof(stack1), "LED1", &task1, task_budget, &a);
//  	OS_CreatePeriodicTask( 1200, 1200, 400, 100, stack2, sizeof(stack2), "LED2", &task2, task_budget, &b);
//  	OS_CreatePeriodicTask( 5000, 5000, 500, 100, stack3, sizeof(stack3), "LED3", &task3, task_budget, &c);
//  	OS_CreatePeriodicTask(400000, 400000, 40000, 10000, stack4, sizeof(stack4), "LED4", &task4, task_budget, &d);

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
#endif

#if TEST_KERNEL==1
	OS_CreateProcess(&test_proc, "test_os", process_entry, NULL);
#else
	// Ensure that ramdisk is enabled
	#if ENABLE_RAMDISK==0
		"In order to load external processes (non-kernel), please set ENABLE_RAMDISK in OS Configuration file"
	#endif
	
	OS_CreateProcessFromFile(&test_proc, "test_os", "applications/bin/test_os.elf", NULL);
#endif
	return 0;
}
