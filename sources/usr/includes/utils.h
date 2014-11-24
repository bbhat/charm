///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2014 xxxxxxx, xxxxxxx
//	File:	utils.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Utility function header file
//	
///////////////////////////////////////////////////////////////////////////////

#ifndef _UTIL_H
#define _UTIL_H

#include "types.h"

void SetResourceStatus(UINT32 res_mask[], INT32 res_index, BOOL free);
INT32 GetFreeResIndex(UINT32 res_mask[], INT32 count);
BOOL IsResourceBusy(UINT32 res_mask[], INT32 res_index);

#endif // _UTILS_H
