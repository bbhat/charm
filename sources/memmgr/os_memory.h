///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	os_memory.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: OS Memory Management Routines
//		Safety critical systems such as this do not support deallocation of memory
//		once allocated. Because that will lead to memory fragmentation and may
//		eventually lead to failure. All memory needed should be allocated at the
//		time of initialization and never deallocated. 
//		If there is a need of frequent allocation / deallocation, then we should
//		use memory pools to accomplish the same
//	
///////////////////////////////////////////////////////////////////////////////

#ifndef _OS_MEMORY_H
#define _OS_MEMORY_H

#include "os_core.h"
#include "os_types.h"
#include "os_task.h"
#include "mmu.h"

#if ENABLE_MMU

// Maps heap space into kernel
void _OS_MapHeapMemory(_MMU_L1_PageTable * ptable);

#endif // ENABLE_MMU

// Routine for allocating memory in the kernel with a specified alignment
void * kmallocaligned(UINT32 size, UINT32 aligned);

// Routine for allocating memory in the kernel. Aligned memory will have word alignment
void * kmalloc(UINT32 size);

// Routine for allocating memory for user space process
void * malloc(UINT32 size);

#endif // _OS_MEMORY_H
