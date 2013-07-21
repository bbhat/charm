// -------------------------------------------------------------------------------
// 
// 						Copyright 2013 xxxxxxx, xxxxxxx
// 	File:	sysctl.S
// 	Author: Bala B. (bhat.balasubramanya@gmail.com)
// 	Description: Functions related to System Control Coprocessor in arm920t
// 
// -------------------------------------------------------------------------------

// -------------------------------------------------------------------------------
// cache_line_size - get the cache line size from the CTYPE (cache type) register
// Decoding cache line length field: 1 << (len+3) bytes 
// -------------------------------------------------------------------------------
	.macro	dcache_line_size, reg, tmp
	mrc	p15, 1, \tmp, c0, c0, 1		// read CTYPE
	mov	\tmp, \tmp, lsl #12			// get to cache line size
	and	\tmp, \tmp, #3				// cache line size mask
	mov	\reg, #8					// 1 << (len+3) is same as 8 << len
	mov	\reg, \reg, lsl \tmp		// actual cache line size
	.endm

	.code 32             		//  CODE32

//----------------------------------------------------------------------------------------
// Function to initialize System Control Coprocessor
// Registers used r0-r3, lr
//----------------------------------------------------------------------------------------

	.global _sysctl_init		// This function requires a valid stack
_sysctl_init:
	
	stmfd	sp!, {lr}			// This function requires a valid stack
	
	// Initialize cache parameters
	bl _sysctl_query_icache_params
	bl _sysctl_query_dcache_params
	
	ldmfd 	sp!, {lr}
	
	// Done for now
	mov  pc, lr

//----------------------------------------------------------------------------------------
//  Functions to enable / disable  instruction and data caches
//----------------------------------------------------------------------------------------

	.global _sysctl_enable_icache
_sysctl_enable_icache:
	mrc  p15,0,r0,c1,c0,0 
	orr  r0,r0,#(1<<12) 
	mcr  p15,0,r0,c1,c0,0 
	mov  pc, lr
	
	.global _sysctl_disable_icache
_sysctl_disable_icache:
	mrc  p15,0,r0,c1,c0,0 
	bic  r0,r0,#(1<<12) 
	mcr  p15,0,r0,c1,c0,0 
	mov  pc, lr
	
	.global _sysctl_enable_dcache
_sysctl_enable_dcache:
	mrc  p15,0,r0,c1,c0,0 
	orr  r0,r0,#(1<<2) 
	mcr  p15,0,r0,c1,c0,0 
	mov  pc, lr
	
	.global _sysctl_disable_dcache
_sysctl_disable_dcache:
	mrc  p15,0,r0,c1,c0,0 
	bic  r0,r0,#(1<<2) 
	mcr  p15,0,r0,c1,c0,0 
	mov  pc, lr

//----------------------------------------------------------------------------------------
//  Functions to invalidate instruction and data caches
//----------------------------------------------------------------------------------------
	
	.global _sysctl_invalidate_icache_all
_sysctl_invalidate_icache_all:
	mcr  p15,0,r0,c7,c5,0
	mov  pc, lr
	
	.global _sysctl_invalidate_dcache_all
_sysctl_invalidate_dcache_all:
	mcr  p15,0,r0,c7,c6,0
	mov  pc, lr

	.global _sysctl_invalidate_cache_all
_sysctl_invalidate_cache_all:
	mcr  p15,0,r0,c7,c7,0
	mov  pc, lr

//----------------------------------------------------------------------------------------
//  Functions to clean data cache
//----------------------------------------------------------------------------------------

	.global _sysctl_clean_dcache_all
_sysctl_clean_dcache_all:
	mcr  p15,0,r0,c7,c10,0
	mov  pc, lr

//----------------------------------------------------------------------------------------
//  Functions to clean and invalidate data cache
//----------------------------------------------------------------------------------------
	.global _sysctl_clean_invalidate_dcache_all
_sysctl_clean_invalidate_dcache_all:
	mcr  p15,0,r0,c7,c14,0
	mov  pc, lr

//----------------------------------------------------------------------------------------
// Cache functions that operate on a range of virtual addresses
// Invalidate the data cache within the specified region; we will
// be performing a DMA operation in this region and we want to
// purge old data in the cache.
//
// start   - r0 -virtual start address of region
// end     - r1 - virtual end address of region
// 
// Note: If the start and end addresses are not cache line aligned, then we should
// clean & invalidate those two lines while other lines are just invalidated
//----------------------------------------------------------------------------------------		
	.global _sysctl_invalidate_dcache_range
_sysctl_invalidate_dcache_range:
	dcache_line_size r2, r3
	sub	r3, r2, #1
	tst	r0, r3
	bic	r0, r0, r3
	mcrne	p15, 0, r0, c7, c14, 1		// clean & invalidate D / U cache line

	tst	r1, r3
	bic	r1, r1, r3
	mcrne	p15, 0, r1, c7, c14, 1		// clean & invalidate D / U cache line
1:
	mcr	p15, 0, r0, c7, c6, 1			// invalidate D / U cache line
	add	r0, r0, r2
	cmp	r0, r1
	blo	1b
	mov	pc, lr

//----------------------------------------------------------------------------------------
// Invalidate the data cache within the specified region
// start   - r0 -virtual start address of region
// end     - r1 - virtual end address of region
//----------------------------------------------------------------------------------------
	.global _sysctl_clean_dcache_range
_sysctl_clean_dcache_range:
	dcache_line_size r2, r3
	sub	r3, r2, #1
	bic	r0, r0, r3
1:
	mcr	p15, 0, r0, c7, c10, 1		@ clean D / U cache line
	add	r0, r0, r2
	cmp	r0, r1
	blo	1b
	mov	pc, lr
 
//----------------------------------------------------------------------------------------
// Clean & Invalidate the data cache within the specified region
// start   - r0 -virtual start address of region
// end     - r1 - virtual end address of region
//----------------------------------------------------------------------------------------
	.global _sysctl_clean_invalidate_dcache_range
_sysctl_clean_invalidate_dcache_range:
	dcache_line_size r2, r3
	sub	r3, r2, #1
	bic	r0, r0, r3
1:
	mcr	p15, 0, r0, c7, c14, 1		@ clean & invalidate D / U cache line
	add	r0, r0, r2
	cmp	r0, r1
	blo	1b
	mov	pc, lr

//----------------------------------------------------------------------------------------
//  wait for interrupt 
//----------------------------------------------------------------------------------------		
	.global _sysctl_wait_for_interrupt
_sysctl_wait_for_interrupt:
   mcr  p15,0,r0,c7,c0,4 
   mov  pc, lr

//----------------------------------------------------------------------------------------
//  Functions to read various cache sizes
//----------------------------------------------------------------------------------------		
	SELECT_DCACHE	=	0
	SELECT_ICACHE	=	1
	
_sysctl_query_icache_params:
	// Select icache in Cache Size Selection Register (CSSELR)
	mov  r0, #SELECT_ICACHE
	mcr  p15,2,r0,c0,c0,0        	   	// Write to CSSELR
	
	// Now read Cache Size ID Registers (CCSIDR)
	mrc  p15,1,r0,c0,c0,0        	   	// Read CCSIDR
	
	// Extract counts
	// First cache line size
	and  r1, r0, #7						// Extract bits[2:0]
	add  r1, r1, #2						// Add #2
	mov  r2, #4
	lsl  r2, r2, r1						// Number of Bytes
	ldr	 r1, =_icache_line_size
	str  r2, [r1]

	// Extract number of ways
	lsr  r1, r0, #0x3
	ldr  r2, =0x3ff						// Extract bits[12:3]
	and  r1, r1, r2
	add  r2, r1, #1						// Add #1
	ldr	 r1, =_icache_ways_count
	str  r2, [r1]
	
	// Extract number of sets
	lsr  r1, r0, #13
	ldr  r2, =0x7fff					// Extract bits[27:13]
	and  r1, r1, r2
	add  r2, r1, #1						// Add #1
	ldr	 r1, =_icache_set_count
	str  r2, [r1]
	
	mov  pc, lr
	
_sysctl_query_dcache_params:
	// Select dcache in Cache Size Selection Register (CSSELR)
	mov  r0, #SELECT_DCACHE
	mcr  p15,2,r0,c0,c0,0        	   	// Write to CSSELR
	
	// Now read Cache Size ID Registers (CCSIDR)
	mrc  p15,1,r0,c0,c0,0        	   	// Read CCSIDR
	
	// Extract counts
	// First cache line size
	and  r1, r0, #7						// Extract bits[2:0]
	add  r1, r1, #2						// Add #2
	mov  r2, #4
	lsl  r2, r2, r1						// Number of Bytes
	ldr	 r1, =_dcache_line_size
	str  r2, [r1]

	// Extract number of ways
	lsr  r1, r0, #3
	ldr  r2, =0x3ff						// Extract bits[12:3]
	and  r1, r1, r2
	add  r2, r1, #1						// Add #1
	ldr	 r1, =_dcache_ways_count
	str  r2, [r1]
	
	// Extract number of sets
	lsr  r1, r0, #13
	ldr  r2, =0x7fff					// Extract bits[27:13]
	and  r1, r1, r2
	add  r2, r1, #1						// Add #1
	ldr	 r1, =_dcache_set_count
	str  r2, [r1]

	mov  pc, lr
	
//----------------------------------------------------------------------------------------
// Some global variables to hold cache attributes
//----------------------------------------------------------------------------------------
	.section .data

//---------- Instruction cache ------------

	.global	_icache_set_count
_icache_set_count:
	.space		4

	.global	_icache_ways_count
_icache_ways_count:
	.space		4

	.global	_icache_line_size
_icache_line_size:
	.space		4

//---------- Data cache ------------
	.global	_dcache_set_count
_dcache_set_count:
	.space		4

	.global	_dcache_ways_count
_dcache_ways_count:
	.space		4

	.global	_dcache_line_size
_dcache_line_size:
	.space		4
