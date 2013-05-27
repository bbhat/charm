///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	os_process.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: OS Task Related routines
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_process.h"
#include "os_core.h"
#include "util.h"
#include "fs_api.h"

UINT16 g_process_id_counter;

OS_Process * g_process_list_head;
OS_Process * g_process_list_tail;

OS_Process * g_current_process;

#ifdef _USE_STD_LIBS
	#define FAULT(x, ...) printf(x, ...);
#else
	#define FAULT(x, ...)
#endif

OS_Error elf_load(void * elfdata, void ** start_address);

OS_Error OS_CreateProcess(
	OS_Process *process,
	const INT8 *process_name,
	void (*process_entry_function)(void *pdata),
	void *pdata)
{
	UINT32 intsts;
	
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
	
	// Clear the process structure
	memset(process, 0, sizeof(OS_Process));
	
	// Copy process name
	strncpy(process->name, process_name, OS_PROCESS_NAME_SIZE);
	
	process->process_entry_function = process_entry_function;
	process->pdata = pdata;
	process->next = NULL;
	
	OS_ENTER_CRITICAL(intsts); // Enter critical section
	
	process->id = ++g_process_id_counter;	// Assign a unique process id
	
	// Add to the process list
	if(g_process_list_tail)
	{
		g_process_list_tail->next = process;
		g_process_list_tail = process;
	}
	else
	{
		g_process_list_head = g_process_list_tail = process;
	}
	
	OS_EXIT_CRITICAL(intsts); 	// Exit the critical section
	
	return SUCCESS; 
}

OS_Error OS_CreateProcessFromFile(
		OS_Process *process,
		const INT8 * process_name,
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
		status = OS_CreateProcess(process, process_name, start_address, pdata);
	}

	return status;	
}
