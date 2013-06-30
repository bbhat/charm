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

int test_kernel()
{	
	SyslogStr("Calling - ",  __func__);

	OS_CreatePeriodicTask( 100000, 100000, 30000, 5000, stack1, sizeof(stack1), "LED1", &task1, task_task, &a);
  	OS_CreatePeriodicTask( 120000, 120000, 20000, 10000, stack2, sizeof(stack2), "LED2", &task2, task_task, &b);
  	OS_CreatePeriodicTask( 500000, 500000, 30000, 15000, stack3, sizeof(stack3), "LED3", &task3, task_task, &c);
  	OS_CreatePeriodicTask( 200000, 200000, 40000, 20000, stack4, sizeof(stack4), "LED4", &task4, task_task, &d);
	
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
