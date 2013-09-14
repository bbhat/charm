///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	c_entry.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: c entry routine
//	
///////////////////////////////////////////////////////////////////////////////


extern int __bss_start__;
extern int __bss_end__;

int _start(int argc, char *argv[]) __attribute__ ((section (".text.startup")));
extern int main(int argc, char *argv[]);

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
	
	// Invoke main
	return main(argc, argv);
}
