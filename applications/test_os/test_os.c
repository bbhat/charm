///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	os_test.c
//	Author:	Bala bhat (bhat.balasubramanya@gmail.com)
//	Description: OS Test file
//	
///////////////////////////////////////////////////////////////////////////////


#include "os_api.h"

OS_Task task1;
OS_Task task2;
OS_Task task3;
OS_Task task4;

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

int main(int argc, char *argv[])
{
	OS_CreatePeriodicTask( 100000, 100000, 30000, 5000, stack1, sizeof(stack1), "LED1", &task1, task_period, &a);
  	OS_CreatePeriodicTask( 120000, 120000, 20000, 10000, stack2, sizeof(stack2), "LED2", &task2, task_period, &b);
//   OS_CreatePeriodicTask( 500000, 500000, 30000, 15000, stack3, sizeof(stack3), "LED3", &task3, task_period, &c);
//   OS_CreatePeriodicTask( 200000, 200000, 40000, 20000, stack4, sizeof(stack4), "LED4", &task4, task_period, &d);
		
	return 0;
}
