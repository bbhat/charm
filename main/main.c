///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	main.c
//	Author:	Bala bhat (bhat.balasubramanya@gmail.com)
//	Description: main function
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_core.h"

OS_Process test_proc;

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
	
	OS_CreateProcessFromFile(&test_proc, "test_os", "/applications/bin/test_os.elf", NULL);
	
	OS_Start();
	
	return 0;
}
