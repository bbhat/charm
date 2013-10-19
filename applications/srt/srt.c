///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	test_aperiodic.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Sorting test
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_api.h"

#define WORSTCASE 1
#define FALSE 0
#define TRUE 1
#define NUMELEMS 500
#define MAXDIM   (NUMELEMS+1)

typedef struct
{
	int array[NUMELEMS];
	int led;
} Argument;


OS_Task task1;
OS_Task task2;
OS_Task task3;
OS_Task task4;

UINT32 stack1[0x200];
UINT32 stack2[0x200];
UINT32 stack3[0x200];
UINT32 stack4[0x200];

Argument arg1, arg2, arg3, arg4;

/* BUBBLESORT BENCHMARK PROGRAM:
 * This program tests the basic loop constructs, integer comparisons,
 * and simple array handling of compilers by sorting 10 arrays of
 * randomly generated integers.
 */

//int Array[MAXDIM];

void Initialize(Array)
	int Array[];
	/*
	 * Initializes given array with randomly generated integers.
	 */

{
	int  Index, factor;

#ifdef WORSTCASE
	factor = -1;
#else
	factor = 1;
#endif

	for (Index = 0; Index < NUMELEMS; Index ++)
		Array[Index] = Index*factor;
}

void BubbleSort(Array)
	int Array[];
	/*
	 * Sorts an array of integers of size NUMELEMS in ascending order.
	 */
{
	int Sorted = FALSE;
	int Temp, Index, i;

	/* Created by Sibin M on May 11, 2005.
	 * to reflect true bubble sort.
	 */
	for (i = 0; i < NUMELEMS; i++) 
	{ 
		Sorted = TRUE; 
		for (Index = 0; Index < ( NUMELEMS - ( i + 1 ) ) ; Index ++) { 
			/*
			   if (Index > NUMELEMS-i) 
			   break; 
			 */

			if (Array[Index] > Array[Index + 1]) 
			{ 
				Temp = Array[Index]; 
				Array[Index] = Array[Index+1]; 
				Array[Index+1] = Temp; 
				Sorted = FALSE; 
			} 
		} 

		if (Sorted) 
			break; 
	} 
}

int TestResults(Array)
	int Array[];
	/*
	 * Initializes given array with randomly generated integers.
	 */

{
	int  Index;
	int result = 1;
	for (Index = 1; Index < NUMELEMS; Index ++)
		result = result && (Array[Index-1] <= Array[Index]);

	return result;
}

void srt_main(Argument *arg)
{
	int ret = 1;
	while(1)
	{
		Initialize(arg->array);
		BubbleSort(arg->array);

		ret = TestResults(arg->array);

		if(ret)
		{	
			PFM_SetUserLED(arg->led, LED_TOGGLE);
		}
		else
		{
			// The LED should stop blinking
			while(1);
		}
	}
}

int main(int argc, char *argv[]) { 
	arg1.led = 0; 
	arg2.led = 1; 
	arg3.led = 2; 
	arg4.led = 3;
	
	OS_CreatePeriodicTask( 100000, 100000, 30000, 5000, stack1, sizeof(stack1), "LED1", &task1, srt_main, &arg1);
	OS_CreatePeriodicTask( 120000, 120000, 20000, 10000, stack2, sizeof(stack2), "LED2", &task2, srt_main, &arg2);
  	OS_CreatePeriodicTask( 500000, 500000, 30000, 15000, stack3, sizeof(stack3), "LED3", &task3, srt_main, &arg3);
  	OS_CreatePeriodicTask( 200000, 200000, 40000, 20000, stack4, sizeof(stack4), "LED4", &task4, srt_main, &arg4);
		
	return 0;
}
