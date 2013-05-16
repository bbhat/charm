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

UINT16 g_process_id_counter;

OS_Process * g_process_list_head;
OS_Process * g_process_list_tail;

OS_Process * g_current_process;

#ifdef _USE_STD_LIBS
	#define FAULT(x, ...) printf(x, ...);
#else
	#define FAULT(x, ...)
#endif

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

	if(!process_entry_function)
	{
	    FAULT("One or more invalid %s arguments", "process");
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
	UINT32 intsts;
	
	if(!process)
	{
	    FAULT("Invalid process");
		return INVALID_ARG;
	}

	if(!exec_path)
	{
	    FAULT("One or more invalid %s arguments", "exec_path");
		return INVALID_ARG;
	}
	
	// Clear the process structure
	memset(process, 0, sizeof(OS_Process));
		
	// Copy process name
	strncpy(process->name, process_name, OS_PROCESS_NAME_SIZE);


	return SUCCESS;	
}
