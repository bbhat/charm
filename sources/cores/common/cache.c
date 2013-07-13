///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2012-2013 xxxxxxx, xxxxxxx
//	File:	cache.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Cache functions
//	
///////////////////////////////////////////////////////////////////////////////

#include "cache.h"

#if ENABLE_DATA_CACHE == 1

void _OS_CleanInvalidateDCache(void * va, UINT32 len)
{
	// Get a 32 Byte aligned address
	UINT8 * valigned = (UINT8*)((UINT32)va & ~0x1f);
	UINT32 offset = 0;
	
	if(!len || !va || (len >= WHOLE_CACHE_OP_THRESHOLD))
	{
		// Clean the whole cache
		_sysctl_clean_invalidate_dcache();
	}
	else
	{
		// Clean individual addresses
		while(offset < len)
		{
			_sysctl_clean_invalidate_dcache_mva(valigned + offset);
			offset += CACHE_LINE_SIZE;
		}
	}
}

#endif	// ENABLE_DATA_CACHE
