///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	os_process.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: OS Process Related routines
//	
///////////////////////////////////////////////////////////////////////////////

#ifndef _OS_PROCESS_H
#define _OS_PROCESS_H

#include "os_types.h"

///////////////////////////////////////////////////////////////////////////////
// Process PCB 
///////////////////////////////////////////////////////////////////////////////
typedef struct OS_Process
{
	INT8 name[OS_PROCESS_NAME_SIZE];
	void (*process_entry_function)(void *pdata),
	void * pdata	
	
	// Pointer to next process in the list
	OS_Process *next;	
} OS_Process;

extern OS_Process * g_process_list_head;
extern OS_Process * g_process_list_tail;

extern OS_Process * g_current_process;

#endif // _OS_PROCESS_H