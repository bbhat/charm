// -------------------------------------------------------------------------------
// 
// 						Copyright 2013 xxxxxxx, xxxxxxx
// 	File:	sysctl.s
// 	Author: Bala B. (bhat.balasubramanya@gmail.com)
// 	Description: Functions related to System Control Coprocessor in Cortex A8
// 
// -------------------------------------------------------------------------------

// -------------------------------------------------------------------------------
// cache_line_size - get the cache line size from the CSIDR register
// (available on ARMv7+). It assumes that the CSSR register was configured
// to access the L1 data cache CSIDR.
// -------------------------------------------------------------------------------
	.macro	dcache_line_size, reg, tmp
	mrc	p15, 1, \tmp, c0, c0, 0		// read CSIDR
	and	\tmp, \tmp, #7				// cache line size encoding
	mov	\reg, #16					// size offset
	mov	\reg, \reg, lsl \tmp		// actual cache line size
	.endm
	
	
	.code 32             		//  CODE32

//----------------------------------------------------------------------------------------
// Function to initialize System Control Coprocessor
// Registers used r0-r3, lr
//----------------------------------------------------------------------------------------
	.global _sysctl_init		
_sysctl_init:
	
	stmfd	sp!, {lr}			// This function requires a valid stack
	
	// Lets use SWP instructions
	mrc  p15,0,r0,c1,c0,0
	orr  r0, r0, #(1<<10)		// Set the SW bit in System Control Register (SCTLR)
	mcr  p15,0,r0,c1,c0,0
	
	// Initialize cache parameters
	bl _sysctl_query_icache_params
	bl _sysctl_query_dcache_params
	bl _sysctl_query_l2_cache_params
	
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
// Functions to enable / disable  L2 Unified cache
// The L2 cache is enabled when both the C bit of the CP15 Control Register c1 and the 
// L2EN bit of the CP15 Auxiliary Control Register, c1, are both active
//----------------------------------------------------------------------------------------
	.global _sysctl_enable_l2cache
_sysctl_enable_l2cache:

	// Program the Auxiliary Control Register to set the L2EN bit to 1
	mrc  p15,0,r0,c1,c0,1
	orr  r0, r0, #(1 << 1)
	mcr  p15, 0, r0, c1, c0, 1
	
	// Program the L2 Auxiliary Control Register to configure L2 cache
	// L2 data RAM read multiplexer select, Tag RAM latency, Data RAM latency 
	ldr  r0, =((1 << 29) | (1 << 6) | (1 << 0))
	mcr  p15, 1, r0, c9, c0, 2 

	// Program the C bit in the CP15 Control Register c1	
	mrc  p15,0,r0,c1,c0,0 
	orr  r0,r0,#(1<<2) 
	mcr  p15,0,r0,c1,c0,0 
	
	mov  pc, lr

	.global _sysctl_disable_l2cache
_sysctl_disable_l2cache:

	stmfd	sp!, {lr}			// This function requires a valid stack
	
	// If L2 cache is not enabled, simply return
	mrc  p15, 0, r0, c1, c0, 1
	tst  r0, #(1<<1)
	beq	 1f
	
	// Disable the C bit.
	mrc  p15,0,r0,c1,c0,0 
	stmfd sp!, {r0}				// Temporarily Store the value for later restore
	bic  r0,r0,#(1<<2) 
	mcr  p15,0,r0,c1,c0,0 
	
	// Clean and Invalidate whold D Cache
	bl _sysctl_clean_invalidate_dcache_all
	
	// Invalidate whole iCache
	bl _sysctl_invalidate_icache_all
	
	// Disable the L2 cache by clearing the L2EN bit to 0.
	mrc  p15, 0, r0, c1, c0, 1
	bic  r0, r0, #(1<<1)
	mcr  p15, 0, r0, c1, c0, 1	
	
	// Set the old coltrol register value
	ldmfd sp!, {r0}
	mcr  p15,0,r0,c1,c0,0 	
1:	
	ldmfd	sp!, {lr}
	mov	 pc, lr

//----------------------------------------------------------------------------------------
// Invalidate the whole instruction cache
//----------------------------------------------------------------------------------------
	.global _sysctl_invalidate_icache_all
_sysctl_invalidate_icache_all:
	mcr  p15, 0, r1, c7, c5, 0           @ invalidate I cache
	mov  pc, lr

//----------------------------------------------------------------------------------------
// Invalidate the whole D-cache.
// Registers used: r0-r7, r9-r11 (r6 only in Thumb mode)
// Note: This function requires a valid stack
// Reference: See section B2-16 of ARMv7-A ARM Architecture Reference Manual
//----------------------------------------------------------------------------------------		
	.global _sysctl_invalidate_dcache_all
_sysctl_invalidate_dcache_all:
	stmfd sp!, {r4-r5, r7, r9-r11}	// Store registers used here. We dont have to store r0-r3
	dmb								// ensure ordering with previous memory accesses
	mrc	p15, 1, r0, c0, c0, 1		// read Cache Level ID register
	ands r3, r0, #0x7000000			// extract loc from clidr
	mov	r3, r3, lsr #23				// left align loc bit field
	beq	finished1					// if loc is 0, then no need to clean
	mov	r10, #0						// start clean at cache level 0
loop1:
	add	r2, r10, r10, lsr #1		// work out 3x current cache level
	mov	r1, r0, lsr r2				// extract cache type bits from clidr
	and	r1, r1, #7					// mask of the bits for current cache only
	cmp	r1, #2						// see what cache we have at this level
	blt	skip1						// skip if no cache, or just i-cache
	mcr	p15, 2, r10, c0, c0, 0		// select current cache level in cssr
	isb								// isb to sych the new cssr & csidr
	mrc	p15, 1, r1, c0, c0, 0		// read the new csidr
	and	r2, r1, #7					// extract the length of the cache lines
	add	r2, r2, #4					// add 4 for the line length offset (log2 16 bytes)
	ldr	r4, =0x3ff
	ands r4, r4, r1, lsr #3			// R4 is the max number on the way size (right aligned)
	clz	r5, r4						// R5 is the bit position of the way size increment
	ldr	r7, =0x7fff
	ands	r7, r7, r1, lsr #13		// R7 is the max number of the index size (right aligned)
loop2:
	mov	r9, r4						// R9 working copy of the max way size (right aligned)
loop3:
 	orr	r11, r10, r9, lsl r5		// factor way and cache number into r11
 	orr	r11, r11, r7, lsl r2		// factor index number into r11
	mcr  p15, 0, r0, c7, c6, 2		// invalidate by set/way
	subs r9, r9, #1					// decrement the way
	bge	loop3
	subs r7, r7, #1					// decrement the index
	bge	loop2
skip1:
	add	r10, r10, #2				// increment cache number
	cmp	r3, r10
	bgt	loop1
finished1:
	mov	r10, #0						// swith back to cache level 0
	mcr	p15, 2, r10, c0, c0, 0		// select current cache level in cssr
	ldmfd sp!, {r4-r5, r7, r9-r11}	// Restore register
	dsb
	isb
	mov	pc, lr

//----------------------------------------------------------------------------------------
// Invalidate the whole cache system (instruction & data)
// Note: This function requires a valid stack
//----------------------------------------------------------------------------------------		
	.global _sysctl_invalidate_cache_all
_sysctl_invalidate_cache_all:
	stmfd	sp!, {lr}			// This function requires a valid stack
	bl _sysctl_invalidate_dcache_all
	bl _sysctl_invalidate_icache_all
	ldmfd	sp!, {lr}			// This function requires a valid stack
	mov	pc, lr

//----------------------------------------------------------------------------------------
// clean & invalidate the whole D-cache.
// Registers used: r0-r7, r9-r11 (r6 only in Thumb mode)
// Note: This function requires a valid stack
// Reference: See section B2-16 of ARMv7-A ARM Architecture Reference Manual
//----------------------------------------------------------------------------------------		
	.global _sysctl_clean_invalidate_dcache_all
_sysctl_clean_invalidate_dcache_all:
	stmfd sp!, {r4-r5, r7, r9-r11}	// Store registers used here. We dont have to store r0-r3
	dmb								// ensure ordering with previous memory accesses
	mrc	p15, 1, r0, c0, c0, 1		// read Cache Level ID register
	ands r3, r0, #0x7000000			// extract loc from clidr
	mov	r3, r3, lsr #23				// left align loc bit field
	beq	finished11					// if loc is 0, then no need to clean
	mov	r10, #0						// start clean at cache level 0
loop11:
	add	r2, r10, r10, lsr #1		// work out 3x current cache level
	mov	r1, r0, lsr r2				// extract cache type bits from clidr
	and	r1, r1, #7					// mask of the bits for current cache only
	cmp	r1, #2						// see what cache we have at this level
	blt	skip11						// skip if no cache, or just i-cache
	mcr	p15, 2, r10, c0, c0, 0		// select current cache level in cssr
	isb								// isb to sych the new cssr & csidr
	mrc	p15, 1, r1, c0, c0, 0		// read the new csidr
	and	r2, r1, #7					// extract the length of the cache lines
	add	r2, r2, #4					// add 4 for the line length offset (log2 16 bytes)
	ldr	r4, =0x3ff
	ands r4, r4, r1, lsr #3			// R4 is the max number on the way size (right aligned)
	clz	r5, r4						// R5 is the bit position of the way size increment
	ldr	r7, =0x7fff
	ands	r7, r7, r1, lsr #13		// R7 is the max number of the index size (right aligned)
loop21:
	mov	r9, r4						// R9 working copy of the max way size (right aligned)
loop31:
 	orr	r11, r10, r9, lsl r5		// factor way and cache number into r11
 	orr	r11, r11, r7, lsl r2		// factor index number into r11
	mcr	p15, 0, r11, c7, c14, 2		// clean & invalidate by set/way
	subs r9, r9, #1					// decrement the way
	bge	loop31
	subs r7, r7, #1					// decrement the index
	bge	loop21
skip11:
	add	r10, r10, #2				// increment cache number
	cmp	r3, r10
	bgt	loop11
finished11:
	mov	r10, #0						// swith back to cache level 0
	mcr	p15, 2, r10, c0, c0, 0		// select current cache level in cssr
	ldmfd sp!, {r4-r5, r7, r9-r11}	// Restore register
	dsb
	isb
	mov	pc, lr

//----------------------------------------------------------------------------------------
// Cache functions that operate on a range of virtual addresses
// Invalidate the data cache within the specified region; we will
// be performing a DMA operation in this region and we want to
// purge old data in the cache.
//
// start   - r0 -virtual start address of region
// end     - r1 - virtual end address of region
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
	dsb
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
	dsb
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
	dsb
	mov	pc, lr
	
//----------------------------------------------------------------------------------------
//  wait for interrupt 
//----------------------------------------------------------------------------------------		
	.global _sysctl_wait_for_interrupt
_sysctl_wait_for_interrupt:
	wfi									// There is wfi instruction available in armv7
	mov  pc, lr

//----------------------------------------------------------------------------------------
//  Functions to read various cache sizes
//----------------------------------------------------------------------------------------		
	SELECT_DCACHE	=	0
	SELECT_ICACHE	=	1
	SELECT_L2_CACHE	=	2
	
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
	
_sysctl_query_l2_cache_params:
	// Select l2 cache in Cache Size Selection Register (CSSELR)
	mov  r0, #SELECT_L2_CACHE
	mcr  p15,2,r0,c0,c0,0        	   	// Write to CSSELR
	
	// Now read Cache Size ID Registers (CCSIDR)
	mrc  p15,1,r0,c0,c0,0        	   	// Read CCSIDR
	
	// Extract counts
	// First cache line size
	and  r1, r0, #7						// Extract bits[2:0]
	add  r1, r1, #2						// Add #2
	mov  r2, #4
	lsl  r2, r2, r1						// Number of Bytes
	ldr	 r1, =_l2_cache_line_size
	str  r2, [r1]

	// Extract number of ways
	lsr  r1, r0, #3
	ldr  r2, =0x3ff						// Extract bits[12:3]
	and  r1, r1, r2
	add  r2, r1, #1						// Add #1
	ldr	 r1, =_l2_cache_ways_count
	str  r2, [r1]
	
	// Extract number of sets
	lsr  r1, r0, #13
	ldr  r2, =0x7fff					// Extract bits[27:13]
	and  r1, r1, r2
	add  r2, r1, #1						// Add #1
	ldr	 r1, =_l2_cache_set_count
	str  r2, [r1]
	
	mov  pc, lr

//----------------------------------------------------------------------------------------
// Helper functions
//----------------------------------------------------------------------------------------
	.global _sysctl_query_l2_aux_control_reg
_sysctl_query_l2_aux_control_reg:
	mrc p15, 1, r0, c9, c0, 2
	mov pc, lr
	
//----------------------------------------------------------------------------------------
// Functions to enable/disable MMU
//----------------------------------------------------------------------------------------
	.global _sysctl_enable_mmu
_sysctl_enable_mmu:

	mrc  p15, 0, r1, c1, c0, 0     // read CP15 Register 1
	orr  r1, r1, #1
	mcr  p15, 0, r1, c1, c0, 0     // Enable MMU
	
	// Virtual addressing starts from here
	mov  pc, lr
	
	.global _sysctl_enable_mmu
_sysctl_enable_mmu:
		
	mrc  p15, 0, r1, c1, c0, 0     // read CP15 Register 1
	bic  r1, r1, #1
	mcr  p15, 0, r1, c1, c0, 0     // Disable MMU
	
	// Flat addressing from here
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

//---------- L2 cache ------------

	.global	_l2_cache_set_count
_l2_cache_set_count:
	.space		4

	.global	_l2_cache_ways_count
_l2_cache_ways_count:
	.space		4

	.global	_l2_cache_line_size
_l2_cache_line_size:
	.space		4
