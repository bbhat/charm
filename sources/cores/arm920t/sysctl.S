// -------------------------------------------------------------------------------
// 
// 						Copyright 2013 xxxxxxx, xxxxxxx
// 	File:	sysctl.S
// 	Author: Bala B. (bhat.balasubramanya@gmail.com)
// 	Description: Functions related to System Control Coprocessor in arm920t
// 
// -------------------------------------------------------------------------------

	.code 32             		//  CODE32

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
	
	.global _sysctl_invalidate_icache
_sysctl_invalidate_icache:
	mcr  p15,0,r0,c7,c5,0
	mov  pc, lr
	
	.global _sysctl_invalidate_dcache
_sysctl_invalidate_dcache:
	mcr  p15,0,r0,c7,c6,0
	mov  pc, lr

	.global _sysctl_invalidate_cache
_sysctl_invalidate_cache:
	mcr  p15,0,r0,c7,c7,0
	mov  pc, lr

//----------------------------------------------------------------------------------------
//  Functions to clean data cache
//----------------------------------------------------------------------------------------

	.global _sysctl_clean_dcache_mva
_sysctl_clean_dcache_mva:
	mcr  p15,0,r0,c7,c10,1 
	mov  pc, lr

	.global _sysctl_clean_dcache_index
_sysctl_clean_dcache_index:
	mcr  p15,0,r0,c7,c10,2 
	mov  pc, lr

//----------------------------------------------------------------------------------------
//  Functions to clean and invalidate data cache
//----------------------------------------------------------------------------------------
	.global _sysctl_clean_invalidate_dcache_mva
_sysctl_clean_invalidate_dcache_mva:
	mcr  p15,0,r0,c7,c14,1 
	mov  pc, lr

	.global _sysctl_clean_invalidate_dcache_index
_sysctl_clean_invalidate_dcache_index:
	mcr  p15,0,r0,c7,c14,2 
	mov  pc, lr

	.global _sysctl_clean_invalidate_dcache
_sysctl_clean_invalidate_dcache:
    mov    r1, #0                        //  Initialize segment counter 
next_segment:
    mov    r0, #0                        //  Initialize cache line counter 
next_index:
    orr    r2, r1, r0                    //  Generate segment and line address
    mcr    p15, 0, r2, c7, c14, 2        //  Clean and flush the line
    add    r0, r0, #0x20                 //  Increment to next line
    cmp    r0, #0x400                    //  Complete all entries in one segment?
    bne    next_index                    //  If not branch back to next_index
    add    r1, r1, #0x40000000           //  Increment segment counter
    cmp    r1, #0x0                      //  Complete all segments
    bne    next_segment                  //  If not branch back to next_segment
	mov  pc, lr

//----------------------------------------------------------------------------------------
//  wait for interrupt 
//----------------------------------------------------------------------------------------		
	.global _sysctl_wait_for_interrupt
_sysctl_wait_for_interrupt:
   mcr  p15,0,r0,c7,c0,4 
   mov  pc, lr
