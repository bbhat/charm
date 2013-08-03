// -------------------------------------------------------------------------------
// 
// 						Copyright 2012 xxxxxxx, xxxxxxx
// 	File:	S3C2440_OS_Context.S
// 	Author: Bala B. (bhat.balasubramanya// gmail.com)
// 	Description: OS Context Switch code
// 
// -------------------------------------------------------------------------------

SP_OFFSET_IN_TCB			=	12
OWNER_OFFSET_IN_TCB	=	16

SOLICITED_STACK_TYPE	=	1
INTERRUPT_STACK_TYPE	=	2
ZERO_CONTEXT_STACK_TYPE	=	3


NOINT  					= 	0xc0
I_Bit 					= 	0x80 	// IRQ bit

// -------------------------------------------------------------------------------
// SOLICITED STACK TYPE FRAME
// SOLICITED_STACK_TYPE
// -------------------------------------------------------------------------------

// -------------------------------------------------------------------------------
// INTERRUPT STACK TYPE FRAME
// INTERRUPT_STACK_TYPE
// -------------------------------------------------------------------------------

// -------------------------------------------------------------------------------
// ZERO_CONTEXT STACK TYPE FRAME
// SOLICITED_STACK_TYPE
// -------------------------------------------------------------------------------

// -------------------------------------------------------------------------------
// INTERRUPT
// -------------------------------------------------------------------------------
SRCPND   	=  0x4a000000    // Interrupt rest status
INTMOD   	=  0x4a000004    // Interrupt mode control
INTMSK   	=  0x4a000008    // Interrupt mask control
PRIORITY 	=  0x4a00000c    // IRQ priority control           <-- May 06, 2002 SOP
INTPND   	=  0x4a000010    // Interrupt request status
INTOFFSET	=  0x4a000014    // Interruot request source offset
SUSSRCPND	=  0x4a000018    // Sub source pending
INTSUBMSK	=  0x4a00001c    // Interrupt sub mask

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
INTMASK     = 0xc0

.global	_OS_BuildTaskStack
.global _OS_ContextRestore
.global _OS_ContextSw
.global	_OS_ReSchedule

.global	g_current_task
.global	g_current_process
.global	_SVC_STACK_TOP_
.global _jump_to_custom_IRQ_Handler

	.code 32             		// CODE32
	
// ----------------------------------------------------------------------------
// Build Initial stack for the thread
// ----------------------------------------------------------------------------
_OS_BuildTaskStack:		// _OS_BuildTaskStack(UINT32 * stack_ptr (R0), 
						// void (*task_function)(void *) (R1), void * arg(R2), bool system_task(r3));
	
	// TODO: Do you really need LR register as part of context? Note: This may be to help GDB show right callstack.
	
	stmfd	r0!, {r1}		// First store the task_function as the return address (PC)
	ldr		r1, =CompletedTaskTrap
	stmfd	r0!, {r1}		// Create space for Link Register (LR). Finished task should fall to CompletedTaskTrap
	stmfd	r0!, {r2}		// Store the argument as r0 value
	cmp		r3,	#0
	moveq	r1, #SVCMODE	// If system_task is true
	movne	r1, #USERMODE	// if system_task is false	// TODO: We should create in USERMODE after we have that ability
		
	stmfd	r0!, {r1}		// Create space for CPSR
	mov		r1, #ZERO_CONTEXT_STACK_TYPE
	stmfd	r0!, {r1}		// Store the stack type
	
	// Update the latest stack pointer in task`s TCB
	str		r0,[r2, #SP_OFFSET_IN_TCB]	// Update latest SP in the task TCB

	mov   	pc, lr


// ---------------------------------------------------------------------
// ISR(Interrupt Service Routine) IRQ
// The IRQ can come from SVC / USR / SYSTEM modes only
// ---------------------------------------------------------------------
	.global		_IRQ_TABLE_
	.global		_IRQHandler_
_IRQHandler_:
	
	// We need to switch back to old mode (SVC) in order to save context in its stack
	// But interrupt return address is in LR register of IRQ mode. This needs to be passed
	// to SVC mode.
	sub		lr, lr, #4			// Adjust IRQ LR to point to the right instruction
	
	stmfd	sp!, {r0-r3}		// Free some registers for passing arguments to SVC/SYS mode
	mov		r0, sp				// sp_irq in r0
	mrs		r1, spsr			// spsr_irq in r1	
	add		sp, sp, #4*4		// Reset IRQ stack (the data is still present in stack)
								// This data will be read after switching mode
	mrs		r3, cpsr			// Prepare for mode switch
	bic		r3, r3, #MODEMASK	// Clear the mode bits
	and		r2, r1, #MODEMASK	// Extract the old mode number
	cmp		r2, #SVCMODE
	orreq	r3, r3, #SVCMODE	// If old mode is SVC mode, switch to that mode
	orrne	r3, r3, #SYSMODE	// If it was USR/SYS mode go to SYS mode
	
	mov		r2, lr				// lr_irq in r2
	msr		cpsr_c, r3			// Finish mode switch
	
	// Welcome to the pre-interrupt mode
	stmfd	sp!, {r2}			// Store lr_irq
	stmfd	sp!, {lr}			// lr_<cur>
	
	stmfd	sp!, {r4-r12}		// Store some registers
	
	ldmfd	r0!, {r4-r7}		// Restore r0-r3 from the IRQ stack
	stmfd	sp!, {r4-r7}		// Store old r0-r3 which are in r4-r7
	
	stmfd	sp!, {r1}			// Save spsr_irq
	
	// Store the stack type
	mov		r0, #INTERRUPT_STACK_TYPE
	stmfd	sp!, {r0}	
	
	// Update the latest stack pointer in current task`s TCB
	// Also set g_current_task to NULL as we have fully saved the context
	ldr		r1,=g_current_process		
	mov		r0, #0
	str		r0, [r1]					// Reset the g_current_process to NULL
	ldr		r1,=g_current_task
	mov		r0, #0
	swp		r0,	r0, [r1]				// Exchange g_current_task value with register
	cmp		r0, #0
	strne	sp,[r0, #SP_OFFSET_IN_TCB]	// Update the task TCB if not NULL
	ldr		r1,=_SVC_STACK_TOP_
	streq	sp,[r1]						// Update the SVC Stack Top Pointer if task is NULL
	
	// Switch to SVC mode before calling User defined ISR
	bic		r3, r3, #MODEMASK
	orr		r3, r3, #SVCMODE
	msr		cpsr_c, r3					// Switch to SVC mode

	// Now that we don`t have a good stack to use, we should use the SVC stack
	ldr		sp,[r1]						// Load _SVC_STACK_TOP_

	// Now it is safe to jump to the user defined interrupt handler
	// Register r0 has the old g_current_task that will be passed to the called function
	b 		_jump_to_custom_IRQ_Handler


// ----------------------------------------------------------------------------
// Context Restore
// ----------------------------------------------------------------------------
	
	// Restore the new task (fall to below code)
_OS_ContextRestore:		// void C6713_ContextRestore(new_thread_pointer)

	// Disable Interrupt and retain the SVC mode
	mov		r1, #NOINT|SVCMODE
	msr		cpsr_c, r1 

_OS_ContextRestore_safe:
	
	// Set the g_current_process to the current threads owner
	ldr		r1, =g_current_process
	ldr		r2, [r0, #OWNER_OFFSET_IN_TCB]
	str		r2, [r1]				// Store the new thread`s owner into g_current_process

	// Move the current thread pointer into r1
	ldr		r1, =g_current_task
	str		r0, [r1]				// Store the new thread`s TCB pointer into g_current_task
		
	// Get the new stack pointer from the new task`s TCB
	ldr		r2, [r0, #SP_OFFSET_IN_TCB]
	
	// First Check the stack type before popping from the stack
	ldmfd	r2!, {r1}
	
	cmp		r1, #SOLICITED_STACK_TYPE
	beq		_OS_SolStackRestore

	cmp		r1, #INTERRUPT_STACK_TYPE
	beq		_OS_IntStackRestore

	cmp		r1, #ZERO_CONTEXT_STACK_TYPE
	beq		_OS_ZeroContextRestore

	// Possible stack corruption
_OS_StackTypeError:
	// TODO: Print some useful message for the stack corruption
	b	_OS_StackTypeError

// ----------------------------------------------------------------------------
// Solicited Stack Restore
// ----------------------------------------------------------------------------

_OS_SolStackRestore:

	ldmfd 	r2!, {r0}					// Pop CPSR
	msr 	spsr_cxsf, r0				// Save into SPSR
	orr		r0, r0, #NOINT
	msr		cpsr_c, r0					// Change the mode so that can set the SP of the new mode
	mov		sp, r2						// Set the SP for the new mode
	ldmfd 	sp!, {r0-r12, pc}^		// Return with CPSR update

// ----------------------------------------------------------------------------
// Interrupt Stack Restore
// ----------------------------------------------------------------------------

_OS_IntStackRestore:

	ldmfd 	r2!, {r0}					// Pop CPSR	
	msr 	spsr_cxsf, r0				// Save into SPSR
	orr		r0, r0, #NOINT
	msr		cpsr_c, r0					// Change the mode so that can set the SP of the new mode
	mov		sp, r2						// Set the SP for the new mode
	ldmfd 	sp!, {r0-r12, lr, pc}^		// Return with CPSR update

// ----------------------------------------------------------------------------
// Zero Context Restore
// ----------------------------------------------------------------------------
_OS_ZeroContextRestore:

	ldmfd 	r2!, {r0}					// Pop CPSR
	msr 	spsr_cxsf, r0				// Save into SPSR
	orr		r0, r0, #NOINT
	msr		cpsr_c, r0					// Change the mode so that can set the SP of the new mode
	mov		sp, r2						// Set the SP for the new mode
	ldmfd 	sp!, {r0, lr, pc}^			// Return with CPSR update

// -------------------------------------------------------------------------------
// Switches the context from one thread to another.
// The g_current_task is updated as part of this function
// to point to the new thread
// -------------------------------------------------------------------------------

_OS_ContextSw:	// void C6713_ContextSw(new_thread_pointer)

	// Disable Interrupt
	mrs		r1, CPSR		// Read CPSR into r1
	orr		r2, r1, #NOINT 
	msr		cpsr_c, r2

	// Save current thread context
	stmfd	sp!, {lr}		// save pc 
	stmfd	sp!, {r0-r12}	// save register file 
	stmfd	sp!, {r1}		// save current CPSR 
	
	// Store the stack type
	mov		r1, #SOLICITED_STACK_TYPE
	stmfd	sp!, {r1}		
	
	// Update the latest stack pointer in current task`s TCB or update _SVC_STACK_TOP_
	ldr		r1,=g_current_task
	ldr		r1,[r1]
	cmp 	r1, #0
	strne	sp,[r1, #SP_OFFSET_IN_TCB]
	ldr		r2,=_SVC_STACK_TOP_
	streq	sp,[r2]		// Update the SVC Stack Top Pointer if task is NULL
	
	// Restore the new task
	b		_OS_ContextRestore_safe

// -------------------------------------------------------------------------------
// A placeholder for tasks that have finished. 
// -------------------------------------------------------------------------------
CompletedTaskTrap:

	// TODO: Add support for thrashing finished tasks and its stack.
	// Currently it simply yields to other tasks

	bl	_OS_ReSchedule
	b	CompletedTaskTrap
	
	.LTORG   

.END
