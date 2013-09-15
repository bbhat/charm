///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	test_aperiodic.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Test Aperiodic APIs
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_api.h"

UINT32 stack1 [0x400];
UINT32 stack2 [0x400];

int a = 0;
int b = 1;
int c = 2;
int d = 3;

OS_Task task1, task2;
OS_Sem sem1;

void task_ap(void * ptr)
{
	int led = *(int *) ptr;
	while(1)
	{	
		PFM_SetUserLED(led, LED_TOGGLE);
		OS_SemWait(sem1);			
	}
}

void task_p(void * ptr)
{
	int led = *(int *) ptr;
	while(1)
	{	
		PFM_SetUserLED(led, LED_TOGGLE);
		OS_SemPost(sem1);
	}
}

int main(int argc, char *argv[])
{
	OS_SemAlloc(&sem1, 0);

	OS_CreatePeriodicTask(100000, 100000,5000, 0, stack1, sizeof(stack1), "LED1", &task1, task_p, &a);
	OS_CreateAperiodicTask(3, stack2, sizeof(stack2), "LED2", &task2, task_ap, &b);

	return 0;
}


