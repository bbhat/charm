/********************************************************************************
	
						Copyright 2012-2013 xxxxxxx, xxxxxxx
	File:	boot.ld
	Author:	Bala B. (bhat.balasubramanya@gmail.com)
	Description: Linker script for the boot program
	
********************************************************************************/

/********************************************************************************
					Memory Map
		Exception Vector Table		0xD003_7400
		Global Variable				0xD003_7480
		Signature					0xD003_7580
		SVC Stack					0xD003_7780
		IRQ Stack					0xD003_7D80
		BL1 Code Begin				0xD002_0010
		SRAM Begin					0xD002_0000
		
		BL1 start address should be 0xD002_0010 to be compatible with iROM

********************************************************************************/

OUTPUT_ARCH(arm)
ENTRY(_start)

MEMORY 
{
	CODE		: ORIGIN = 0xD0020010,  LENGTH = 0x153F0
	HEAP		: ORIGIN = 0xD0035400,  LENGTH = 0x2000
	VECTORS		: ORIGIN = 0xD0037400,  LENGTH = 0x80
	DATA		: ORIGIN = 0xD0037480,  LENGTH = 0x100
	SIGNATURE	: ORIGIN = 0xD0037580,  LENGTH = 0x200
	SVC_STACK	: ORIGIN = 0xD0037780,  LENGTH = 0x600
	IRQ_STACK	: ORIGIN = 0xD0037D80,  LENGTH = 0x200
}

SECTIONS
{	
	.text : {
		* (.startup)
		* (.text)
	}	> CODE
    		
	.data : {
		* (.data)
	}	> DATA
	
	.bss :
	{
		. = ALIGN(4);
		__bss_start__ = .;
		*(.bss)
		*(COMMON)
		. = ALIGN(4);
		__bss_end__ = .;
		
	} > DATA
	
	.stack :
	{
		*(.stack)
						
	} > SVC_STACK

}
