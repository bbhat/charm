//------------------------------------------------------------------------------
//
//						Copyright 2012-2013 xxxxxxx, xxxxxxx
//	File:	VectorZero.s
//	Author: Bala B. (bhat.balasubramanya//gmail.com)
//	Description: Vector table residing in NOR Flash at address 0
//	This simply redirects the interrupt to Vector table in SDRAM
//
//-------------------------------------------------------------------------------

   .text       // AREA ???, CODE, READONLY
   .code 32    // CODE32
//---------------------------------------------------------------------
//  Interrupt Vectors
//---------------------------------------------------------------------
	.global _start
	
_start:
	ldr	pc, [pc, #0x18]
	ldr	pc, [pc, #0x18]
	ldr	pc, [pc, #0x18]
	ldr	pc, [pc, #0x18]
	ldr	pc, [pc, #0x18]
	ldr	pc, [pc, #0x18]
	ldr	pc, [pc, #0x18]
	ldr	pc, [pc, #0x18]
	
	// Jump to interrupt handlers residing in DRAM
	
	.word	0x30000000
	.word	0x30000020
	.word	0x30000040
	.word	0x30000060
	.word	0x30000080
	.word	0x300000a0
	.word	0x300000c0
	.word	0x300000e0

	.string "Copyright 2012-2013 Bala B. xxxxxxxx, xxxxxxxx"
	.align 4 	/* re-align to the word boundary */	
	
   .END
