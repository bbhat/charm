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
OS_Sem sem2;

void task_fn3(void * ptr)
{
	int led = *(int *) ptr;
	volatile int i=0;		
	while(1)
	{	
		if(i++ % 1000000 == 0)
		{
			i = 1;
			PFM_SetUserLED(led, LED_TOGGLE);
			OS_SemWait(&sem1);			
		}
	}
}

void task_fn4(void * ptr)
{
	int led = *(int *) ptr;
	while(1)
	{	
		user_led_toggle(led);
		OS_SemPost(&sem1);
	}
}


int main(int argc, char *argv[])
{
	int ret;
	OS_SemInit(&sem1, 0, 0);
	OS_SemInit(&sem2, 0, 0);
	
	ret = OS_CreatePeriodicTask(100000, 100000, 5000, 0, stack1, sizeof(stack1), &task1, task_fn, &a);
	ret = OS_CreatePeriodicTask(200000, 200000, 10000, 0, stack2, sizeof(stack2), &task2, task_fn, &b);
	
	//aperiodic
	LED_on(2);
	LED_off(3);
	ret = OS_CreateAperiodicTask(3, stack3, sizeof(stack3), &task3, task_fn3, &c);
	ret = OS_CreateAperiodicTask(4, stack4, sizeof(stack4), &task4, task_fn4, &d);

	OS_Start();

	return ret;
}


