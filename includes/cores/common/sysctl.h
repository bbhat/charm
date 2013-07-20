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
void _sysctl_invalidate_icache_all(void);
void _sysctl_invalidate_dcache_all(void);
void _sysctl_invalidate_cache_all(void);

void _sysctl_invalidate_dcache_range(void *start, void *end);

//----------------------------------------------------------------------------------------
//  Functions to clean data cache
//----------------------------------------------------------------------------------------
void _sysctl_clean_dcache_range(void *start, void *end);

//----------------------------------------------------------------------------------------
//  Functions to clean & invalidate data cache
//----------------------------------------------------------------------------------------
void _sysctl_clean_invalidate_dcache_all(void);
void _sysctl_clean_invalidate_dcache_range(void *start, void *end);

//----------------------------------------------------------------------------------------
//  Miscellaneous functions
//----------------------------------------------------------------------------------------
void _sysctl_wait_for_interrupt(void);

#if ENABLE_L2_CACHE==1
	void _sysctl_enable_l2cache(void);
	void _sysctl_disable_l2cache(void);
#endif


//----------------------------------------------------------------------------------------
//  Global Data members
//----------------------------------------------------------------------------------------
extern UINT32 _icache_set_count;
extern UINT32 _icache_ways_count;
extern UINT32 _icache_line_size;
extern UINT32 _dcache_set_count;
extern UINT32 _dcache_ways_count;
extern UINT32 _dcache_line_size;
extern UINT32 _l2_cache_set_count;
extern UINT32 _l2_cache_ways_count;
extern UINT32 _l2_cache_line_size;

#endif // _SYSCTL_H_
