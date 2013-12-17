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
UINT32 stack3 [0x400];
UINT32 stack4 [0x400];

int a = 0;
int b = 1;
int c = 2;
int d = 3;

OS_Task_t task1, task2, task3, task4;
OS_Sem_t sem1, sem2, sem3;

void task_ap1(void * ptr)
{
	int led = *(int *) ptr;
	while(1)
	{	
		OS_SemWait(sem1);			
		PFM_SetUserLED(led, LED_TOGGLE);
		OS_SemPost(sem2);
	}
}
void task_ap2(void * ptr)
{
	int led = *(int *) ptr;
	while(1)
	{	
		OS_SemWait(sem2);			
		PFM_SetUserLED(led, LED_TOGGLE);
		OS_SemPost(sem3);
	}
}
void task_ap3(void * ptr)
{
	int led = *(int *) ptr;
	while(1)
	{	
		OS_SemWait(sem3);			
		PFM_SetUserLED(led, LED_TOGGLE);
	}
}

void task_p(void * ptr)
{
	int led = *(int *) ptr;
	PFM_SetUserLED(led, LED_TOGGLE);
	OS_SemPost(sem1);
}

int main(int argc, char *argv[])
{
	OS_SemAlloc(&sem1, 0);
	OS_SemAlloc(&sem2, 0);
	OS_SemAlloc(&sem3, 0);

	OS_CreatePeriodicTask(1000, 1000, 400, 0, stack1, sizeof(stack1), "LED1", &task1, task_p, &a);
	OS_CreateAperiodicTask(3, stack2, sizeof(stack2), "LED2", &task2, task_ap1, &b);
	OS_CreateAperiodicTask(4, stack3, sizeof(stack3), "LED3", &task3, task_ap2, &c);
	OS_CreateAperiodicTask(5, stack4, sizeof(stack4), "LED4", &task4, task_ap3, &d);

	return 0;
}
