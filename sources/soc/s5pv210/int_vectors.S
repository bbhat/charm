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

.global _jump_to_custom_IRQ_Handler
.global _interrupt_vector_table

//------------------------------------------------------------------------------
// Interrupt vector in Internal SRAM. We get control into this section from the  IROM. 
// The iROM has fixed the address of interrupt vector table at 0xD0037400
//------------------------------------------------------------------------------
	.section .vectors

_interrupt_vector_table:
	.space   0x80

	
//-------------------------------------------------------------------------	
// Function to jump to custom IRQ Handler
// void _jump_to_custom_IRQ_Handler(old task)
//-------------------------------------------------------------------------

	.section .text       	// AREA ???, CODE, READONLY
	.code 32             	// CODE32

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
	
