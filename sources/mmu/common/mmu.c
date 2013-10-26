///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	mmu.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: MMU functions
//	
///////////////////////////////////////////////////////////////////////////////

#include "mmu.h"
#include "target.h"
#include "util.h"

#if ENABLE_MMU

// Following two variable are derived from the linker script
extern UINT32 __page_table_area_start__;
extern UINT32 __page_table_area_length__;

// Global variables to track page table allocations
static UINTPTR g_page_table_alloc_head = (UINTPTR)&__page_table_area_start__;
static UINT32 g_page_table_space_available = (UINT32)&__page_table_area_length__;

// Function to allocate L1 Page Table
_MMU_L1_PageTable * _MMU_allocate_l1_page_table()
{
	_MMU_L1_PageTable * pt = (_MMU_L1_PageTable *) g_page_table_alloc_head;
	
	if(g_page_table_space_available < sizeof(_MMU_L1_PageTable))
	{
		KlogStr(KLOG_WARNING, "%s: Could not allocate L1 page table", __FUNCTION__);
		return NULL;
	}
	
	// Clear the page table so that we don't have incorrect va to pa mappings
	memset(pt, 0, sizeof(_MMU_L1_PageTable));
	
	// Adjust the page table area pointers
	g_page_table_alloc_head += sizeof(_MMU_L1_PageTable);
	g_page_table_space_available -= sizeof(_MMU_L1_PageTable);
	
	return pt;
}

// Function to allocate L2 Page Table
_MMU_L2_PageTable * _MMU_allocate_l2_page_table()
{
	_MMU_L2_PageTable * pt = (_MMU_L2_PageTable *) g_page_table_alloc_head;
	
	if(g_page_table_space_available < sizeof(_MMU_L2_PageTable))
	{
		KlogStr(KLOG_WARNING, "%s: Could not allocate L2 page table", __FUNCTION__);
		return NULL;
	}

	// Clear the page table so that we don't have incorrect va to pa mappings
	memset(pt, 0, sizeof(_MMU_L2_PageTable));
	
	// Adjust the page table area pointers
	g_page_table_alloc_head += sizeof(_MMU_L2_PageTable);
	g_page_table_space_available -= sizeof(_MMU_L2_PageTable);
	
	return pt;
}

// Function to create L1 VA to PA mapping for a given process
void _MMU_add_l1_va_to_pa_map(_MMU_L1_PageTable * ptable, VADDR va, PADDR pa, 
								UINT32 size, _MMU_PTE_AccessPermission ap, 
								BOOL cache_enable, BOOL write_buffer)
{
#if _ARM_ARCH >= 6

	// Validate the inputs
	ASSERT(ptable && size);
	
	// Extract the section_base_address. Since each section is 1MB in size, extract
	// the top 12 bits of the virtual address
	UINT32 section_base_address = (va >> 20) & 0xfff;
	
	// Convert size to multiples of 1 MB
	size += (0x100000 - 1);
	size &= ~(0x100000 - 1);
	
	while(size > 0)
	{
		ptable->pte[section_base_address] = 
			((pa & 0xfff00000) | 				// Base physical address of the section
			((ap & 0x4) << 13) |				// APX: Extended Access permissions
			((ap & 0x3) << 10) |				// Access permissions
			(KERNEL_DOMAIN << 5) |				// Domain
			(cache_enable ? (1 << 3) : 0) |		// Enable cache?
			(write_buffer ? (1 << 2) : 0) |		// Enable write buffer?
			PTE_SECTION);						// Section Page Table Entry
		
		section_base_address++;
		size -= 0x100000;
	}	
	
#else
	#error "Not implemented Yet"
#endif
	
}

// Function to create L2 VA to PA mapping for a given process								
void _MMU_add_l2_va_to_pa_map(_MMU_L1_PageTable * ptable, VADDR va, PADDR pa, 
								UINT32 size, _MMU_PTE_AccessPermission ap,
								BOOL cache_enable, BOOL write_buffer)
{
}
#endif // ENABLE_MMU
