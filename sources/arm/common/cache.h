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

#include "os_config.h"
#include "sysctl.h"

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
	#define _OS_EnableICache() _sysctl_enable_icache()
	#define _OS_DisableICache() _sysctl_disable_icache()
	#define _OS_InvalidateICache() _sysctl_invalidate_icache_all()
#endif

#if ENABLE_DATA_CACHE == 1
	#define _OS_EnableDCache() _sysctl_enable_dcache()
	#define _OS_DisableDCache() _sysctl_disable_dcache()

	#define _OS_InvalidateDCache() _sysctl_invalidate_dcache_all()
	#define _OS_CleanInvalidateDCache() _sysctl_clean_invalidate_dcache_all()

	#define _OS_InvalidateDCacheRange(start, end) _sysctl_invalidate_dcache_range(start, end)
	#define _OS_CleanDCacheRange(start, end) _sysctl_clean_dcache_range(start, end)
	#define _OS_CleanInvalidateDCacheRange(start, end) _sysctl_clean_invalidate_dcache_range(start, end)
	
	void _OS_CleanInvalidateDCacheArea(void * va, UINT32 len);
#endif

#if ENABLE_L2_CACHE == 1
	#define _OS_Enable_L2_Cache() _sysctl_enable_l2cache()
	#define _OS_Disable_L2_Cache() _sysctl_disable_l2cache()
#endif

#if ENABLE_INSTRUCTION_CACHE == 1 || ENABLE_DATA_CACHE == 1
	#define _OS_invalidateCache() _sysctl_invalidate_cache_all()
#endif


#endif // _CACHE_H_
