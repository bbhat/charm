///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	sysctl.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: System Controller Coprocessor Related functions
//	
///////////////////////////////////////////////////////////////////////////////

#ifndef _SYSCTL_H_
#define _SYSCTL_H_

#include "os_types.h"
#include "os_config.h"

//----------------------------------------------------------------------------------------
//  Functions to enable / disable  instruction and data caches
//----------------------------------------------------------------------------------------
void _sysctl_enable_icache(void);
void _sysctl_disable_icache(void);
void _sysctl_enable_dcache(void);
void _sysctl_disable_dcache(void);

//----------------------------------------------------------------------------------------
//  Functions to invalidate instruction and data caches
//  Not all functions are implemented by all platforms
//----------------------------------------------------------------------------------------
void _sysctl_invalidate_icache(void);
void _sysctl_invalidate_dcache(void);
void _sysctl_invalidate_cache(void);

void _sysctl_invalidate_icache_mva(UINT32 mva);
void _sysctl_invalidate_dcache_index(UINT32 index);
void _sysctl_invalidate_dcache_mva(UINT32 mva);

//----------------------------------------------------------------------------------------
//  Functions to clean data cache
//----------------------------------------------------------------------------------------
void _sysctl_clean_dcache_mva(UINT32 mva);
void _sysctl_clean_dcache_index(UINT32 index);

//----------------------------------------------------------------------------------------
//  Functions to clean & invalidate data cache
//----------------------------------------------------------------------------------------
void _sysctl_clean_invalidate_dcache_mva(UINT32 mva);
void _sysctl_clean_invalidate_dcache_index(UINT32 index);
void _sysctl_clean_invalidate_dcache(void);

//----------------------------------------------------------------------------------------
//  Miscellaneous functions
//----------------------------------------------------------------------------------------
void _sysctl_wait_for_interrupt(void);

#if ENABLE_L2_CACHE==1
	void _sysctl_enable_l2cache(void);
	void _sysctl_enable_l2cache(void);
#endif

#endif // _SYSCTL_H_
