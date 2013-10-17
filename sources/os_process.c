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

OS_Error elf_load(void * elfdata, void ** start_address);

OS_Error OS_CreateProcess(
	OS_Process *process,
	const INT8 *process_name,
	UINT16 attributes,
	void (*process_entry_function)(void *pdata),
	void *pdata)
{
	UINT32 intsts;
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
	
	pcb->attributes = attributes;
	pcb->process_entry_function = process_entry_function;
	pcb->pdata = pdata;
	pcb->next = NULL;
	
	OS_ENTER_CRITICAL(intsts); // Enter critical section
	
	pcb->id = ++g_process_id_counter;	// Assign a unique process id
	
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
	
	OS_EXIT_CRITICAL(intsts); 	// Exit the critical section
	
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
	
	// Load the executable file to memory
	void * start_address = NULL;
	OS_Error status = elf_load(program, &start_address);
	if(status == SUCCESS)
	{
		status = OS_CreateProcess(process, process_name, attributes, start_address, pdata);
	}

	return status;	
}
