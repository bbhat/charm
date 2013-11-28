///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	os_process.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: OS Task Related routines
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_core.h"
#include "os_process.h"
#include "util.h"
#include "fs_api.h"
#include "elf_loader.h"
#include "mmu.h"

UINT16 g_process_id_counter;

OS_ProcessCB * g_process_list_head;
OS_ProcessCB * g_process_list_tail;

OS_ProcessCB * g_current_process;

// Placeholders for all the process control blocks
OS_ProcessCB	g_process_pool[MAX_PROCESS_COUNT];
UINT32 			g_process_usage_mask[(MAX_PROCESS_COUNT + 31) >> 5];

// Placeholders for Ramdisk file structures
FILE g_rdfile_pool[MAX_OPEN_FILES];
UINT32 g_rdfile_usage_mask[(MAX_OPEN_FILES + 31) >> 5];
	
#ifdef _USE_STD_LIBS
	#define FAULT(x, ...) printf(x, ...);
#else
	#define FAULT(x, ...)
#endif

#define MAX_LOADABLE_SECTIONS	16

OS_Error OS_CreateProcess(
	OS_Process *process,
	const INT8 *process_name,
	UINT16 attributes,
	void (*process_entry_function)(void *pdata),
	void *pdata)
{
	OS_ProcessCB *pcb;
	
	if(!process)
	{
	    FAULT("Invalid process");
		return INVALID_ARG;
	}

	if(!process_entry_function || !process_name)
	{
	    FAULT("OS_CreateProcess: invalid arguments");
		return INVALID_ARG;
	}
	
	// Now get a free PCB resource from the pool
	*process = (OS_Process) GetFreeResIndex(g_process_usage_mask, MAX_PROCESS_COUNT);
	if(*process < 0) 
	{
		FAULT("OS_CreateProcess failed for '%s': Exhausted all resources\n", g_current_process->name);
		return RESOURCE_EXHAUSTED;	
	}

	// Get a pointer to actual PCB
	pcb = &g_process_pool[*process];
	
	// Clear the process structure
	memset(pcb, 0, sizeof(OS_ProcessCB));
	
	// Copy process name
	strncpy(pcb->name, process_name, OS_PROCESS_NAME_SIZE - 1);
	pcb->name[OS_PROCESS_NAME_SIZE - 1] = '\0';
		
	pcb->id = *process;	// Assign a unique process id
	pcb->attributes = attributes;
	pcb->process_entry_function = process_entry_function;
	pcb->pdata = pdata;
	pcb->next = NULL;

#if ENABLE_MMU
	pcb->ptable = _MMU_allocate_l1_page_table();
	if(!pcb->ptable)
	{
		FAULT("OS_CreateProcess failed for '%s': No space in page table area\n", g_current_process->name);
		return RESOURCE_EXHAUSTED;	
	}

	// Now create a memory map for the task for the kernel space. Every process is going to have map for 
	// the whole kernel process. This makes it more efficient during system calls and interrupts
	// by eliminating the need to change the page table.
	// Note that this does not compromise the security as the user mode cannot read/write
	// anything in the kernel memory map
	_OS_create_kernel_memory_map(pcb->ptable);
#endif	

	// Block the process resource
	SetResourceStatus(g_process_usage_mask, *process, FALSE);
	
	// Add to the process list
	if(g_process_list_tail)
	{
		g_process_list_tail->next = pcb;
		g_process_list_tail = pcb;
	}
	else
	{
		g_process_list_head = g_process_list_tail = pcb;
	}
	
	return SUCCESS; 
}

OS_Error OS_CreateProcessFromFile(
		OS_Process *process,
		const INT8 * process_name,
		UINT16 attributes,
		const INT8 * exec_path,
		void *pdata
	)
{		
	// Validate inputs
	if(!exec_path)
	{
		FAULT("OS_CreateProcessFromFile: invalid file path");
		return INVALID_ARG;
	}
	
	// First read the elf file
	INT32 fd = ramdisk_open(exec_path, O_RDONLY);
	if(fd < 0)
	{
		FAULT("OS_CreateProcessFromFile: could not open '%s'", exec_path);
		return FILE_ERROR;
	}
	
	void * program = ramdisk_GetDataPtr(fd, NULL);
	if(!program)
	{
		FAULT("OS_CreateProcessFromFile: could not read '%s'", exec_path);
		return FILE_ERROR;		
	}

	// First read the section attributes from the ELF file so that we can create memory maps
	// before we load the sections into memory
	Elf_SectionAttribute sections[MAX_LOADABLE_SECTIONS];
	UINT32 scount = MAX_LOADABLE_SECTIONS;
	void * start_address = NULL;

	// Read the section attributes
	if(elf_get_sections(program, &start_address, sections, &scount) == SUCCESS)
	{
		if(scount > MAX_LOADABLE_SECTIONS) 
		{	
			// Not all sections were loaded. Generate an error
			return EXCEEDED_MAX_SECTIONS;
		}
	}
	
	// All sections can be loaded. 
	OS_Error status = OS_CreateProcess(process, process_name, attributes, start_address, pdata);
	if(status == SUCCESS)
	{
		// Get a pointer to actual PCB
		OS_ProcessCB *pcb = &g_process_pool[*process];
	
#if ENABLE_MMU
		// Create Memory Map for all sections in this program.
		// The kernel space has already been mapped to this process
		UINT32 i;
		for(i = 0; i < scount; i++)
		{
			_MMU_PTE_AccessPermission ap;
			switch(sections[i].flags)
			{
				case (PF_R):
					ap = KERNEL_RW_USER_RO;
					break;
				
				case (PF_X):
				case (PF_R | PF_X):
					ap = KERNEL_RW_USER_EX;
					break;
											
				case (PF_R | PF_W):
					ap = KERNEL_RW_USER_RW;
					break;
								
				default:
					ap = KERNEL_NA_USER_NA;
					break;				
			}

#if USER_PAGE_SIZE==4
			_MMU_add_l2_small_page_va_to_pa_map(pcb->ptable,
									sections[i].vaddr, sections[i].vaddr,
									sections[i].size, ap, 
									TRUE, TRUE);
#elif USER_PAGE_SIZE==64
			_MMU_add_l2_large_page_va_to_pa_map(pcb->ptable,
									sections[i].vaddr, sections[i].vaddr,
									sections[i].size, ap, 
									TRUE, TRUE);
#else
	#error "Supported values for USER_PAGE_SIZE is either 4 or 64"
#endif
		}
#endif // ENABLE_MMU

		// Now we are ready to actually load the program into memory
		status = elf_load(program);
	}
	
	return status;	
}
