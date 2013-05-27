///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	elf.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Utility functions for handling ELF files
//	
///////////////////////////////////////////////////////////////////////////////

#include "elf.h"
#include "util.h"
#include "os_core.h"
#include "cache.h"

#ifdef _USE_STD_LIBS
	#define FAULT(x, ...) printf(x, ...);
#else
	#define FAULT(x, ...)
#endif

void MMU_CleanDCache(void);
void MMU_flushICache(void);

OS_Error elf_load(void * elfdata, void ** start_address)
{	
	INT32 i;
	
	// Validate the inputs
	if(!elfdata) 
	{
		FAULT("elf_load failed: address cannot be NULL\n");
		return ARGUMENT_ERROR;
	}

	Elf32_Ehdr *elf_hdr = (Elf32_Ehdr *) elfdata;
	
	// Validate the ELF file
	
	// 1. First check the target machine
	if(elf_hdr->e_machine != EM_ARM)
	{
		FAULT("elf_load failed: Target machine is not ARM\n");
		return INVALID_ELF_FILE;
	}

	// 2. Next check the identifier class
	if(elf_hdr->e_ident[EI_CLASS] != ELFCLASS32)
	{
		FAULT("elf_load failed: class is not ELFCLASS32\n");
		return INVALID_ELF_FILE;
	}

	// 3. Next check the endianness
	if(elf_hdr->e_ident[EI_DATA] != ELFDATA2LSB)
	{
		FAULT("elf_load failed: Data is not in Little Endian\n");
		return INVALID_ELF_FILE;
	}

	// Now step through each program header
	for(i = 0; i < elf_hdr->e_phnum; i++) 
	{
		Elf32_Phdr * elf_phhdr = (Elf32_Phdr *)((INT8*)elfdata + elf_hdr->e_phoff + (sizeof(Elf32_Phdr) * i));
	
		// If this section is not a Loadable segment, we need to ignore it.
		if(elf_phhdr->p_type != PT_LOAD)
			continue;
		
		// We need to load this segment at the target virtual address
		memcpy((void *)elf_phhdr->p_vaddr, (INT8*)elfdata + elf_phhdr->p_offset, elf_phhdr->p_memsz);
	}
	
	// TODO: Issue #19, Make selective cache flushing/cleaning in elf_load
	
	// DCache, ICache, and memory coherence is generally achieved by:
	// 		cleaning the DCache to ensure memory is up to date with all changes
	// 		invalidating the ICache to ensure that the ICache is forced to re-fetch instructions from memory.
	
	// Flush the data cache
	_OS_CleanInvalidateDCache(NULL, 0);	// Passing NULL, 0 invalidates the whole cache
	
	// Invalidate the Instruction Cache
	_OS_flushICache();

	// Update the start address
	if(start_address) {
		*start_address = elf_hdr->e_entry;
	}
	
	// The ELF file was loaded successfully
	return SUCCESS;
}
