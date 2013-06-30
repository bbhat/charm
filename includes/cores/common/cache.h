///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	cache.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Cache Related functions
//	
///////////////////////////////////////////////////////////////////////////////

#ifndef _CACHE_H_
#define _CACHE_H_

#include "mmu.h"
#include "os_config.h"

#if defined(SOC_S5PV210)

	// S5PV210 has 32KB Instruction and Data Caches, 512 KB Unified L2 cache
	#define ICACHE_SIZE				0x8000
	#define DCACHE_SIZE			0x8000
	#define CACHE_LINE_SIZE		32

	// If the amount of memory to flush is >= half the cache size, we will flush the whole cache
	// Or else we will flush individual addresses
	#define WHOLE_CACHE_OP_THRESHOLD		0x4000

#elif defined(SOC_S3C2440)

	// ARM920T has 16KB Instruction and Data Caches
	#define ICACHE_SIZE				0x4000
	#define DCACHE_SIZE			0x4000
	#define CACHE_LINE_SIZE		32

	// If the amount of memory to flush is >= half the cache size, we will flush the whole cache
	// Or else we will flush individual addresses
	#define WHOLE_CACHE_OP_THRESHOLD		0x2000

#endif

#if ENABLE_INSTRUCTION_CACHE == 1
	#define _OS_EnableICache() MMU_EnableICache()
	#define _OS_DisableICache() MMU_DisableICache()
	#define _OS_flushICache() MMU_flushICache()
#endif

#if ENABLE_DATA_CACHE == 1
	#define _OS_EnableDCache() MMU_EnableDCache()
	#define _OS_DisableDCache() MMU_DisableDCache()
	#define _OS_flushDCache() MMU_flushDCache()
	#define _OS_CleanDCacheMVA(mva) MMU_CleanDCacheMVA(mva)
	#define _OS_CleanInvalidateDCacheMVA(mva) MMU_CleanInvalidateDCacheMVA(mva)
	#define _OS_CleanDCacheIndex(index) MMU_CleanDCacheIndex(index);
	#define _OS_CleanInvalidateDCacheIndex(index) MMU_CleanInvalidateDCacheIndex(index)
	void _OS_CleanInvalidateDCache(void * va, UINT32 len);
#endif

#if ENABLE_INSTRUCTION_CACHE == 1 || ENABLE_DATA_CACHE == 1
	#define _OS_flushCache() MMU_flushCache()
#endif


#endif // _CACHE_H_