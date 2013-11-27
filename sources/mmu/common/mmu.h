///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	mmu.h
//	Author: Balasubramanya Bhat (bhat.balasubramanya@gmail.com)
//	Description: Header file for the OS Semaphore APIs
//	
///////////////////////////////////////////////////////////////////////////////
// 
//  The OS only supports 1M Sections and 4K Pages
//  1M section maps are added to L1 Page Table
//  4K page maps are added to L2 Page Tables
//
//  For efficiency and simplicity, we will have 1M pages for the kernel and 
//  4K pages for applications
///////////////////////////////////////////////////////////////////////////////

#include "os_core.h"

#ifndef _MMU_H
#define _MMU_H

#if ENABLE_MMU

#define PAGE_SIZE			(4 * ONE_KB)
#define SECTION_PAGE_SIZE	(ONE_MB)

typedef enum
{
	// See Table B4-1 MMU access permissions of ARM Architecture Reference Manual	
	// The values are assigned as follows:
	//	EX 	: Bit #3
	//	APX	: Bit #2
	//	AP	: Bit #1,0
	KERNEL_NA_USER_NA = (0),
	KERNEL_RW_USER_NA = (1),
	KERNEL_RW_USER_RO = (2),
	KERNEL_RW_USER_RW = (3),
	KERNEL_RO_USER_NA = ((1 << 2) | 1),
	KERNEL_RO_USER_RO = ((1 << 2) | 2),
	KERNEL_EX_USER_NA = ((1 << 3) | (1 << 2) | 1),
	KERNEL_RW_USER_EX = ((1 << 3) | 2)
	
} _MMU_PTE_AccessPermission;

typedef enum
{
	// L1 Page Table Entries
	PTE_FAULT		= 0,
	PTE_CORSE		= 1,
	PTE_SECTION		= 2,
	PTE_FINE		= 3,
	
	// L2 Page Table Entries
	L2PTE_FAULT		= 0,
	L2PTE_LARGE		= 1,
	L2PTE_SMALL		= 2
		
} _MMU_PTE_Type;

// The L1 page table has 4096 entries. Each entry describes 1M section of memory
typedef struct
{
	UINT32 pte[4096];
	
} _MMU_L1_PageTable;

// The L2 course page table has 256 entries. Each entry describes 4K sized page
typedef struct
{
	UINT32 pte[256];
	
} _MMU_L2_PageTable;

// The domain number to be used for kernel
#define KERNEL_DOMAIN		0

enum
{
	DOMAIN_ACCESS_NONE		= 0,	// generates a domain fault
	DOMAIN_ACCESS_CLIENT	= 1,	// access controlled by permission values set in PTE
	DOMAIN_ACCESS_MANAGER	= 3,	// access is uncontrolled, no permission aborts generated
	DOMAIN_ACCESS_MASK		= 3
};

// Functions to enable / disable MMU in SYSCTL co-processor
void _sysctl_enable_mmu();
void _sysctl_disable_mmu();

// Function to set page table address in the SYSCTL register
void _sysctl_set_ptable(PADDR ptable);

// Function to flush TLB
void _sysctl_flush_tlb(void);

// Function to set domain access rights
void _sysctl_set_domain_rights(UINT32 value, UINT32 mask);

// Function to allocate L1 Page Table
_MMU_L1_PageTable * _MMU_allocate_l1_page_table();

// Function to allocate L2 Page Table
_MMU_L2_PageTable * _MMU_allocate_l2_course_page_table();

// Function to create L1 VA to PA mapping for a given page table
OS_Error _MMU_add_l1_va_to_pa_map(_MMU_L1_PageTable * ptable, VADDR va, PADDR pa, 
								UINT32 size, _MMU_PTE_AccessPermission access, 
								BOOL cache_enable, BOOL write_buffer);

// Function to create L2 VA to PA mapping for a given page table							
OS_Error _MMU_add_l2_va_to_pa_map(_MMU_L1_PageTable * ptable, VADDR va, PADDR pa, 
								UINT32 size, _MMU_PTE_AccessPermission access,
								BOOL cache_enable, BOOL write_buffer);

// Function to create Kernel VA to PA mapping
void _OS_create_kernel_memory_map(_MMU_L1_PageTable * ptable);

#endif // ENABLE_MMU
#endif // _MMU_H
