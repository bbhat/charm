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
// Assembly routine that makes the system call which does not result in context switches
// OS_Error _OS_Syscall(const _OS_Syscall_Args * param_info, 
//						const void * arg, void * ret)
//---------------------------------------------------------------------
	.global _OS_Syscall	
_OS_Syscall:

	stmfd   sp!, {lr}  			    // Store the return address
	svc		0x0						// Call SVC 0x0 with parameter value in R0
	ldmfd   sp!, {pc}  			    // Restore registers and return

//---------------------------------------------------------------------
// Assembly routine that makes advanced system call which may result in context switches
// This is always safe to call
// OS_Error _OS_SyscallX(const _OS_Syscall_Args * param_info, 
//						const void * arg, void * ret)
//---------------------------------------------------------------------
	.global _OS_SyscallX
_OS_SyscallX:

	stmfd   sp!, {lr}  			    // Store the return address
	svc		0x1						// Call SVC 0x1 with parameter value in R0
	ldmfd   sp!, {pc}  			    // Restore registers and return
