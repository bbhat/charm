/********************************************************************************
	
						Copyright 2012-2013 xxxxxxx, xxxxxxx
	File:	boot.ld
	Author:	Bala B. (bhat.balasubramanya@gmail.com)
	Description: Linker script for the boot program
	
********************************************************************************/

OUTPUT_ARCH(arm)
ENTRY(_start)

MEMORY 
{
	ROM		: ORIGIN = 0x00000,  LENGTH = 0x1000
}

SECTIONS
{	
	.text : {
		* (.startup)
		* (.text)
	}	> ROM    		
}
