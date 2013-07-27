///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	int_vectors.S
//	Author:	Bala B.
//	Description: S5PV210 Interrupt handlers
///////////////////////////////////////////////////////////////////////////////

VIC0_BASE	=	0xF2000000
VIC1_BASE	=	0xF2100000
VIC2_BASE	=	0xF2200000
VIC3_BASE	=	0xF2300000

// VIC0
VIC0IRQSTATUS	=	(VIC0_BASE + 0x00)
VIC0FIQSTATUS	=	(VIC0_BASE + 0x04)
VIC0ADDR		=	(VIC0_BASE + 0xf00)

// VIC1
VIC1IRQSTATUS	=	(VIC1_BASE + 0x00)
VIC1FIQSTATUS	=	(VIC1_BASE + 0x04)
VIC1ADDR		=	(VIC1_BASE + 0xf00)

// VIC2
VIC2IRQSTATUS	=	(VIC2_BASE + 0x00)
VIC2FIQSTATUS	=	(VIC2_BASE + 0x04)
VIC2ADDR		=	(VIC2_BASE + 0xf00)

// VIC3
VIC3IRQSTATUS	=	(VIC3_BASE + 0x00)
VIC3FIQSTATUS	=	(VIC3_BASE + 0x04)
VIC3ADDR		=	(VIC3_BASE + 0xf00)

	.global _reset_handler
	.global _undefined_instr_handler
	.global _software_interrupt_handler
	.global _prefetch_abort_handler
	.global _data_abort_handler
	.global _reserved_interrupt
	.global _IRQHandler_
	.global _FIQHandler_
//---------------------------------------------------------------------
//  Interrupt Vectors
//---------------------------------------------------------------------
	.section	.text.vectors
   	.code 32    // CODE32
   	
	.global _interrupt_vector_table
_interrupt_vector_table:

	ldr	pc, =_reset_handler
	ldr	pc, =_undefined_instr_handler
	ldr	pc, =_SWIHandler_
	ldr	pc, =_prefetch_abort_handler
	ldr	pc, =_data_abort_handler
	ldr	pc, =_reserved_interrupt
	ldr	pc, =_IRQHandler_
	ldr	pc, =_FIQHandler_
	

	.string "Copyright 2012-2013 Bala B. xxxxxxxx, xxxxxxxx"
	.align 4 	/* re-align to the word boundary */	
		
	
//-------------------------------------------------------------------------	
// Function to jump to custom IRQ Handler
// void _jump_to_custom_IRQ_Handler(old task)
//-------------------------------------------------------------------------

	.section .text       	// AREA ???, CODE, READONLY
	.code 32             	// CODE32
	
	.global _SWIHandler_
_SWIHandler_:
	
	stmfd   sp!, {lr}  			   // Store registers
	mrs     r3, spsr               // Get spsr into r3. Note that r3 is reserved in syscall
	stmfd   sp!, {r3}              // Store spsr onto stack
								   // Only required for nested SVCs.
    ldr		r3, [lr,#-4]           // Load the instruction word and...
    bic   	r3, r3, #0xff000000    // ...extract comment field

    // r0 - r2 now contains syscall parameters
    // r3 now contains SVC number
    
    bl      _software_interrupt_handler		// Call main part of handler
    ldmfd   sp!, {r3}          	   // Get spsr from stack
    msr     SPSR_cxsf, r3          // Restore spsr
    ldmfd   sp!, {pc}^ 			   // Restore registers and return	
	
	
	.global _jump_to_custom_IRQ_Handler
_jump_to_custom_IRQ_Handler:
	
	// Check VIC0
	ldr		r1,=VIC0IRQSTATUS
	ldr 	r1,[r1]
	cmp		r1, #0
	ldrne 	r2,=VIC0ADDR
	ldrne	pc,[r2]			// We dont retrun from here

	// Check VIC1
	ldr		r1,=VIC0IRQSTATUS
	ldr 	r1,[r1]
	cmp		r1, #0
	ldrne 	r2,=VIC1ADDR
	ldrne	pc,[r2]			// We dont retrun from here

	// Check VIC2
	ldr		r1,=VIC2IRQSTATUS
	ldr 	r1,[r1]
	cmp		r1, #0
	ldrne 	r2,=VIC2ADDR
	ldrne	pc,[r2]			// We dont retrun from here

	// Check VIC3
	ldr		r1,=VIC3IRQSTATUS
	ldr 	r1,[r1]
	cmp		r1, #0
	ldrne 	r2,=VIC3ADDR
	ldrne	pc,[r2]			// We dont retrun from here

	// If we dont find any interrupts, lets hangup. This should never happen
	b		.
