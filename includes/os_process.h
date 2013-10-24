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
#include "os_config.h"
#include "os_task.h"
#include "fs_api.h"
#include "mmu.h"

///////////////////////////////////////////////////////////////////////////////
// Process PCB 
///////////////////////////////////////////////////////////////////////////////
typedef enum
{
	ADMIN_PROCESS = 1	
} OS_ProcessAttr;

typedef struct OS_ProcessCB
{
	INT8 name[OS_PROCESS_NAME_SIZE];
	void (*process_entry_function)(void *pdata);
	void * pdata;
	UINT16 id;
	UINT16 attributes;

#if ENABLE_MMU
	_MMU_L1_PageTable * ptable;
#endif

	// Pointer to next process in the list
	struct OS_ProcessCB *next;	
} OS_ProcessCB;

extern OS_ProcessCB * g_process_list_head;
extern OS_ProcessCB * g_process_list_tail;

extern OS_ProcessCB * g_current_process;
extern UINT16 g_process_id_counter;

extern OS_ProcessCB	* g_kernel_process;	// Kernel process

// Placeholders for all the process control blocks
extern OS_ProcessCB	g_process_pool[MAX_PROCESS_COUNT];
extern UINT32 g_process_usage_mask[];

extern FILE g_rdfile_pool[MAX_OPEN_FILES];
extern UINT32 g_rdfile_usage_mask[];
	
#endif // _OS_PROCESS_H
