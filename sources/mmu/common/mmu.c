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

// Global variable to track L1 page table allocations
static UINTPTR g_l1_page_table_alloc_head = (UINTPTR)&__page_table_area_start__;

// Reserve space for L1 page tables for all processes
static INT32 g_l1_page_table_space_available = (MAX_PROCESS_COUNT * sizeof(_MMU_L1_PageTable));

// Global variable to track L2 page table allocations. The L2 page tables start where the L1 page tables end.
static UINTPTR g_l2_page_table_alloc_head = (UINTPTR)&__page_table_area_start__ + (MAX_PROCESS_COUNT * sizeof(_MMU_L1_PageTable));

// Space available for L2 page tables
static INT32 g_l2_page_table_space_available = ((UINT32)&__page_table_area_length__ - (MAX_PROCESS_COUNT * sizeof(_MMU_L1_PageTable)));

extern OS_Process	* g_kernel_process;	// Kernel process

#ifdef _USE_STD_LIBS
	#define FAULT(x, ...) printf(x, ...);
#else
	#define FAULT(x, ...)
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// Function to allocate L1 Page Table
//		The L1 page table address should be 16 KB aligned
//////////////////////////////////////////////////////////////////////////////////////////
_MMU_L1_PageTable * _MMU_allocate_l1_page_table()
{
	_MMU_L1_PageTable * pt = (_MMU_L1_PageTable *) g_l1_page_table_alloc_head;
	
	if(g_l1_page_table_space_available < sizeof(_MMU_L1_PageTable))
	{
		KlogStr(KLOG_WARNING, "%s: Could not allocate L1 page table", __FUNCTION__);
		return NULL;
	}
	
	// Clear the page table so that we don't have incorrect va to pa mappings
	memset(pt, 0, sizeof(_MMU_L1_PageTable));
	
	// Adjust the page table area pointers
	g_l1_page_table_alloc_head += sizeof(_MMU_L1_PageTable);
	g_l1_page_table_space_available -= sizeof(_MMU_L1_PageTable);
	
	return pt;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Function to allocate L2 Page Course Table
//		The L2 page table address should be 1 KB aligned
//////////////////////////////////////////////////////////////////////////////////////////
_MMU_L2_PageTable * _MMU_allocate_l2_course_page_table()
{
	_MMU_L2_PageTable * pt = (_MMU_L2_PageTable *) g_l2_page_table_alloc_head;
	
	if(g_l2_page_table_space_available < sizeof(_MMU_L2_PageTable))
	{
		KlogStr(KLOG_WARNING, "%s: Could not allocate L2 page table", __FUNCTION__);
		return NULL;
	}

	// Clear the page table so that we don't have incorrect va to pa mappings
	memset(pt, 0, sizeof(_MMU_L2_PageTable));
	
	// Adjust the page table area pointers
	g_l2_page_table_alloc_head += sizeof(_MMU_L2_PageTable);
	g_l2_page_table_space_available -= sizeof(_MMU_L2_PageTable);
	
	return pt;
}

// Function to create L1 VA to PA mapping for a given process
OS_Return _MMU_add_l1_va_to_pa_map(_MMU_L1_PageTable * ptable, VADDR va, PADDR pa, 
								UINT32 size, _MMU_PTE_AccessPermission access, 
								BOOL cache_enable, BOOL write_buffer)
{
#if _ARM_ARCH >= 6
	
	// If ptable is NULL, assume kernel process	
	if(!ptable) {
		ASSERT(g_kernel_process);
		ptable = g_kernel_process->ptable;
	}
	
	// Validate the inputs
	ASSERT(ptable && size);
	
	// Extract the section_base_address. Since each section is 1MB in size, extract
	// the top 12 bits of the virtual address
	UINT32 section_base_address = (va >> 20) & 0xfff;
	
	// Ensure that virtual and physical addresses are SECTION_PAGE_SIZE aligned
	ASSERT((va & (SECTION_PAGE_SIZE - 1)) == 0);
	ASSERT((pa & (SECTION_PAGE_SIZE - 1)) == 0);
	
	// Convert size to multiples of SECTION_PAGE_SIZE
	size += (SECTION_PAGE_SIZE - 1);
	size &= ~(SECTION_PAGE_SIZE - 1);
	
	// Extract access permissions
	const UINT32 AP = (access & 0x03);
	const UINT32 APX = (access & 0x04) >> 2;
	const UINT32 XN = (~access & 0x08) >> 3;
	
	while(size > 0)
	{
		ptable->pte[section_base_address] = 
			((pa & 0xfff00000) | 				// Base physical address of the section
			(APX << 15) |						// APX: Extended Access permissions
			(AP << 10) |						// Access permissions
			(XN << 4) |							// Execute Never
			(KERNEL_DOMAIN << 5) |				// Domain
			(cache_enable ? (1 << 3) : 0) |		// Enable cache?
			(write_buffer ? (1 << 2) : 0) |		// Enable write buffer?
			PTE_SECTION);						// Section Page Table Entry
		
		section_base_address++;
		size -= SECTION_PAGE_SIZE;
		pa += SECTION_PAGE_SIZE;
		va += SECTION_PAGE_SIZE;
	}
#else
	#error "Not implemented Yet"
#endif
	return SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////
// Function to create L2 VA to PA mapping in a given ptable using large pages (64K)
// If we run out of page table space the function just returns with whatever 
// area it could map before failing
/////////////////////////////////////////////////////////////////////////////////
OS_Return _MMU_add_l2_large_page_va_to_pa_map(_MMU_L1_PageTable * ptable, VADDR va, PADDR pa, 
								UINT32 size, _MMU_PTE_AccessPermission access,
								BOOL cache_enable, BOOL write_buffer)
{
#if _ARM_ARCH >= 6

	_MMU_L2_PageTable * l2_ptable;
	PADDR l2_pa;
	
	// If ptable is NULL, assume kernel process	
	if(!ptable) {
		ASSERT(g_kernel_process);
		ptable = g_kernel_process->ptable;
	}
	
	// Validate the inputs
	ASSERT(ptable && size);

	// Ensure that virtual and physical addresses are LARGE_PAGE_SIZE aligned
	ASSERT((va & (LARGE_PAGE_SIZE - 1)) == 0);
	ASSERT((pa & (LARGE_PAGE_SIZE - 1)) == 0);	
		
	// Convert size to multiples of PAGE_SIZE
	size += (LARGE_PAGE_SIZE - 1);
	size &= ~(LARGE_PAGE_SIZE - 1);
	
	// Extract access permissions
	const UINT32 AP = (access & 0x03);
	const UINT32 APX = (access & 0x04) >> 2;
	const UINT32 XN = (~access & 0x08) >> 3;

	// For L2 page tables, we should first have an L1 page table entry
	// First level loop for the L1 page table	
	while(size > 0)
	{
		// Extract the section_base_address. Since each section is 1MB in size, extract
		// the top 12 bits of the virtual address
		UINT32 section_base_address = (va >> 20) & 0xfff;
		
		// Check if there is already an L1 page table entry. 
		// If not, then allocate an L2 page table and create an entry in the L1 page table to point 
		// to this L2 table
		if((ptable->pte[section_base_address] & 0x03) == PTE_FAULT)
		{
			// Allocate L2 course page table
			l2_ptable = _MMU_allocate_l2_course_page_table();
			
			// Physical address of L2 table. We are using va == pa.
			l2_pa = (PADDR) l2_ptable;
			
			if(!l2_ptable)
			{
				FAULT("Could not allocate L2 page table: No space in page table area\n");
				return RESOURCE_EXHAUSTED;	
			}
			
			// Update the L1 page table
			ptable->pte[section_base_address] = 
				((l2_pa & 0xfffffc00) | 			// Base physical address of the section
				(KERNEL_DOMAIN << 5) |				// Domain
				PTE_CORSE);							// Section Page Table Entry						
		}
		else
		{
			// There is already an L2 page table allocated. Get it's address. Since we are using va == pa,
			// we can reconstruct it back from the l1 page table
			l2_pa = (PADDR) ptable->pte[section_base_address] & 0xfffffc00;
			l2_ptable = (_MMU_L2_PageTable *) l2_pa;			
		}
		
		// The l2 index is contained in bits [19..12]
		UINT32 l2_index = (va >> 12) & 0xff;
		do
		{
			// Create l2 course page table entry
			l2_ptable->pte[l2_index] = 
				((pa & 0xffff0000) | 				// Base physical address of the section
				(XN << 15) |						// Execute Never flag
				(APX << 9) |						// APX: Extended Access permissions
				(AP << 4) |							// Access permissions
				(cache_enable ? (1 << 3) : 0) |		// Enable cache?
				(write_buffer ? (1 << 2) : 0) |		// Enable write buffer?
				L2PTE_LARGE);						// Small Page Table Entry
			
			// We need to repeat the large page table entry multiple times (16)
			// Using PAGE_SIZE instead of LARGE_PAGE_SIZE below will accomplish this
			l2_index = (l2_index + 1) & 0xff;
			size -= PAGE_SIZE;
			va += PAGE_SIZE;
			pa += PAGE_SIZE;
		}
		while(l2_index && size);
	}
#else
	#error "Not implemented Yet"
#endif
	return SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////
// Function to create L2 VA to PA mapping for a given ptable using small pages (4K)
// This creates L2 Course Page table entries. If we run out of page table space
// the function just returns with whatever area it could map before failing
/////////////////////////////////////////////////////////////////////////////////
OS_Return _MMU_add_l2_small_page_va_to_pa_map(_MMU_L1_PageTable * ptable, VADDR va, PADDR pa, 
								UINT32 size, _MMU_PTE_AccessPermission access,
								BOOL cache_enable, BOOL write_buffer)
{
#if _ARM_ARCH >= 6

	_MMU_L2_PageTable * l2_ptable;
	PADDR l2_pa;
	
	// If ptable is NULL, assume kernel process	
	if(!ptable) {
		ASSERT(g_kernel_process);
		ptable = g_kernel_process->ptable;
	}
	
	// Validate the inputs
	ASSERT(ptable && size);

	// Ensure that virtual and physical addresses are PAGE_SIZE aligned
	ASSERT((va & (PAGE_SIZE - 1)) == 0);
	ASSERT((pa & (PAGE_SIZE - 1)) == 0);	
		
	// Convert size to multiples of PAGE_SIZE
	size += (PAGE_SIZE - 1);
	size &= ~(PAGE_SIZE - 1);
	
	// Extract access permissions
	const UINT32 AP = (access & 0x03);
	const UINT32 APX = (access & 0x04) >> 2;
	const UINT32 XN = (~access & 0x08) >> 3;

	// For L2 page tables, we should first have an L1 page table entry
	// First level loop for the L1 page table	
	while(size > 0)
	{
		// Extract the section_base_address. Since each section is 1MB in size, extract
		// the top 12 bits of the virtual address
		UINT32 section_base_address = (va >> 20) & 0xfff;
		
		// Check if there is already an L1 page table entry. 
		// If not, then allocate an L2 page table and create an entry in the L1 page table to point 
		// to this L2 table
		if((ptable->pte[section_base_address] & 0x03) == PTE_FAULT)
		{
			// Allocate L2 course page table
			l2_ptable = _MMU_allocate_l2_course_page_table();
			
			// Physical address of L2 table. We are using va == pa.
			l2_pa = (PADDR) l2_ptable;
			
			if(!l2_ptable)
			{
				FAULT("Could not allocate L2 page table: No space in page table area\n");
				return RESOURCE_EXHAUSTED;	
			}
			
			// Update the L1 page table
			ptable->pte[section_base_address] = 
				((l2_pa & 0xfffffc00) | 			// Base physical address of the section
				(KERNEL_DOMAIN << 5) |				// Domain
				PTE_CORSE);							// Section Page Table Entry						
		}
		else
		{
			// There is already an L2 page table allocated. Get it's address. Since we are using va == pa,
			// we can reconstruct it back from the l1 page table
			l2_pa = (PADDR) ptable->pte[section_base_address] & 0xfffffc00;
			l2_ptable = (_MMU_L2_PageTable *) l2_pa;			
		}
		
		// The l2 index is contained in bits [19..12]
		UINT32 l2_index = (va >> 12) & 0xff;
		do
		{
			// Create l2 course page table entry
			l2_ptable->pte[l2_index] = 
				((pa & 0xfffff000) | 				// Base physical address of the section
				(APX << 9) |						// APX: Extended Access permissions
				(AP << 4) |							// Access permissions
				(cache_enable ? (1 << 3) : 0) |		// Enable cache?
				(write_buffer ? (1 << 2) : 0) |		// Enable write buffer?
				L2PTE_SMALL |						// Small Page Table Entry
				(XN << 0));							// Execute Never flag		
			
			l2_index = (l2_index + 1) & 0xff;
			size -= PAGE_SIZE;
			va += PAGE_SIZE;
			pa += PAGE_SIZE;
		}
		while(l2_index && size);
	}
#else
	#error "Not implemented Yet"
#endif
	return SUCCESS;
}

#endif // ENABLE_MMU
