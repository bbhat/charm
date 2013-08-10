//------------------------------------------------------------------------------
//
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	os_syscall.S
//	Author: Bala B. (bhat.balasubramanya//gmail.com)
//	Description: Assembly routine that makes a system call
//
//-------------------------------------------------------------------------------

// -------------------------------------------------------------------------------
// Pre-defined constants
// -------------------------------------------------------------------------------
USERMODE    = 0x10
FIQMODE     = 0x11
IRQMODE     = 0x12
SVCMODE     = 0x13
ABORTMODE   = 0x17
UNDEFMODE   = 0x1b
SYSMODE     = 0x1f
MODEMASK    = 0x1f

SYSCALL_BASIC     = 0x0
SYSCALL_SWITCHING = 0x1

   	.section .text
   	.code 32

//---------------------------------------------------------------------
// Assembly routine that makes the basic system call
// OS_Error _OS_Syscall(const _OS_Syscall_Args * param_info, 
//						const void * arg, void * ret, Syscall_type type)
//---------------------------------------------------------------------
	.global _OS_Syscall
_OS_Syscall:

	stmfd   sp!, {lr}  			    // Store the return address	
	
	teq     r3, #SYSCALL_SWITCHING  // Check if we need switching syscall
	beq	    1f						// Call SVC 0x1 with parameter value in R0
	svcne	0x0						// Call SVC 0x0 with parameter value in R0
0:                                  // SYSCALL_BASIC
	mrs     r3, cpsr                // Check the current mode
	and     r3, #MODEMASK
	teq     r3, #SVCMODE            // Are we already in SVC mode?
	bleq    _BasicSWIHandler_       // If we are already in SVC, then make a simple function call
	svcne	0x0						// Call SVC 0x0 with parameter value in R0
	ldmfd   sp!, {pc}  			    // Restore registers and return
1:                                  // SYSCALL_SWITCHING
	mrs     r3, cpsr                // Check the current mode
	and     r3, #MODEMASK
	teq     r3, #SVCMODE            // Are we already in SVC mode?
	bleq    _AdvancedSWIHandler_    // If we are already in SVC, then make a simple function call
	svcne	0x1						// Call SVC 0x1 with parameter value in R0
	ldmfd   sp!, {pc}  			    // Restore registers and return
