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
#include "os_driver.h"

#if ENABLE_RTC

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
	
} OS_Weekday;

///////////////////////////////////////////////////////////////////////////////
// Driver functions
///////////////////////////////////////////////////////////////////////////////
OS_Return _RTC_DriverInit(void);

///////////////////////////////////////////////////////////////////////////////
// Date and Time functions
///////////////////////////////////////////////////////////////////////////////
OS_Return _OS_SetDateAndTime(const OS_DateAndTime *date_and_time);
OS_Return _OS_GetDateAndTime(OS_DateAndTime *date_and_time);
OS_Return _OS_GetTime(OS_Time *time);

#if ENABLE_RTC_ALARM

OS_Return _OS_SetAlarm(const OS_DateAndTime *date_and_time);
OS_Return _OS_GetAlarm(OS_DateAndTime *date_and_time);

#endif // ENABLE_RTC_ALARM

#endif // ENABLE_RTC

#endif // _RTC_H_
