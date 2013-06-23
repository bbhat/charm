///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	rtc.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Common RTC Driver
//	
///////////////////////////////////////////////////////////////////////////////

#include "rtc.h"
#include "os_core.h"
#include "soc.h"

#if ENABLE_RTC==1

#define RTC_ENABLE	0x01
#define RTC_DISABLE	0x00

OS_Error OS_GetDateAndTime(OS_DateAndTime *date_and_time)
{
    UINT32 intsts;
    UINT32 i;
    
    ASSERT(date_and_time);

	// RTC Control enable
    rRTCCON = RTC_ENABLE;

	OS_ENTER_CRITICAL(intsts);	// Enter critical section

    for(i = 0; i < 2; i++)
    {
		date_and_time->year = (rBCDYEAR & 0xff);
		date_and_time->mon = (rBCDMON & 0x1f);
		date_and_time->date = (rBCDDATE & 0x3f);
		date_and_time->day = (rBCDDAY & 0x07);
		date_and_time->hour = (rBCDHOUR & 0x3f);
		date_and_time->min = (rBCDMIN & 0x7f);
		date_and_time->sec = (rBCDSEC & 0x7f);
		
		// If second is zero, we may have tripped minute and others. So we have to
		// read everything again.
		if(date_and_time->sec > 0) break;
	}
	
	OS_EXIT_CRITICAL(intsts);	// Exit critical section

	// RTC Control disable  
    rRTCCON = RTC_DISABLE;
    
    return SUCCESS;
}

OS_Error OS_SetDateAndTime(const OS_DateAndTime *date_and_time)
{
	UINT32 intsts;

	ASSERT(date_and_time);
	
	if((date_and_time->year < 0 || date_and_time->year > 99)
	|| (date_and_time->mon < 1 || date_and_time->mon > 12)
	|| (date_and_time->date < 1 || date_and_time->date > 31)
	|| (date_and_time->day < 1 || date_and_time->day > 7)
	|| (date_and_time->hour < 0 || date_and_time->hour > 23)
	|| (date_and_time->min < 0 || date_and_time->min > 59)
	|| (date_and_time->sec < 0 || date_and_time->sec > 59))
	{
		return ARGUMENT_ERROR;
	}
	
	// RTC Control enable
    rRTCCON = RTC_ENABLE;
    
    OS_ENTER_CRITICAL(intsts);	// Enter critical section

    rBCDYEAR = (date_and_time->year & 0xff);
    rBCDMON  = (date_and_time->mon & 0x1f);
    rBCDDATE = (date_and_time->date & 0x3f);
    rBCDDAY  = (date_and_time->day & 0x07);
    rBCDHOUR = (date_and_time->hour & 0x3f);
    rBCDMIN  = (date_and_time->min & 0x7f);
    rBCDSEC  = (date_and_time->sec & 0x7f);

	OS_EXIT_CRITICAL(intsts);	// Exit critical section
	
	// RTC Control disable  
    rRTCCON = RTC_DISABLE;
    
    return SUCCESS;
}

OS_Error OS_GetTime(OS_Time *time)
{
    UINT32 intsts;
    UINT32 i;

	ASSERT(time);
	
	// RTC Control enable
    rRTCCON = RTC_ENABLE;

	OS_ENTER_CRITICAL(intsts);	// Enter critical section

    for(i = 0; i < 2; i++)
    {
		time->hour = (rBCDHOUR & 0x3f);
		time->min = (rBCDMIN & 0x7f);
		time->sec = (rBCDSEC & 0x7f);
		
		// If second is zero, we may have tripped minute and others. So we have to
		// read everything again.
		if(time->sec > 0) break;
	}
	
	OS_EXIT_CRITICAL(intsts);	// Exit critical section

	// RTC Control disable  
    rRTCCON = RTC_DISABLE;	
    
    return SUCCESS;
}

#if ENABLE_RTC_ALARM==1
OS_Error OS_SetAlarm(const OS_DateAndTime *date_and_time)
{
	ASSERT(date_and_time);
	
	// TODO:
	panic("Not implemented");
	
	return NOT_SUPPORTED;
}

OS_Error OS_GetAlarm(OS_DateAndTime *date_and_time)
{
	ASSERT(date_and_time);
	
	// TODO:
	panic("Not implemented");
	
	return NOT_SUPPORTED;
}
#endif // ENABLE_RTC_ALARM

#endif // ENABLE_RTC
