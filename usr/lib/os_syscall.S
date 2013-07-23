//------------------------------------------------------------------------------
//
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	os_syscall.S
//	Author: Bala B. (bhat.balasubramanya//gmail.com)
//	Description: Assembly routine that makes a system call
//
//-------------------------------------------------------------------------------


   	.section .text
   	.code 32

//---------------------------------------------------------------------
// Assembly routine that makes the system call
// OS_Error _OS_Syscall(const _OS_Syscall_Args * param_info, 
//						void * arg, void * ret, void * reserved)
//---------------------------------------------------------------------
	.global _OS_Syscall	
_OS_Syscall:
	
	svc		0x0			// Call SVC 0x0 with parameter value in R0
	
	mov		r0, #0		// Return Success
	mov		pc, lr
