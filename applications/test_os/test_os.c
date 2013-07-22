///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	os_test.c
//	Author:	Bala bhat (bhat.balasubramanya@gmail.com)
//	Description: OS Test file
//	
///////////////////////////////////////////////////////////////////////////////


#include "os_api.h"

OS_PeriodicTask task1;
int a = 0;

void task_period(void * ptr)
{
	return;
}

int main(int argc, char *argv[])
{
	OS_CreatePeriodicTask( 100000, 100000, 30000, 5000, 512, "LED1", &task1, task_period, &a);
	
	if(!task1)
	{
		return -1;
	}

	return 0;
}
