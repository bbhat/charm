///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	mmu.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: MMU Related functions
//	
///////////////////////////////////////////////////////////////////////////////

#ifndef _MMU_H_
#define _MMU_H_

#include "os_types.h"
#include "os_config.h"

void MMU_EnableICache(void);
void MMU_DisableICache(void);
void MMU_EnableDCache(void);
void MMU_DisableDCache(void);
void MMU_flushICache(void);
void MMU_flushDCache(void);
void MMU_flushCache(void);
void MMU_CleanDCacheMVA(UINT32 mva);
void MMU_CleanInvalidateDCacheMVA(UINT32 mva);
void MMU_CleanDCacheIndex(UINT32 index);
void MMU_CleanInvalidateDCacheIndex(UINT32 index);
void MMU_CleanInvalidateDCache(void);
void MMU_WaitForInterrupt(void);

#if ENABLE_MMU==1
	#define _OS_WaitForInterrupt() MMU_WaitForInterrupt();
#else
	#define _OS_WaitForInterrupt()
#endif

#endif // _MMU_H_
