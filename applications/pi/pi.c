///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	test_aperiodic.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: PI value calculation
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_core.h"
#include "target.h"

extern OS_Task * g_current_task;
void pi_main(void* data);

void pi_main(void* data)
{	

	int numPartitions = 12000;
	int circleCount = 0;
	double interval = 0, pi = 0;
	int i = 0, j = 0;

	numPartitions = 300;

	interval = 1.0/(double)numPartitions;
	for (i = 0; i < numPartitions; i++) 
	{
		double a = (i + .5)*interval;
		for (j = 0; j < numPartitions; j++) 
		{
			double b = (j + .5)*interval;
			if ((a*a + b*b) <= 1) circleCount++;
		}
	}

	pi = (double)(4*circleCount)/(numPartitions * numPartitions);
	
	if(pi > 3.14 && pi < 3.15)
	{	
		// Correct Value
		user_led_toggle(*(int *)data);
	}
	else
	{
		// The LED should stop blinking
		while(1);
	}
}
