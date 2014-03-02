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

#include "os_core.h"
#include "os_types.h"
#include "os_queue.h"
#include "os_process.h"

typedef struct 
{
	UINT32 count;
	UINT32 attributes;
	OS_Process * owner;					// Owner process
	_OS_Queue periodic_wait_queue;    		// Wait queue for periodic tasks
	_OS_Queue aperiodic_wait_queue;  		// Wait queue for aperiodic tasks
    
} OS_SemaphoreCB;

extern OS_SemaphoreCB g_semaphore_pool[MAX_SEMAPHORE_COUNT];

///////////////////////////////////////////////////////////////////////////////
//
// The semaphores in this RTOS is specifically adapted to the periodic EDF scheduler.
// If there are multiple tasks waiting for a semaphore, periodic tasks get the
// first preference (provided there is a job waiting) when the semaphore becomes 
// available followed by aperiodic tasks.
// Within the periodic tasks, a task with earliest approaching deadline is given
// priority. This does not necessarily mean a task with the smallest period. 
// This semaphore behavior ideal for use with an EDF scheduler
// The periodic tasks are given preference in the order of their priority.
//
///////////////////////////////////////////////////////////////////////////////

OS_Return _OS_SemAlloc(OS_Sem_t *sem, UINT32 value, BOOL binary);
OS_Return _OS_SemWait(OS_Sem_t sem);
OS_Return _OS_SemPost(OS_Sem_t sem);
OS_Return _OS_SemFree(OS_Sem_t sem);
OS_Return _OS_SemGetValue(OS_Sem_t sem, UINT32 *val);

#endif //_OS_SEM_H
