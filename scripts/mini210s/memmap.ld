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
	DRAM			: ORIGIN = 0x20000000,  LENGTH = 0x20000000	

	/* 
		Since we use only L1 page table for the kernel, each of the following sections
		should be 1MB aligned
	*/
	OS_MEM_EX		: ORIGIN = 0x20000000,  LENGTH = 0x100000
	OS_MEM_RO		: ORIGIN = 0x20100000,  LENGTH = 0x100000
	OS_MEM_RW		: ORIGIN = 0x20200000,  LENGTH = 0x100000
	PAGE_TABLES 	: ORIGIN = 0x20300000,  LENGTH = 0x100000
	RAMDISK			: ORIGIN = 0x20400000,  LENGTH = 0x100000
	APP_MEM			: ORIGIN = 0x20500000,  LENGTH = 0x1FC00000
}

PHDRS
{
   code_seg		PT_LOAD;
   rodata_seg	PT_LOAD;
   data_seg		PT_LOAD;
}

SECTIONS
{
	/* OS System space for page tables */
	__EX_system_area_start__  = 0x20000000;
	__EX_system_area_length__ = 0x100000;
	__RO_system_area_start__  = 0x20100000;
	__RO_system_area_length__ = 0x100000;
	__RW_system_area_start__  = 0x20200000;
	__RW_system_area_length__ = 0x100000;
	
	/* Allocatine space for page tables */
	__page_table_area_start__  = 0x20300000;
	__page_table_area_length__ = 0x100000;
	
	/* Creating space for Ramdisk */
	__ramdisk_start__  = 0x20400000;
	__ramdisk_length__ = 0x100000;

	/* Place other sections */
	.text :
	{
		*(.text.vectors)
		*(.text)
		*(.text.*)	
		
	} > OS_MEM_EX : code_seg

	.rodata : ALIGN(0x1000)
	{
		*(.rodata)
		*(.rodata.*)
		
	} > OS_MEM_RO : rodata_seg
	
	.data : ALIGN(0x1000)
	{
		*(.data)
		
	} > OS_MEM_RW : data_seg
	
	.bss :
	{
		. = ALIGN(4);
		__bss_start__ = .;
		*(.bss)
		*(COMMON)
		. = ALIGN(4);
		__bss_end__ = .;
		
	} > OS_MEM_RW : data_seg
	
	.stack :
	{
		*(.stack)
						
	} > OS_MEM_RW : data_seg
	
}
