///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	test_sem.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Semaphore Test file
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_core.h"
#include "target.h"

UINT32 stack1 [0x400];
UINT32 stack2 [0x400];
UINT32 stack3 [0x400];

int a = 0;
int b = 1;
int c = 2;
int d = 3;

OS_PeriodicTask task1, task2, task3;
OS_Sem_t sem1;
OS_Sem_t sem2;

void task_fn1(void * ptr)
{
	OS_SemWait(&sem1);
	user_led_toggle(*(int*)ptr);
}

void task_fn2(void * ptr)
{
	OS_SemWait(&sem2);
	user_led_toggle(*(int *)ptr);
	OS_SemPost(&sem1);
}

void task_fn3(void * ptr)
{
	OS_SemPost(&sem2);
	user_led_toggle(*(int *)ptr);
}

int main(int argc, char *argv[])
{
	OS_SemInit(&sem1, 0, 0);
	OS_SemInit(&sem2, 0, 0);
	LED_off(0);
	LED_off(1);
	LED_on(2);
	LED_off(3);
	OS_CreatePeriodicTask(500000, 300000, 10000, 0, stack1, sizeof(stack1), &task1, task_fn1, &a);
	OS_CreatePeriodicTask(500000, 400000, 10000, 100000, stack2, sizeof(stack2), &task2, task_fn2, &b);
	OS_CreatePeriodicTask(500000, 500000, 10000, 200000, stack3, sizeof(stack3), &task3, task_fn3, &c);
	
	return 0;
}


