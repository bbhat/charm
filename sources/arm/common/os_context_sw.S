// -------------------------------------------------------------------------------
// 
//                         Copyright 2012 xxxxxxx, xxxxxxx
//     File:    S3C2440_OS_Context.S
//     Author: Bala B. (bhat.balasubramanya// gmail.com)
//     Description: OS Context Switch code
// 
// -------------------------------------------------------------------------------

SP_OFFSET_IN_TCB        = 24
OWNER_OFFSET_IN_TCB     = 28
FUNCTION_OFFSET_IN_TCB  = 40
PDATA_OFFSET_IN_TCB     = 44

SOLICITED_STACK_TYPE    = 1
INTERRUPT_STACK_TYPE    = 2
ZERO_CONTEXT_STACK_TYPE = 3

NOINT                   = 0xc0
I_Bit                   = 0x80     // IRQ bit

BASIC_SWI       = 0
ADVANCED_SWI    = 1

// -------------------------------------------------------------------------------
// INTERRUPT
// -------------------------------------------------------------------------------
SRCPND       =  0x4a000000    // Interrupt rest status
INTMOD       =  0x4a000004    // Interrupt mode control
INTMSK       =  0x4a000008    // Interrupt mask control
PRIORITY     =  0x4a00000c    // IRQ priority control           <-- May 06, 2002 SOP
INTPND       =  0x4a000010    // Interrupt request status
INTOFFSET    =  0x4a000014    // Interruot request source offset
SUSSRCPND    =  0x4a000018    // Sub source pending
INTSUBMSK    =  0x4a00001c    // Interrupt sub mask

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

.global     _OS_BuildKernelTaskStack
.global     _OS_BuildUserTaskStack
.global     _OS_ContextRestore
.global     _OS_Schedule
.global		_OS_KernelSyscall
.global     panic

.global     g_current_task
.global     g_current_process
.global     _SVC_STACK_TOP_
.global     _jump_to_custom_IRQ_Handler

    .code 32            // CODE32
    
// ----------------------------------------------------------------------------
// Build Initial stack for the kernel thread
// ----------------------------------------------------------------------------
_OS_BuildKernelTaskStack:     // _OS_BuildKernelTaskStack(UINT32 * stack_ptr (R0), 
                              // void (*task_function)(void *) (R1), void * arg(R2));
    
    // TODO: Do you really need LR register as part of context? Note: This may be to help GDB show right callstack.
    stmfd    r0!, {r1}        // First store the task_function as the return address (PC)
    ldr      r1, =CompletedTaskTrap
    stmfd    r0!, {r1}        // Create space for Link Register (LR). Finished task should fall to CompletedTaskTrap
    add      r1, r0, #8
    stmfd    r0!, {r1}        // Store SP value
    mov      r3, #0
    stmfd    r0!, {r3}        // Store FP value with 0 so that backtrace looks clean
    stmfd    r0!, {r2}        // Create a dummy slot for r1 (this is used for user task stack)
    stmfd    r0!, {r2}        // Store the argument as r0 value    
    mov      r1, #SVCMODE     // System task
    stmfd    r0!, {r1}        // Create space for CPSR
    mov      r1, #ZERO_CONTEXT_STACK_TYPE
    stmfd    r0!, {r1}        // Store the stack type
    
    // Update the latest stack pointer in task`s TCB
    str      r0,[r2, #SP_OFFSET_IN_TCB]    // Update latest SP in the task TCB

    mov      pc, lr


// ----------------------------------------------------------------------------
// Build Initial stack for the user thread
// ----------------------------------------------------------------------------
_OS_BuildUserTaskStack:       // _OS_BuildUserTaskStack(UINT32 * stack_ptr (R0), 
                              // void (*task_function)(void *) (R1), void * arg(R2));
    
    // TODO: Do you really need LR register as part of context? Note: This may be to help GDB show right callstack.
    
    stmfd    r0!, {r1}        // First store the task_function as the return address (PC)
    ldr      r1, =CompletedTaskTrap
    stmfd    r0!, {r1}        // Create space for Link Register (LR). Finished task should fall to CompletedTaskTrap
    add      r1, r0, #8
    stmfd    r0!, {r1}        // Store SP value
    mov      r3, #0
    stmfd    r0!, {r3}        // Store FP value with 0 so that backtrace looks clean
    ldr      r1, [r2, #PDATA_OFFSET_IN_TCB]    // Get the PDATA offset in the TCB
    stmfd    r0!, {r1}        // Store the argument as r1 value
    ldr      r1, [r2, #FUNCTION_OFFSET_IN_TCB] // Get the FUNCTION offset in the TCB
    stmfd    r0!, {r1}        // Store the argument as r0 value    
    mov      r1, #USERMODE    // User task
    stmfd    r0!, {r1}        // Create space for CPSR
    mov      r1, #ZERO_CONTEXT_STACK_TYPE
    stmfd    r0!, {r1}        // Store the stack type
    
    // Update the latest stack pointer in task`s TCB
    str      r0,[r2, #SP_OFFSET_IN_TCB]    // Update latest SP in the task TCB

    mov      pc, lr


// ---------------------------------------------------------------------
// ISR(Interrupt Service Routine) IRQ
// The IRQ can come from SVC / USR / SYSTEM modes only
// ---------------------------------------------------------------------
    .global        _IRQHandler_
_IRQHandler_:
    
    // We need to switch back to old mode (SVC/SYS) in order to save context in its stack
    // But interrupt return address is in LR register of IRQ mode. This needs to be passed
    // to SVC/SYS mode.
    sub     lr, lr, #4          // Adjust IRQ LR to point to the right instruction
    
    stmfd   sp!, {r0-r3}        // Free some registers for passing arguments to SVC/SYS mode
    mov     r0, sp              // sp_irq in r0
    mrs     r1, spsr            // spsr_irq in r1    
    add     sp, sp, #4*4        // Reset IRQ stack (the data is still present in stack)
                                // This data will be read after switching mode
    mrs     r3, cpsr            // Prepare for mode switch
    bic     r3, r3, #MODEMASK   // Clear the mode bits
    and     r2, r1, #MODEMASK   // Extract the old mode number
    teq     r2, #SVCMODE
    orreq   r3, r3, #SVCMODE    // If old mode is SVC mode, switch to that mode
    orrne   r3, r3, #SYSMODE    // If it was USR/SYS mode go to SYS mode
    
    mov     r2, lr              // lr_irq in r2
    msr     cpsr_c, r3          // Finish mode switch
    
    // Welcome to the pre-interrupt mode
    stmfd   sp!, {r2}           // Store lr_irq
    stmfd   sp!, {lr}           // lr_<cur>
    
    add     r2, sp, #8          // While restoring context we need to simultaneously restore the SP of the new mode
    stmfd   sp!, {r2}           // Store SP as the bottom of Interrupt stack frame
    
    stmfd   sp!, {r4-r12}       // Store some registers
    
    ldmfd   r0!, {r4-r7}        // Restore r0-r3 from the IRQ stack
    stmfd   sp!, {r4-r7}        // Store old r0-r3 which are in r4-r7
    
    stmfd   sp!, {r1}           // Save spsr_irq
    
    // Store the stack type
    mov     r0, #INTERRUPT_STACK_TYPE
    stmfd   sp!, {r0}    
    
    // Update the latest stack pointer in current task`s TCB
    // Also set g_current_task to NULL as we have fully saved the context
    ldr     r1,=g_current_process        
    mov     r0, #0
    str     r0, [r1]                      // Reset the g_current_process to NULL
    ldr     r1,=g_current_task
    mov     r0, #0
    swp     r0, r0, [r1]                  // Exchange g_current_task value with register
    cmp     r0, #0
    strne   sp,[r0, #SP_OFFSET_IN_TCB]    // Update the task TCB if not NULL
    ldr     r1,=_SVC_STACK_TOP_
    streq   sp,[r1]                       // Update the SVC Stack Top Pointer if task is NULL
    
    // Switch to SVC mode before calling User defined ISR
    bic     r3, r3, #MODEMASK
    orr     r3, r3, #SVCMODE
    msr     cpsr_c, r3                    // Switch to SVC mode

    // Now that we don`t have a good stack to use, we should use the SVC stack
    ldr     sp,[r1]                        // Load _SVC_STACK_TOP_

    // Now it is safe to jump to the user defined interrupt handler
    // Register r0 has the old g_current_task that will be passed to the called function
    b       _jump_to_custom_IRQ_Handler


// ----------------------------------------------------------------------------
// Main SWI Handler
// ----------------------------------------------------------------------------
	.global _SWIHandler_
_SWIHandler_:

	// There are two types of SWI handlers. 
	// 1. One that returns from SWI handler to the caller thread. This does not result in context switch
	// 2. Ones that results in context switch. This does not return from SWI handler using the same path
	// Both are handled separately. 
	// The first method can be handled efficiently by using the SVC stack and storing only few registers
	// The second method needs to be handled more comprehensively by storing context in the caller thread
	// just like IRQ handler

    ldr		r3, [lr,#-4]           // Load the instruction word and...
    bic   	r3, r3, #0xff000000    // ...extract comment field
	teq     r3, #ADVANCED_SWI
	beq     _AdvancedSWIHandler_    
	
	// Fall to basic SWI handler

// ----------------------------------------------------------------------------
// Basic SWI Handler
// ----------------------------------------------------------------------------
    .global _BasicSWIHandler_
_BasicSWIHandler_:

    mrs     r3, spsr               // Note that r3 is free (reserved in syscall)
    and     r3, #MODEMASK
    teq     r3, #SVCMODE
    beq     1f     

	// Basic SWI Handling. Called for only those system services that do not involve context switch
    // The SWI is always taken in SVC mode. Lets make sure we start with a known good stack
	ldr     r3, =_SVC_STACK_TOP_   // Note that r3 is free (reserved in syscall)
	ldr     sp, [r3]
1:	
	stmfd   sp!, {lr}  			   // Store registers
	mrs     r3, spsr               // Get spsr into r3. Note that r3 is reserved in syscall
	stmfd   sp!, {r3}              // Store spsr onto stack. Only required for nested SVCs.
	
    // r0 - r2 now contains syscall parameters        
    bl      _OS_KernelSyscall      // Call main part of handler
    
    // Returning from basic SWI handler
    ldmfd   sp!, {r3}          	   // Get spsr from stack
    msr     spsr, r3               // Restore spsr
    ldmfd   sp!, {pc}^ 			   // Restore registers and return	

// ----------------------------------------------------------------------------
// Advanced SWI Handler
// ----------------------------------------------------------------------------
    .global _AdvancedSWIHandler_
_AdvancedSWIHandler_:
    
    mrs     r3, spsr
    and     r3, #MODEMASK
    teq     r3, #SVCMODE
    beq     _KernelSWIHandler_     // Note that r3 is free (reserved in syscall)
    
    // If the syscall is from Userland, fall through
	// The SWI is always taken in SVC mode. SWI is called only from SYS or USER mode.
	// Lets make sure we start with a known good stack
	ldr     r3, =_SVC_STACK_TOP_   
	ldr     r3, [r3]
	
	stm     r3, {sp}^              // Get the user mode SP
	ldm     r3, {sp}               // Set as the new SP in SVC mode
	
    stmfd   sp!, {lr}              // Save PC value to be used for return
    
    add     r3, sp, #4             // We need to store the SP to use when we restore the stack
    stmfd   sp!, {r3}              // Store SP as the bottom of solicited STACK
    
    stmfd   sp!, {r4-r12}          // save register file 
    
    mrs     r3, spsr
    stmfd   sp!, {r3}              // save current SPSR
    
    // Store the stack type
    mov     r3, #SOLICITED_STACK_TYPE
    stmfd   sp!, {r3}        
    
    // Update the latest stack pointer in current task`s TCB or update _SVC_STACK_TOP_
    ldr     r3,=g_current_task
    ldr     r3,[r3]
    str     sp,[r3, #SP_OFFSET_IN_TCB]

	ldr     r3, =_SVC_STACK_TOP_    // Set SP to SVC mode stack again
	ldr     sp, [r3]
    
                                    // r0 - r2 contains syscall parameters    
    bl      _OS_KernelSyscall		// Call main part of handler
	
    b       _OS_Schedule            // Call reschedule (will reach here if there is a syscall error 
    								// or if the syscall does not call _OS_Schedule)
    
// -------------------------------------------------------------------------------
// Stores the context of the current task. After storing the context, it switches 
// to the SVC stack. 
// -------------------------------------------------------------------------------

_KernelSWIHandler_:

    // Save current thread context
    stmfd   sp!, {lr}        // save pc 
    
    add     r3, sp, #4       // We need to store the SP to use when we restore the stack
    stmfd   sp!, {r3}        // Store SP as the bottom of solicited STACK
    
    stmfd   sp!, {r4-r12}    // save register file 
    mrs     r3, spsr
    stmfd   sp!, {r3}        // save current SPSR
    
    // Store the stack type
    mov     r3, #SOLICITED_STACK_TYPE
    stmfd   sp!, {r3}
    
    // Update the latest stack pointer in current task`s TCB or update _SVC_STACK_TOP_
    ldr     r3,=g_current_task
    ldr     r3,[r3]
    cmp     r3, #0
    strne   sp,[r3, #SP_OFFSET_IN_TCB]
    
    ldr     r3, =_SVC_STACK_TOP_    // Set SP to base of SVC mode stack
	ldr     sp, [r3]

                                    // r0 - r2 contains syscall parameters    
    bl      _OS_KernelSyscall		// Call main part of handler
	
    b       _OS_Schedule            // Call reschedule (will reach here if there is a syscall error)
    
// ----------------------------------------------------------------------------
// Context Restore
// ----------------------------------------------------------------------------
    
    // Restore the new task (fall to below code)
_OS_ContextRestore:        // void _OS_ContextRestore(new_thread_pointer)

    // Disable Interrupt and retain the SVC mode
    msr     cpsr_c, #NOINT|SVCMODE

_OS_ContextRestore_safe:
    
    // Set the g_current_process to the new threads owner
    ldr     r1, =g_current_process
    ldr     r2, [r0, #OWNER_OFFSET_IN_TCB]
    str     r2, [r1]                // Store the new thread`s owner into g_current_process

    // Move the current thread pointer into r1
    ldr     r1, =g_current_task
    str     r0, [r1]                // Store the new thread`s TCB pointer into g_current_task
        
    // Get the new stack pointer from the new task`s TCB
    // We can destory the SVC mode stack as we are not going to return to the caller
    ldr     sp, [r0, #SP_OFFSET_IN_TCB]
    
    // First Check the stack type before popping from the stack
    ldmfd   sp!, {r1}
    
    cmp     r1, #SOLICITED_STACK_TYPE
    beq     _OS_SolStackRestore

    cmp     r1, #INTERRUPT_STACK_TYPE
    beq     _OS_IntStackRestore

    cmp     r1, #ZERO_CONTEXT_STACK_TYPE
    beq     _OS_ZeroContextRestore

    // Possible stack corruption
_invalid_stack_type:
    ldr     r0, =_stack_corrupted_string
    bl      panic

// ----------------------------------------------------------------------------
// Solicited Stack Restore
// ----------------------------------------------------------------------------

_OS_SolStackRestore:

    ldmfd   sp!, {r0}                     // Pop CPSR
    msr     spsr, r0                      // Save into SPSR. This should follow CPSR setting to use the new mode
    and     r0, r0, #MODEMASK             // Extract the mode bits
    teq     r0, #SVCMODE                  // Check if we are returning to SVC MODEMASK
	beq     1f
	
    // If we are going to USER/SYS mode, then we have special method to restore user registers
    // Read section B6.1.3 of DDI0406B Arm Architecture Reference Manual
    ldmfd   sp, {r4-r12, sp}^             // You cant use base register writeback
    add     sp, sp, #4*10                 // Discard above values from stack
    ldmfd   sp, {pc}^                     // Return with CPSR update
1:
    // Method to return to SVC mode is different than USER/SYS Mode
    ldmfd   sp, {r4-r12, sp, pc}^         // Return with CPSR update

// ----------------------------------------------------------------------------
// Interrupt Stack Restore
// ----------------------------------------------------------------------------

_OS_IntStackRestore:
    ldmfd   sp!, {r0}                     // Pop CPSR
    msr     spsr, r0                      // Restore SPSR.
    and     r0, r0, #MODEMASK             // Extract the mode bits
    teq     r0, #SVCMODE                  // Check if we are returning to SVC MODEMASK
    beq     1f

    // If we are going to USER/SYS mode, then we have special method to restore user registers
    // Read section B6.1.3 of DDI0406B Arm Architecture Reference Manual
    ldmfd   sp, {r0-r12, sp, lr}^         // You cant use base register writeback
    add     sp, sp, #4*15                 // Discard above values from stack
    ldmfd   sp, {pc}^                     // Return with CPSR update
1:
    // Method to return to SVC mode is different than USER/SYS Mode
    ldmfd   sp, {r0-r12, sp, lr, pc}^     // Return with CPSR update

// ----------------------------------------------------------------------------
// Zero Context Restore
// ----------------------------------------------------------------------------
_OS_ZeroContextRestore:

    ldmfd   sp!, {r0}                     // Pop CPSR
    msr     spsr, r0                      // Restore SPSR.
    and     r0, r0, #MODEMASK             // Extract the mode bits
    teq     r0, #SVCMODE                  // Check if we are returning to SVC MODEMASK
    beq     1f
    
    // If we are going to USER/SYS mode, then we have special method to restore user registers
    // Read section B6.1.3 of DDI0406B Arm Architecture Reference Manual
    ldmfd   sp, {r0-r1, r11, sp, lr}^     // You cant use base register writeback
    add     sp, sp, #4*5                  // Discard above values from stack
    ldmfd   sp, {pc}^                     // Return with CPSR update
1:
    // Method to return to SVC mode is different than USER/SYS Mode
    ldmfd   sp, {r0-r1, r11, sp, lr, pc}^         

	
// -------------------------------------------------------------------------------
// A placeholder for tasks that have finished. 
// -------------------------------------------------------------------------------
CompletedTaskTrap:

    // TODO: Add support for thrashing finished tasks and its stack.
    // Currently it simply yields to other tasks

    bl      _OS_Schedule   
    b       CompletedTaskTrap
    

//------------------------------------------------------------------------------
// Some data constants
//------------------------------------------------------------------------------
	.section .data

_stack_corrupted_string:
	.string "Invalid stack type. Possibly corrupted."
	.align 4 	/* re-align to the word boundary */
