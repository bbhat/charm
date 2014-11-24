///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2014 xxxxxxx, xxxxxxx
//	File:	utils.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Utility functions
//	
///////////////////////////////////////////////////////////////////////////////

#include "utils.h"

static __inline__ UINT32 clz(UINT32 input)
{
	unsigned int result;
	
	__asm__ volatile("clz %0, %1" : "=r" (result) : "r" (input));
	
	return result;
}

//////////////////////////////////////////////////////////////////////////////////////////
// This function finds an available resource given a bit mask of resource availability
// The res_mask is an array of UINT32 words. Each bit in each word stands for one resource
// If this bit is 0, then the resource is free. Else it is used.
// This method first searches the given array for a word (a block of 32 resources) which
// has a free resource.
// Then it uses binary search to find an unused bit index within a 32 bit word
// There is no maximum limit for the number of resources
//////////////////////////////////////////////////////////////////////////////////////////
INT32 GetFreeResIndex(UINT32 res_mask[], INT32 res_count)
{
	// Get the number of 32 bit words
	INT32 count = (res_count + 31) >> 5;
	INT32 free_res_index = -1;
	INT32 i;
	
	for(i = 0; i < count; i++)
	{
		if(~res_mask[i])
		{
			free_res_index = (i << 5) + (31 - clz(~res_mask[i]));
		}
	}

	// If the res_count is not a multiple of 32, then we will find a free resource which
	// is actually not available. So check for this condition and return -1
	return (free_res_index < res_count) ? free_res_index : -1;
}

void SetResourceStatus(UINT32 res_mask[], INT32 res_index, BOOL free)
{
	if(free) 
	{
		res_mask[res_index >> 5] &= ~(1 << (res_index & 0x1f));
	}
	else
	{
		res_mask[res_index >> 5] |= (1 << (res_index & 0x1f));
	}
}

BOOL IsResourceBusy(UINT32 res_mask[], INT32 res_index)
{
	return (res_mask[res_index >> 5] & (1 << (res_index & 0x1f)));
}
