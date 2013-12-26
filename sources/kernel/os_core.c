///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2012-2013 xxxxxxx, xxxxxxx
//	File:	os_core.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: 
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_core.h"
#include "os_config.h"
#include "uart.h"
#include "util.h"
#include "target.h"

///////////////////////////////////////////////////////////////////////////////
// Function to get the currently running thread. It returns a void pointer 
// which may be used as a Periodic / Aperiodic Task pointers
///////////////////////////////////////////////////////////////////////////////
void * OS_GetCurrentTask()
{
	return (void *)g_current_task;
}

///////////////////////////////////////////////////////////////////////////////
// Function to get the TBE count for the currently running task.
// It will be zero for an Aperiodic task.
///////////////////////////////////////////////////////////////////////////////
UINT32 OS_GetTBECount()
{
	OS_PeriodicTask * task = (OS_PeriodicTask *)g_current_task;
	return IS_PERIODIC_TASK(task->attributes) ? task->TBE_count : 0;
}

void panic(const INT8 * format, ...)
{
	// TODO: Need to print whole formatted string
	Uart_Print(DEBUG_UART_CHANNEL, format);
	
	// TODO: Implement this function
	while(1);
}

void SyslogStr(const INT8 * str, const INT8 * value)
{
	// TODO: OS Log should be handled using a different buffer to be fast
	if(str) 
	{
		Uart_Print(DEBUG_UART_CHANNEL, str);
	}
	
	if(value) 
	{
		Uart_Print(DEBUG_UART_CHANNEL, value);
	}
	
	if(str) 
	{
		Uart_Print(DEBUG_UART_CHANNEL, "\n");
	}
}

void Syslog32(const INT8 * str, UINT32 value)
{
	INT8 valueStr[12];
	
	if(str) 
	{
		Uart_Print(DEBUG_UART_CHANNEL, str);
	}
	
	itoa(value, valueStr);
	Uart_Print(DEBUG_UART_CHANNEL, valueStr);	
	
	if(str) 
	{
		Uart_Print(DEBUG_UART_CHANNEL, "\n");
	}
}

void Syslog64(const INT8 * str, UINT64 value)
{
	INT8 valueStr[20];

	if(str) 
	{
		Uart_Print(DEBUG_UART_CHANNEL, str);
	}
	
	itoa64(value, valueStr);
	Uart_Print(DEBUG_UART_CHANNEL, valueStr);	
	
	if(str) 
	{
		Uart_Print(DEBUG_UART_CHANNEL, "\n");
	}
}

///////////////////////////////////////////////////////////////////////////////
// _PFM_SetUserLED
// The led parameter indicates which LED should be turned ON/OFF/Toggled depending on 
// the options provided
///////////////////////////////////////////////////////////////////////////////
OS_Return _PFM_SetUserLED(LED_Number led, LED_Options options)
{
	OS_Return status = SUCCESS;

	switch(options)
	{
	case LED_ON:
		user_led_on(led);
		break;
	case LED_OFF:
		user_led_off(led);
		break;
	case LED_TOGGLE:
		user_led_toggle(led);
		break;
	default:
		status = BAD_ARGUMENT;
		break;
	}

	return status;
}
