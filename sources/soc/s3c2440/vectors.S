///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	int_vectors.S
//	Author:	Bala B.
//	Description: S3C2440 Interrupt handlers
///////////////////////////////////////////////////////////////////////////////

	.global	_IRQHandler_
	.global _reset_handler
	.global _jump_to_custom_IRQ_Handler

//-------------------------------------------------------------------------------
// INTERRUPT Registers
//-------------------------------------------------------------------------------
	SRCPND   	=  0x4a000000    // Interrupt r=est status
	INTMOD   	=  0x4a000004    // Interrupt mode control
	INTMSK   	=  0x4a000008    // Interrupt mask control
	PRIORITY 	=  0x4a00000c    // IRQ priority control
	INTPND   	=  0x4a000010    // Interrupt request status
	INTOFFSET	=  0x4a000014    // Interruot request source offset
	SUSSRCPND	=  0x4a000018    // Sub source pending
	INTSUBMSK	=  0x4a00001c    // Interrupt sub mask

	.section .text       		// AREA ???, CODE, READONLY
	.code 32             		// CODE32

//------------------------------------------------------------------------------
// _jump_to_custom_IRQ_Handler(old task)
// Function to call custom ISR
//------------------------------------------------------------------------------

	// Register r0 has the old g_current_task that will be passed to the called function
_jump_to_custom_IRQ_Handler:

	ldr     r1,=INTOFFSET				// Read the INTOFFSET register before enabling interrupts
	ldr     r1,[r1]  

	// TODO: Enable interrupts once you have debugged the simpler case
	// Now that we have cleared the IRQ stack, we can re-enable interrupts
// 	bic		r7, r7, #I_Bit			
// 	msr		cpsr_c, r7					// Reenable IRQ interrupt

	// Now it is safe to jump to the user defined interrupt handler
	// Note:
	ldr      r2,=_IRQ_TABLE_
	add      r2,r2,r1,lsl #2	// Use INTOFFSET register value to dereference the Vector table
	ldr      pc,[r2]			// We should never return from here

//---------------------------------------------------------------------
// Function to set custom IRQ Interrupt vector
// OS_InterruptVector OS_SetInterruptVector(OS_InterruptVector isr, UINT32 index)
//---------------------------------------------------------------------
	.global OS_SetInterruptVector
OS_SetInterruptVector:

	ldr		r2,=HandleEINT0		// Load the first IRQ handler address
	add		r2, r1, LSL #2
	swp		r0, r0, [r2]		// Read the old interrupt vector and replace with the new one
	
	mov		r3, #1
	mov		r3, r3, LSL r1
	ldr		r2,=INTMSK			// Clear appropriate bit in INTMSK
	ldr		r1, [r2]			// Read INTMSK
	bic		r1, r1, r3
	str		r1, [r2]
		
	mov      pc, lr
	
	.LTORG	


//------------------------------------------------------------------------------
// Interrupt vector in DRAM. We get control into this section from the 
// Vector table in the flash at address zero
//------------------------------------------------------------------------------
	.section .text.vector       // AREA ???, CODE, READONLY
	.code 32             		// CODE32
	
	// Reset  
	b		_reset_handler
	.space  28		

	// Undefined Instruction Exception
	b		.
	.space  28		
    
    // Software Interrupt Exception
	b		.
	.space  28	
		
	// Prefetch Abort Exception
	b		.
	.space  28		

	// Data Abort Exception
	b		.
	.space  28		

	// Reserved
	b		.
	.space  28		

	// IRQ Exception
	b		_IRQHandler_
	.space  28		
	
	// FIQ Exception
	b		.
	.space  28		


	// IRQ Interrupts
	.global		_IRQ_TABLE_
_IRQ_TABLE_:

HandleEINT0:	mov    pc, lr
HandleEINT1:	mov    pc, lr
HandleEINT2:	mov    pc, lr
HandleEINT3:	mov    pc, lr
HandleEINT4_7:	mov    pc, lr
HandleEINT8_23:	mov    pc, lr
HandleCAM:		mov    pc, lr		// Added for 2440.
HandleBATFLT:	mov    pc, lr
HandleTICK:		mov    pc, lr
HandleWDT:		mov    pc, lr
HandleTIMER0: 	mov    pc, lr
HandleTIMER1: 	mov    pc, lr
HandleTIMER2: 	mov    pc, lr
HandleTIMER3: 	mov    pc, lr
HandleTIMER4: 	mov    pc, lr
HandleUART2:  	mov    pc, lr
HandleLCD: 		mov    pc, lr
HandleDMA0:		mov    pc, lr
HandleDMA1:		mov    pc, lr
HandleDMA2:		mov    pc, lr
HandleDMA3:		mov    pc, lr
HandleMMC:		mov    pc, lr
HandleSPI0:		mov    pc, lr
HandleUART1:	mov    pc, lr
HandleNFCON:	mov    pc, lr		// Added for 2440.
HandleUSBD:		mov    pc, lr
HandleUSBH:		mov    pc, lr
HandleIIC:		mov    pc, lr
HandleUART0: 	mov    pc, lr
HandleSPI1: 	mov    pc, lr
HandleRTC: 		mov    pc, lr
HandleADC: 		mov    pc, lr
VectorTableEnd:	mov    pc, lr

