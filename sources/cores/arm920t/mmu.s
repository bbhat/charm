@-------------------------------------------------------------------------------
@
@						Copyright 2013 xxxxxxx, xxxxxxx
@	File:	mmu.s
@	Author: Bala B. (bhat.balasubramanya@gmail.com)
@	Description: MMU Related functions
@
@-------------------------------------------------------------------------------

@-------------------------------------------------------------------------------
@ MMU Cache/TLB/etc on/off functions 
@-------------------------------------------------------------------------------
R1_I    	=	(1<<12) 
R1_C		=	(1<<2) 
R1_A		=	(1<<1) 
R1_M		=	(1) 
R1_iA		=	(1<<31) 
R1_nF		=	(1<<30)

	.global MMU_EnableICache
	.global MMU_DisableICache
	.global MMU_EnableDCache
	.global MMU_DisableDCache
	
	.global MMU_flushICache
	.global MMU_flushDCache
	.global MMU_flushCache
	
	.global MMU_CleanDCache

	.global MMU_CleanDCacheMVA
	.global MMU_CleanInvalidateDCacheMVA
	.global MMU_CleanDCacheIndex
	.global MMU_CleanInvalidateDCacheIndex
	
	.global MMU_WaitForInterrupt
		
	.code 32             		@ CODE32

	@ void MMU_EnableICache(void)
MMU_EnableICache:
	mrc  p15,0,r0,c1,c0,0 
	orr  r0,r0,#R1_I 
	mcr  p15,0,r0,c1,c0,0 
	mov  pc, lr
	
	@ void MMU_DisableICache(void)
MMU_DisableICache:
	mrc  p15,0,r0,c1,c0,0 
	bic  r0,r0,#R1_I 
	mcr  p15,0,r0,c1,c0,0 
	mov  pc, lr
	
	@ void MMU_EnableDCache(void)
MMU_EnableDCache:
	mrc  p15,0,r0,c1,c0,0 
	orr  r0,r0,#R1_C 
	mcr  p15,0,r0,c1,c0,0 
	mov  pc, lr
	
	@ void MMU_DisableDCache(void)
MMU_DisableDCache:
	mrc  p15,0,r0,c1,c0,0 
	bic  r0,r0,#R1_C 
	mcr  p15,0,r0,c1,c0,0 
	mov  pc, lr
	
	@ Flushes the I-cache
	@ void MMU_flushICache(void)
MMU_flushICache:
	mcr  p15,0,r0,c7,c5,0
	mov  pc, lr
	
	@ Flushes the D-cache
	@ void MMU_flushDCache(void)
MMU_flushDCache:
	mcr  p15,0,r0,c7,c6,0
	mov  pc, lr

	@ Flushes all cache
	@ void MMU_flushCache(void)
MMU_flushCache:
	mcr  p15,0,r0,c7,c7,0
	mov  pc, lr
	
	@ void MMU_CleanDCacheMVA(U32 mva) 
MMU_CleanDCacheMVA:
	mcr  p15,0,r0,c7,c10,1 
	mov  pc, lr

	@ void MMU_CleanInvalidateDCacheMVA(U32 mva) 
MMU_CleanInvalidateDCacheMVA:
	mcr  p15,0,r0,c7,c14,1 
	mov  pc, lr

	@ void MMU_CleanDCacheIndex(U32 index) 
MMU_CleanDCacheIndex:
	mcr  p15,0,r0,c7,c10,2 
	mov  pc, lr

	@ void MMU_CleanInvalidateDCacheIndex(U32 index)  
MMU_CleanInvalidateDCacheIndex:
	mcr  p15,0,r0,c7,c14,2 
	mov  pc, lr

	@ void MMU_CleanDCache(void)
MMU_CleanDCache:		
@    mov    r1, #0                        ; Initialize segment counter outer_loop
@    mov    r0, #0                        ; Initialize line counter inner_loop
@    orr    r2, r1, r0                    ; Generate segment and line address
@    mcr    p15, 0, r2, c7, c14, 2        ; Clean and flush the line
@    add    r0, r0, #0x20                 ; Increment to next line
@    cmp    r0, #0x400                    ; Complete all entries in one segment?
@    bne    inner_loop                    ; If not branch back to inner_loop
@    add    r1, r1, #0x40000000           ; Increment segment counter
@    cmp    r1, #0x0                      ; Complete all segments
@    bne    outer_loop                    ; If not branch back to outer_loop
	mov  pc, lr

MMU_WaitForInterrupt:
   mcr  p15,0,r0,c7,c0,4 
   mov  pc, lr
