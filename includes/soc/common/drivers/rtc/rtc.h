///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	rtc.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Driver RTC
//	
///////////////////////////////////////////////////////////////////////////////

#ifndef _RTC_H_
#define _RTC_H_

#include "os_config.h"
#include "os_types.h"

#if ENABLE_RTC==1

// Structure for Date and Time
typedef struct
{
	UINT8	year;	// BCD format
	UINT8	mon;	// BCD format
	UINT8	date;	// BCD format
	UINT8	day;	// SUN-1 MON-2 TUE-3 WED-4 THU-5 FRI-6 SAT-7
	UINT8	hour;	// BCD format
	UINT8	min;	// BCD format
	UINT8	sec;	// BCD format
	
} OS_DateAndTime;

typedef struct
{
	UINT8	hour;	// BCD format
	UINT8	min;	// BCD format
	UINT8	sec;	// BCD format
} OS_Time;

enum
{
	SUNDAY = 1,
	MONDAY,
	TUESDAY,
	WEDNESDAY,
	THURSDAY,
	FIRDAY,
	SATURDAY
} ;

// RTC functions are defined in os_core.h

#endif // ENABLE_RTC

#endif // _RTC_H_