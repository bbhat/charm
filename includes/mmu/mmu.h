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

#ifndef _MMU_H
#define _MMU_H

#include os_config.h

#if ENABLE_MMU

#include "os_types.h"

typedef enum
{
	// See Table B4-1 MMU access permissions of ARM Architecture Reference Manual
	NO_ACCESS = 0,					// No Access: Privileged, User 
	PRIVILEGED_RW_USER_NA = 1,		// Read/Write: Privileged, No Access: User 
	PRIVILEGED_RW_USER_RO = 2,		// Read/Write: Privileged, Read Only: User 
	PRIVILEGED_RW_USER_RW = 3,		// Read/Write: Privileged, Read/Write: User 
	PRIVILEGED_RO_USER_NA = 5,		// Read/Only: Privileged, No Access: User 
	PRIVILEGED_RO_USER_RO = 6		// Read/Only: Privileged, Read/Only: User 
	
} _MMU_PTE_AccessPermission;

typedef enum
{
	PTE_FAULT		= 0,
	PTE_CORSE		= 1,
	PTE_SECTION		= 2,
	PTE_FINE		= 3
} _MMU_PTE_Type;

// The L1 page table has 4096 entries. Each entry describes 1M section of memory
typedef enum
{
	UINT32 pte[4096];
	
} _MMU_L1_PageTable;

// The L2 page table has 256 entries. Each entry describes 4K sized page
typedef enum
{
	UINT32 pte[256];
	
} _MMU_L2_PageTable;

// Functions to enable / disable MMU in SYSCTL co-processor
void _sysctl_enable_mmu();
void _sysctl_disable_mmu();

// Function to allocate L1 Page Table
_MMU_L1_PageTable * _MMU_allocate_l1_page_table();

// Function to allocate L2 Page Table
_MMU_L2_PageTable * _MMU_allocate_l2_page_table();

// Function to create L1 VA to PA mapping for a given page table
void _MMU_add_l1_va_to_pa_map(_MMU_L1_PageTable * ptable, VADDR va, PADDR pa, 
								UINT32 size, _MMU_PTE_AccessPermission ap, 
								BOOL cache_enable, BOOL write_buffer);

// Function to create L2 VA to PA mapping for a given page table							
void _MMU_add_l2_va_to_pa_map(_MMU_L1_PageTable * ptable, VADDR va, PADDR pa, 
								UINT32 size, _MMU_PTE_AccessPermission ap,
								BOOL cache_enable, BOOL write_buffer);

#endif // ENABLE_MMU
#endif // _MMU_H
