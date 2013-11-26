///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	elf.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Utility functions for handling ELF files
//	
///////////////////////////////////////////////////////////////////////////////

#include "elf_loader.h"

#ifdef _USE_STD_LIBS
	#define FAULT(x, ...) printf(x, ...);
#else
	#define FAULT(x, ...)
#endif

OS_Error elf_get_sections(void * elfdata, void ** start_address, 
							Elf_SectionAttribute * sections, UINT32 * count)
{	
	UINT32 scount = 0;
	UINT32 i;
	
	// Validate the inputs
	if(!elfdata) 
	{
		FAULT("elf_get_sections failed: address cannot be NULL\n");
		return ARGUMENT_ERROR;
	}
	
	// If 'sections' argument is provided without the count, it is an error.
	if(sections && !count)
	{
		FAULT("elf_get_sections failed: count cannot be NULL\n");
		return ARGUMENT_ERROR;
	}

	Elf32_Ehdr *elf_hdr = (Elf32_Ehdr *) elfdata;
	
	// Validate the ELF file
	
	// 1. First check the target machine
	if(elf_hdr->e_machine != EM_ARM)
	{
		FAULT("elf_get_sections failed: Target machine is not ARM\n");
		return INVALID_ELF_FILE;
	}

	// 2. Next check the identifier class
	if(elf_hdr->e_ident[EI_CLASS] != ELFCLASS32)
	{
		FAULT("elf_get_sections failed: class is not ELFCLASS32\n");
		return INVALID_ELF_FILE;
	}

	// 3. Next check the endianness
	if(elf_hdr->e_ident[EI_DATA] != ELFDATA2LSB)
	{
		FAULT("elf_get_sections failed: Data is not in Little Endian\n");
		return INVALID_ELF_FILE;
	}

	// Now step through each program header
	for(i = 0; i < elf_hdr->e_phnum; i++) 
	{
		Elf32_Phdr * elf_phhdr = (Elf32_Phdr *)((INT8*)elfdata + elf_hdr->e_phoff + (sizeof(Elf32_Phdr) * i));

		// If this section is not a Loadable segment, we need to ignore it.
		if(elf_phhdr->p_type != PT_LOAD)
			continue;
		
		// Update the 'sections' array
		if(sections && (scount < *count)) 
		{
			sections[scount].vaddr = (VADDR) elf_phhdr->p_vaddr;
			sections[scount].size = elf_phhdr->p_memsz;
			sections[scount].flags = elf_phhdr->p_flags;
			sections[scount].align = elf_phhdr->p_align;			
		}
		
		// Update the section count
		scount++;
	}

	// Update the start address
	if(start_address) 
	{
		*start_address = (void *)elf_hdr->e_entry;
	}
	
	// Update count if requested
	if(count) 
	{
		*count = scount;
	}
	
	// The ELF file was loaded successfully
	return SUCCESS;
}

OS_Error elf_load(void * elfdata)
{	
	UINT32 i;
	
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
		
#if ENABLE_DATA_CACHE==1	
	// Flush the data cache
	_OS_CleanInvalidateDCache();	// Invalidate the whole cache
#endif
	
#if ENABLE_INSTRUCTION_CACHE==1	
	// Invalidate the Instruction Cache
	_OS_InvalidateICache();
#endif

	// The ELF file was loaded successfully
	return SUCCESS;
}
