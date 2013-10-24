///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	mmu.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: MMU functions
//	
///////////////////////////////////////////////////////////////////////////////

#include "mmu.h"

#if ENABLE_MMU

// Following two variable are derived from the linker script
extern UINT32 __page_table_area_start__;
extern UINT32 __page_table_area_length__;

// Global variables to track page table allocations
static UINTPTR g_page_table_alloc_head = &__page_table_area_start__;
static UINT32 g_page_table_space_available = &__page_table_area_length__;

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
void _MMU_add_l1_va_to_pa_map(OS_Process proc, VADDR va, PADDR pa, _MMU_PTE_AccessPermission ap, 
								BOOL cache_enable, BOOL write_buffer)
{
}

// Function to create L2 VA to PA mapping for a given process								
void _MMU_add_l2_va_to_pa_map(OS_Process proc, VADDR va, PADDR pa, _MMU_PTE_AccessPermission ap,
								BOOL cache_enable, BOOL write_buffer)
{
}
#endif // ENABLE_MMU
