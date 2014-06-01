///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	os_memory.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: OS Memory Management Routines
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_memory.h"

/* Allocate space for kernel heap */
extern UINT32 __kernel_heap_start__;
extern UINT32 __kernel_heap_length__;

/* Allocate space for kernel heap */
extern UINT32 __user_heap_start__;
extern UINT32 __user_heap_length__;

// Global variable to track kernel memory allocations
static const UINTPTR g_kernel_heap_start = (UINTPTR)&__kernel_heap_start__;
static const UINT32 g_kernel_heap_length = (UINTPTR)&__kernel_heap_length__;
static UINT32 g_kernel_heap_alloc = 0;

static const UINTPTR g_user_heap_start = (UINTPTR)&__user_heap_start__;
static const UINT32 g_user_heap_length = (UINTPTR)&__user_heap_length__;

#if ENABLE_MMU

// Maps heap space into kernel
void _OS_MapHeapMemory(_MMU_L1_PageTable * ptable)
{
	// TODO: Instead of mapping the whole heap space into kernel at the beginning
	// it is better to map each page as and when it is needed. But this is more
	// complicated as of now because we need to map this space into every process's
	// kernel address space.
	
	// Create Map for Kernel heap space. Kernel will have read/write permissions
	KERNEL_VA_TO_PA_MAP_FUNCTION(ptable, 
			(VADDR) &__kernel_heap_start__, (PADDR) &__kernel_heap_start__, 
			(UINT32) &__kernel_heap_length__, KERNEL_RW_USER_NA, TRUE, TRUE);

	// Create Map for User heap space. Kernel will have read/write permissions.
	// Map for the user process will be added by malloc functions as and when
	// the memory is allocated.
	KERNEL_VA_TO_PA_MAP_FUNCTION(ptable, 
			(VADDR) &__user_heap_start__, (PADDR) &__user_heap_start__, 
			(UINT32) &__user_heap_length__, KERNEL_RW_USER_NA, TRUE, TRUE);
}

#endif	// ENABLE_MMU

// Routine for allocating memory in the kernel. Aligned memory will have word alignment
void * kmalloc(UINT32 size)
{ 
	return kmallocaligned(size, sizeof(UINTPTR)); 
}
 
// Routine for allocating memory in the kernel with a specified alignment
void * kmallocaligned(UINT32 size, UINT32 aligned)
{
	void * mem = NULL;
	UINT32 intsts;
	
	// Validate inputs
	ASSERT((size > 0) && (aligned > 0));
	
	// Ensure that 'aligned' is multiple of word size
	ASSERT((aligned & 0x3) == 0);
	
	// Ensure that 'aligned' is a power of two
	ASSERT((aligned & (aligned - 1)) == 0);
	
	OS_ENTER_CRITICAL(intsts);
	
	// Align 'g_kernel_heap_alloc' to requested alignment
	g_kernel_heap_alloc += (aligned - 1);
	g_kernel_heap_alloc &= ~(aligned - 1);

	// Check if we have sufficient space in the heap
	// Note that we have already mapped the hole heap space into kernel at the beginning
	if((g_kernel_heap_alloc + size) <= g_kernel_heap_length)
	{
		mem = (void *)(g_kernel_heap_start + g_kernel_heap_alloc);
		g_kernel_heap_alloc += size;
	}
	
	OS_EXIT_CRITICAL(intsts);
	
	return mem;
}

// Routine for allocating memory for user space process
void * malloc(UINT32 size)
{
	// TODO
	return NULL;
}
