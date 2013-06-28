/********************************************************************************
	
						Copyright 2012-2013 xxxxxxx, xxxxxxx
	File:	memmap.ld
	Author:	Bala B. (bhat.balasubramanya@gmail.com)
	Description: Linker script for the OS image
	
********************************************************************************/

OUTPUT_ARCH(arm)
ENTRY(_start)

MEMORY 
{
	SRAM		: ORIGIN = 0x40000000,  LENGTH = 0x1000	
	DRAM		: ORIGIN = 0x30000000,  LENGTH = 0x4000000	

	VECTORS		: ORIGIN = 0xD0037400,  LENGTH = 0x80
	OS_MEM		: ORIGIN = 0x30000000,  LENGTH = 0x100000	
	RAMDISK		: ORIGIN = 0x30100000,  LENGTH = 0x100000	
	APP_MEM		: ORIGIN = 0x30200000,  LENGTH = 0x3E00000
}

PHDRS
{
   code_seg		PT_LOAD;
   rodata_seg		PT_LOAD;
   data_seg		PT_LOAD;
}

SECTIONS
{
	/* Creating space for Ramdisk */
	__ramdisk_start__ = 0x30100000;
	
	/* Interrupt Vectors */
	.vector :
	{
		*(.vector)
		
	} > VECTORS : code_seg


	/* Place other sections */
	.text :
	{
		*(.text.vector)
		*(.text.startup)
		*(.text)
		*(.text.*)	
		
	} > OS_MEM : code_seg

	.rodata : ALIGN(0x1000)
	{
		*(.rodata)
		*(.rodata.*)
		
	} > OS_MEM : rodata_seg
	
	.data : ALIGN(0x1000)
	{
		*(.data)
		
	} > OS_MEM : data_seg
	
	.bss :
	{
		. = ALIGN(4);
		__bss_start__ = .;
		*(.bss)
		*(COMMON)
		. = ALIGN(4);
		__bss_end__ = .;
		
	} > OS_MEM : data_seg
	
	.stack :
	{
		*(.stack)
						
	} > OS_MEM : data_seg
		
}
