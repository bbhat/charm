///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	os_test.c
//	Author:	Bala bhat (bhat.balasubramanya@gmail.com)
//	Description: OS Test file
//	
///////////////////////////////////////////////////////////////////////////////


#include "os_api.h"

OS_Task_t task1;
OS_Task_t task2;
OS_Task_t task3;
OS_Task_t task4;

UINT32 stack1 [0x100];
UINT32 stack2 [0x100];
UINT32 stack3 [0x100];
UINT32 stack4 [0x100];

int a = 0;
int b = 1;
int c = 2;
int d = 3;

void task_period(void * ptr)
{
	PFM_SetUserLED(*(LED_Number *)ptr, LED_TOGGLE);
}

int count = 0;
void task_data_abort(void * ptr)
{
	PFM_SetUserLED(*(LED_Number *)ptr, LED_TOGGLE);
	
	// Causes abort after 100 seconds
	if(count++ == 10000)
	{
		count = *((volatile UINT32 *)0x20000000);
	}
}

void task_TBE(void * ptr)
{
	while(1);
}

int main(int argc, char *argv[])
{
	OS_CreatePeriodicTask( 10000, 10000, 3000, 0, stack1, sizeof(stack1), "LED1", &task1, task_period, &a);
	OS_CreatePeriodicTask( 12000, 12000, 2000, 1000, stack2, sizeof(stack2), "LED2", &task2, task_period, &b);
	OS_CreatePeriodicTask( 50000, 50000, 3000, 2000, stack3, sizeof(stack3), "LED3", &task3, task_period, &c);
	OS_CreatePeriodicTask( 20000, 20000, 4000, 2000, stack4, sizeof(stack4), "LED4", &task4, task_period, &d);
	
// 	OS_CreatePeriodicTask( 1000, 1000, 200, 5000, stack1, sizeof(stack1), "LED1", &task1, task_TBE, &a);
// 	OS_CreatePeriodicTask( 2000, 2000, 500, 1000, stack2, sizeof(stack2), "LED2", &task2, task_TBE, &b);
// 	OS_CreatePeriodicTask( 5000, 5000, 500, 0, stack3, sizeof(stack3), "LED3", &task3, task_TBE, &c);
// 	OS_CreatePeriodicTask(100000, 100000, 10000, 3000, stack4, sizeof(stack4), "LED4", &task4, task_TBE, &d);
	
	return 0;
}
