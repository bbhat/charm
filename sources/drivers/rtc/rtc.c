///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	rtc.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Common RTC Driver
//	
///////////////////////////////////////////////////////////////////////////////

#include "rtc.h"
#include "soc.h"

#if ENABLE_RTC

#define RTC_ENABLE	0x1
#define RTC_DISABLE	0x0

typedef OS_Driver RTC_Driver;

// Create a global instance of the RTC driver
RTC_Driver g_rtc_driver;

// Driver functions
static OS_Return _RTC_DriverRead(OS_Driver * driver, IO_Request * req);
static OS_Return _RTC_DriverWrite(OS_Driver * driver, IO_Request * req);

///////////////////////////////////////////////////////////////////////////////
// Basic driver functions
///////////////////////////////////////////////////////////////////////////////
OS_Return _RTC_DriverInit(OS_Driver * driver)
{
    // Validate the input
    ASSERT(driver);
    
    // Override the necessary functions
    driver->read = _RTC_DriverRead;
    driver->write = _RTC_DriverWrite;
    
    // Set Access permissions. User processes can only read date / time
    // The driver framework guarantees that access permissions are enforced
    driver->user_access_mask = ACCESS_READ;
	driver->admin_access_mask = ACCESS_READ | ACCESS_WRITE | ACCESS_EXCLUSIVE;
    
    return SUCCESS;
}

OS_Return _RTC_DriverRead(OS_Driver * driver, IO_Request * req)
{
	OS_Return status = BAD_ARGUMENT;
	
	ASSERT(driver && req);

	if(req->size >= sizeof(OS_DateAndTime)) {
		status = _OS_GetDateAndTime((OS_DateAndTime *) req->buffer);
		req->completed = sizeof(OS_DateAndTime);
		status = SUCCESS;
	}
	else if(req->size >= sizeof(OS_Time)) {
		status = _OS_GetTime((OS_Time *) req->buffer);		
		req->completed = sizeof(OS_Time);
		status = SUCCESS;
	}
	
    return status;
}

OS_Return _RTC_DriverWrite(OS_Driver * driver, IO_Request * req)
{
	OS_Return status = BAD_ARGUMENT;
	
	ASSERT(driver && req);

	if(req->size >= sizeof(OS_DateAndTime)) {
		status = _OS_SetDateAndTime((OS_DateAndTime *) req->buffer);
		req->completed = sizeof(OS_DateAndTime);
		status = SUCCESS;
	}
		
    return status;
}

///////////////////////////////////////////////////////////////////////////////
// Function to get date and time
///////////////////////////////////////////////////////////////////////////////
OS_Return _OS_GetDateAndTime(OS_DateAndTime *date_and_time)
{
    UINT32 i;
    
    ASSERT(date_and_time);

	// RTC Control enable
    rRTCCON = RTC_ENABLE;

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
	
	// RTC Control disable  
    rRTCCON = RTC_DISABLE;
    
    return SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// Functions to get date and time
///////////////////////////////////////////////////////////////////////////////
OS_Return _OS_GetTime(OS_Time *time)
{
    UINT32 i;

	ASSERT(time);
	
	// RTC Control enable
    rRTCCON = RTC_ENABLE;

    for(i = 0; i < 2; i++)
    {
		time->hour = (rBCDHOUR & 0x3f);
		time->min = (rBCDMIN & 0x7f);
		time->sec = (rBCDSEC & 0x7f);
		
		// If second is zero, we may have tripped minute and others. So we have to
		// read everything again.
		if(time->sec > 0) break;
	}
	
	// RTC Control disable  
    rRTCCON = RTC_DISABLE;	
    
    return SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// Functions to get date and time
///////////////////////////////////////////////////////////////////////////////
OS_Return _OS_SetDateAndTime(const OS_DateAndTime *date_and_time)
{
	ASSERT(date_and_time);
	
	if((date_and_time->year < 0 || date_and_time->year > 99)
	|| (date_and_time->mon < 1 || date_and_time->mon > 12)
	|| (date_and_time->date < 1 || date_and_time->date > 31)
	|| (date_and_time->day < 1 || date_and_time->day > 7)
	|| (date_and_time->hour < 0 || date_and_time->hour > 23)
	|| (date_and_time->min < 0 || date_and_time->min > 59)
	|| (date_and_time->sec < 0 || date_and_time->sec > 59))
	{
		return BAD_ARGUMENT;
	}
	
	// RTC Control enable
    rRTCCON = RTC_ENABLE;
    
    rBCDYEAR = (date_and_time->year & 0xff);
    rBCDMON  = (date_and_time->mon & 0x1f);
    rBCDDATE = (date_and_time->date & 0x3f);
    rBCDDAY  = (date_and_time->day & 0x07);
    rBCDHOUR = (date_and_time->hour & 0x3f);
    rBCDMIN  = (date_and_time->min & 0x7f);
    rBCDSEC  = (date_and_time->sec & 0x7f);
	
	// RTC Control disable  
    rRTCCON = RTC_DISABLE;
    
    return SUCCESS;
}

#if ENABLE_RTC_ALARM
OS_Return _OS_SetAlarm(const OS_DateAndTime *date_and_time)
{
	ASSERT(date_and_time);
	
	// TODO:
	panic("Not implemented");
	
	return NOT_SUPPORTED;
}

OS_Return _OS_GetAlarm(OS_DateAndTime *date_and_time)
{
	ASSERT(date_and_time);
	
	// TODO:
	panic("Not implemented");
	
	return NOT_SUPPORTED;
}
#endif // ENABLE_RTC_ALARM

#endif // ENABLE_RTC
