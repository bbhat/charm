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

#include "os_types.h"

enum
{
	SYSCALL_PERIODIC_TASK_CREATE = 0,
	SYSCALL_APERIODIC_TASK_CREATE,
	SYSCALL_PROCESS_CREATE,
	SYSCALL_PROCESS_CREATE_FROM_FILE,
	SYSCALL_SEM_CREATE,
	SYSCALL_SEM_WAIT,
	SYSCALL_SEM_POST,
	SYSCALL_SEM_DESTROY,
	SYSCALL_SEM_GET_VALUE,
	SYSCALL_MUTEX_CREATE,
	SYSCALL_MUTEX_LOCK,
	SYSCALL_MUTEX_UNLOCK,
	SYSCALL_MUTEX_DESTROY,
	SYSCALL_GET_CUR_TASK,
	SYSCALL_TASK_YIELD,
	
	// Reserved space for other syscall
	
	SYSCALL_PFM_LED_SET = 32,	

	SYSCALL_MAX_COUNT
};

enum
{
	SYSCALL_VER_1_0,
	SYSCALL_VER_1_1,
	SYSCALL_VER_1_2,
	SYSCALL_VER_1_3,
	SYSCALL_VER_1_4,
};

typedef struct
{
	UINT16	id;
	UINT16	version;
	UINT16	arg_bytes;
	UINT16	ret_bytes;
	UINT16	reserved[4];
	
} _OS_Syscall_Args;


// Main User mode to kernel mode entry syscall function
OS_Error _OS_Syscall(const _OS_Syscall_Args * param_info, const void * arg, void * ret);

#endif // OS_SYSCALL_H
