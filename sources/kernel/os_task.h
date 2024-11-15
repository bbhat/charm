///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	os_task.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: OS Task Related routines
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_types.h"
#include "os_queue.h"

#ifndef _OS_TASK_H
#define _OS_TASK_H

#if OS_WITH_VALIDATE_TASK==1
#define TASK_SIGNATURE	0xFEEDBACC
#endif

///////////////////////////////////////////////////////////////////////////////
// Helper Macros for Task Types, Modes and associated functions
///////////////////////////////////////////////////////////////////////////////
enum 
{
	APERIODIC_TASK 		= 0,
	PERIODIC_TASK 		= 1,
	TASK_MODE_MASK		= 1,
	
	SYSTEM_TASK			= 2,
	USER_TASK			= 0,
	TASK_PRIVILEGE_MASK	= 2
};

#define IS_PERIODIC_TASK(task_attr)		(((task_attr) & TASK_MODE_MASK) == PERIODIC_TASK)
#define IS_APERIODIC_TASK(task_attr)	(((task_attr) & TASK_MODE_MASK) == APERIODIC_TASK)

#define IS_SYSTEM_TASK(task_attr)	(((task_attr) & TASK_PRIVILEGE_MASK) == SYSTEM_TASK)
#define IS_USER_TASK(task_attr)		(((task_attr) & TASK_PRIVILEGE_MASK) == USER_TASK)

typedef _OS_HybridQNode	_OS_TaskQNode;

///////////////////////////////////////////////////////////////////////////////
// Task TCB 
///////////////////////////////////////////////////////////////////////////////
union OS_Task;
typedef union OS_Task
{
	struct
	{
		// Attribute for maintaining a list/queue of threads.
		// IMPORTANT: Make sure that the Queue node is the first element in this structure
		_OS_HybridQNode qp;

		// Folliwing attributes are common in both type of tasks. They should be in the same order
		UINT32 *top_of_stack;	// Do NOT REORDER THIS MEMBER, THE OFFSET 'SP_OFFSET_IN_TCB' IS USED IN Assembly
		struct OS_Process *owner_process;	// Do NOT REORDER THIS MEMBER, THE OFFSET 'OWNER_OFFSET_IN_TCB' IS USED IN Assembly

		UINT32 *stack;
		UINT32 stack_size;
		void (*task_function)(void *pdata);
		void *pdata;

		UINT32 *syscall_result;		// Some syscalls put the thread to wait. This member holds a pointer to its return argiments
		UINT64 accumulated_budget;

		UINT16 attributes;
		UINT16 id;
		INT8 name[OS_TASK_NAME_SIZE];
	#if OS_WITH_VALIDATE_TASK==1
		UINT32 signature;		// A unique identifier to validate the task structure
	#endif	// OS_WITH_VALIDATE_TASK
	
	} __attribute__ ((packed));
	
	struct
	{
		// Attribute for maintaining a list/queue of threads.
		// IMPORTANT: Make sure that the Queue node is the first element in this structure
		_OS_HybridQNode qp;

		UINT32 *top_of_stack;	// Do NOT REORDER THIS MEMBER, THE OFFSET 'SP_OFFSET_IN_TCB' IS USED IN ASSEMBLY
		struct OS_Process *owner_process;	// Do NOT REORDER THIS MEMBER, THE OFFSET 'OWNER_OFFSET_IN_TCB' IS USED IN Assembly

		UINT32 *stack;
		UINT32 stack_size;
		void (*task_function)(void *pdata);
		void *pdata;
	
		void *syscall_result;		// Some syscalls put the thread to wait. This member holds a pointer to its return argiments
		UINT64 accumulated_budget;

		// Folliwing attributes are common in both type of tasks. They should be in the same order.
		UINT16 attributes;
		UINT16 id;
		INT8 name[OS_TASK_NAME_SIZE];
	#if OS_WITH_VALIDATE_TASK==1
		UINT32 signature;		// A unique identifier to validate the task structure
	#endif	// OS_WITH_VALIDATE_TASK
	
		// Following arguments are specific to Periodic task
		UINT32 period;
		UINT32 deadline;
		UINT32 budget;
		UINT32 phase;
	
		// Following members are used by the scheduling algorithm
		UINT32 remaining_budget;
	
		// When the task is in the wait_queue, it will have its job_release_time as the next release time
		// When it is in the ready queue, it is the task period beginning value for the current period
		UINT64 job_release_time;
		UINT32 exec_count;
		UINT32 TBE_count;
		UINT32 dline_miss_count;
		
	} __attribute__ ((packed)) p;

	struct OS_AperiodicTask
	{
		// Attribute for maintaining a list/queue of threads.
		// IMPORTANT: Make sure that the Queue node is the first element in this structure
		_OS_HybridQNode qp;

		// Folliwing attributes are common in both type of tasks. They should be in the same order
		UINT32 *top_of_stack;	// Do NOT REORDER THIS MEMBER, THE OFFSET 'SP_OFFSET_IN_TCB' IS USED IN Assembly
		struct OS_Process *owner_process;	// Do NOT REORDER THIS MEMBER, THE OFFSET 'OWNER_OFFSET_IN_TCB' IS USED IN Assembly

		UINT32 *stack;
		UINT32 stack_size;
		void (*task_function)(void *pdata);
		void *pdata;
	
		void *syscall_result;		// Some syscalls put the thread to wait. This member holds a pointer to its return argiments
		UINT64 accumulated_budget;

		UINT16 attributes;
		UINT16 id;
		INT8 name[OS_TASK_NAME_SIZE];
	#if OS_WITH_VALIDATE_TASK==1
		UINT32 signature;		// A unique identifier to validate the task structure
	#endif	// OS_WITH_VALIDATE_TASK
	
	} __attribute__ ((packed)) ap;

} OS_Task;

OS_Return _OS_CreatePeriodicTask(
	UINT32 period_in_us,
	UINT32 deadline_in_us,
	UINT32 budget_in_us,
	UINT32 phase_shift_in_us,
	UINT32 *stack,
	UINT32 stack_size_in_bytes,
	const INT8 * task_name,
	UINT16 options,
	OS_Task_t *task,
	void (*periodic_entry_function)(void *pdata),
	void *pdata);

OS_Return _OS_CreateAperiodicTask(UINT16 priority, 
	UINT32 * stack, 
	UINT32 stack_size_in_bytes,
	const INT8 * task_name,
	UINT16 options,
	OS_Task_t * task,
	void(* task_entry_function)(void * pdata),
	void * pdata);

OS_Return _OS_GetTaskAllocMask(UINT32 * alloc_mask, UINT32 count, UINT32 starting_task);

// Function to be called when an Aperiodic task finishes so that it is no more included
// in scheduling. Only Aperiodic tasks are allowed to complete
OS_Return _OS_CompleteAperiodicTask();

// Placeholders for all the process control blocks
extern OS_Task	g_task_pool[MAX_TASK_COUNT];
extern UINT32 	g_task_usage_mask[];

#define	alarm_time()	qp.key
#define	priority()		qp.key

#endif // _OS_TASK_H
