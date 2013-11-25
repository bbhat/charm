///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	os_sem.h
//	Author: Balasubramanya Bhat (bhat.balasubramanya@gmail.com)
//	Description: Header file for the OS Semaphore APIs
//	
///////////////////////////////////////////////////////////////////////////////

#ifndef _OS_SEM_H
#define _OS_SEM_H

#include "os_types.h"
#include "os_queue.h"
#include "os_process.h"

typedef struct 
{
	UINT32 count;						
	OS_ProcessCB * owner;					// Owner process
	_OS_Queue periodic_task_queue;    		// Wait queue for periodic tasks
	_OS_Queue aperiodic_task_queue;  		// Wait queue for aperiodic tasks
    
} OS_SemaphoreCB;

OS_Error _OS_SemAlloc(OS_Sem *sem, UINT32 value);
OS_Error _OS_SemWait(OS_Sem sem);
OS_Error _OS_SemPost(OS_Sem sem);
OS_Error _OS_SemFree(OS_Sem sem);
OS_Error _OS_SemGetValue(OS_Sem sem, UINT32 *val);

#endif //_OS_SEM_H
