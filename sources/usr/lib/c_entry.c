///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	c_entry.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: c entry routine
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_api.h"

extern int __bss_start__;
extern int __bss_end__;

int _start(int argc, char *argv[]) __attribute__ ((section (".text.startup")));
extern int main(int argc, char *argv[]);

OS_Driver_t __console_serial_driver__;

int _start(int argc, char *argv[])
{
	int * data_ptr = &__bss_start__;
	int * end_ptr = &__bss_end__;
	
	// Clear the bss section
	while(data_ptr < end_ptr)
	{
		*data_ptr = 0;
		data_ptr++;
	}
	
	// Get a handle to serial driver used for console output
	if(OS_DriverLookup("Serial", &__console_serial_driver__) != SUCCESS) {
		return -1;
	}
	
	// Open the driver in write mode
	if(OS_DriverOpen(__console_serial_driver__, ACCESS_READ | ACCESS_WRITE) != SUCCESS) {
		return -1;
	}
		
	// Invoke main
	return main(argc, argv);
}
