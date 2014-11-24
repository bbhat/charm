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
		Note: Depending on the KERNEL_PAGE_SIZE used in os_config.h we need to have the
		matching alignment in the below map
	*/
	OS_MEM		    : ORIGIN = 0x20000000,  LENGTH = 0x80000
	OS_HEAP		    : ORIGIN = 0x20080000,  LENGTH = 0x80000
	USER_HEAP		: ORIGIN = 0x20100000,  LENGTH = 0x200000
	PAGE_TABLES 	: ORIGIN = 0x20300000,  LENGTH = 0x100000
	RAMDISK			: ORIGIN = 0x20400000,  LENGTH = 0x100000
	APP_MEM			: ORIGIN = 0x20500000,  LENGTH = 0x1FC00000
	FRAME_BUFFER	: ORIGIN = 0x23000000,  LENGTH = 0x100000
}

PHDRS
{
   code_seg		PT_LOAD;
   rodata_seg	PT_LOAD;
   data_seg		PT_LOAD;
   user_seg		PT_LOAD;
}

SECTIONS
{	
	/* Allocate space for kernel heap */
	__kernel_heap_start__  	   = 0x20080000;
	__kernel_heap_length__     = 0x80000;

	/* Allocate space for kernel heap */
	__user_heap_start__  	   = 0x20100000;
	__user_heap_length__       = 0x200000;

	/* Allocate space for page tables */
	__page_table_area_start__  = 0x20300000;
	__page_table_area_length__ = 0x100000;
	
	/* Creating space for Ramdisk */
	__ramdisk_start__  = 0x20400000;
	__ramdisk_length__ = 0x100000;
	
	/* Creating space for Frame Buffer */
	__frame_buffer_start__ = 0x23000000;
	__frame_buffer_length__ = 0x100000;

	/* Place other sections */
	.text :
	{
		__EX_system_area_start__  = .;
		*(.text.vectors)
		*(.text)
		*(.text.*)	
		__EX_system_area_end__  = .;
		
	} > OS_MEM : code_seg

	.rodata : ALIGN(0x10000)
	{
		__RO_system_area_start__ = .;
		*(.rodata)
		*(.rodata.*)
		__RO_system_area_end__ = .;
		
	} > OS_MEM : rodata_seg
	
	.data : ALIGN(0x10000)
	{
		__RW_system_area_start__  = .;
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
		__RW_system_area_end__ = .;
				
	} > OS_MEM : data_seg
	
	/* There are some user functions in the kernel binary. They should not share page with the kernel */
	.user : ALIGN(0x10000)
	{
		__EX_user_area_start__  = .;
		*(.user)
		__EX_user_area_end__  = .;
		
	} > OS_MEM : user_seg
}
