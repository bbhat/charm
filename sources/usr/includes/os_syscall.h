///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	os_syscall.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Stuff related to System call
//	
///////////////////////////////////////////////////////////////////////////////

#ifndef OS_SYSCALL_H
#define OS_SYSCALL_H

enum
{
	SYSCALL_PERIODIC_TASK_CREATE = 0,
	SYSCALL_APERIODIC_TASK_CREATE,
	SYSCALL_PROCESS_CREATE,
	SYSCALL_PROCESS_CREATE_FROM_FILE,
	SYSCALL_SEM_ALLOC,
	SYSCALL_SEM_WAIT,
	SYSCALL_SEM_POST,
	SYSCALL_SEM_FREE,
	SYSCALL_SEM_GET_VALUE,
	SYSCALL_GET_CUR_TASK,
	SYSCALL_TASK_YIELD,
	SYSCALL_TASK_COMPLETE,
	SYSCALL_OS_GET_STAT,
	SYSCALL_TASK_GET_STAT,
	SYSCALL_GET_TASK_ALLOC_MASK,
	
	// Functions provided Kernel Drivers
	SYSCALL_DRIVER_STANDARD_CALL,           // Calling standard functions such as open/close/read/write
	SYCALL_DRIVER_CUSTOM_CALL,              // Calling custom functions. The sub_id indicates the custom function id
	
	SYSCALL_GET_CUR_PROC,
	SYSCALL_MAP_PHYSICAL_MEM,
	SYSCALL_UNMAP_MEM,
	
	// Reserved space for other syscall
	
	SYSCALL_PFM_LED_SET = 32,	

	SYSCALL_MAX_COUNT
};


enum    // Sub IDs for SYSCALL_DRIVER_STANDARD_CALL
{
    SUBCALL_DRIVER_LOOKUP = 0,
    SUBCALL_DRIVER_OPEN = 1,
    SUBCALL_DRIVER_CLOSE = 2,
    SUBCALL_DRIVER_READ = 3,
    SUBCALL_DRIVER_WRITE = 4,
    SUBCALL_DRIVER_CONFIGURE = 5
};

typedef enum 
{
	// Use SYSCALL_BASIC for basic system call. If the call does not result 
	// in context switch, then use basic call
	// Use SYSCALL_SWITCHING for Advanced system call such as those that result 
	// in switching context
	SYSCALL_BASIC = 0,
	SYSCALL_SWITCHING = 1
} Syscall_type;

typedef struct
{
	UINT16	id;
	UINT16	sub_id;
	UINT16	arg_count;
	UINT16	ret_count;
	UINT16	reserved[4];
	
} _OS_Syscall_Args;

// Main User mode to kernel mode entry syscall function
void _OS_Syscall(const _OS_Syscall_Args * param_info, const void * arg, void * ret, Syscall_type type);

#endif // OS_SYSCALL_H
