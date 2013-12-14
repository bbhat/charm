///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	elf_loader.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Loader for Elf files
//	
///////////////////////////////////////////////////////////////////////////////

#ifndef _ELF_LOADER_H
#define _ELF_LOADER_H

#include "elf.h"
#include "util.h"
#include "os_core.h"
#include "cache.h"


typedef struct {

	VADDR vaddr;
	UINT32 size;
	UINT32 flags;
	UINT32 align;
	
} Elf_SectionAttribute;

///////////////////////////////////////////////////////////////////////////////
// The following function reads elf program section attributes. This 
// can be later used to set the page table entries for the program
// elfdata -> pointer to the beginning elf file in the memory
// start_address -> this is updated with the starting address of the program
// sections -> this array will be populated with the elf program section attributes
// count -> the caller initializes with the number of entries in the sections argument.
//		It will be updated with the actual number of sections updated in the 
//		'sections' array.
///////////////////////////////////////////////////////////////////////////////
OS_Return elf_get_sections(void * elfdata, void ** start_address, 
						Elf_SectionAttribute * sections, UINT32 * count);

///////////////////////////////////////////////////////////////////////////////
// The following function loads elf program sections into memory. This involves
// physical copying of code and data from the elf file to memory.
// elfdata -> pointer to the beginning elf file in the memory
///////////////////////////////////////////////////////////////////////////////
OS_Return elf_load(void * elfdata);

#endif // _ELF_LOADER_H
