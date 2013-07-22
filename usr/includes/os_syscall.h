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
	SYSCALL_CREATE_PERIODIC_TASK = 10000,
	SYSCALL_CREATE_APERIODIC_TASK,
	SYSCALL_CREATE_PROCESS,
	SYSCALL_CREATE_PROCESS_FROM_FILE,
	SYSCALL_CREATE_SEM_CREATE,
	SYSCALL_CREATE_SEM_WAIT,
	SYSCALL_CREATE_SEM_POST,
	SYSCALL_CREATE_SEM_DESTROY,
	SYSCALL_CREATE_SEM_GET_VALUE,
	SYSCALL_CREATE_MUTEX_CREATE,
	SYSCALL_CREATE_MUTEX_LOCK,
	SYSCALL_CREATE_MUTEX_UNLOCK,
	SYSCALL_CREATE_MUTEX_DESTROY,
	SYSCALL_CREATE_GET_CUR_TASK	
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
OS_Error _OS_Syscall(const _OS_Syscall_Args * param_info, void * arg, void * ret, void * reserved);

#endif // OS_SYSCALL_H
