///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	os_rtc.c
//	Author:	Bala bhat (bhat.balasubramanya@gmail.com)
//	Description: OS Test file
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_core.h"
#include "target.h"
#include "util.h"

OS_PeriodicTask task1;
OS_PeriodicTask task2;
OS_PeriodicTask task3;
OS_AperiodicTask task4;
OS_Sem input_ready;
char input_str[48];

char weekdays[7][12] = { 
						"Sunday", 
						"Monday",
						"Tuesday",
						"Wednesday",
						"Thursday",
						"Friday",
						"Saturday"
					};

void task_input(void * ptr)
{
	static int len = 0;
	static char buffer[48];
	char ch = getchar();
	
	if(ch != 0)
	{
		putchar(ch);	// Echo back the character
		
		// Toggle the LED for fun
		user_led_toggle(*(int *)ptr);
		
		// Ignore the initial whitespace
		if((len == 0) && (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n'))
		{
			return;		// Ignore the characters
		}
		
		if(ch == '\r' || ch == '\n')
		{
			buffer[len++] = '\0';
			strcpy(input_str, buffer);
			len = 0;
			OS_SemPost(&input_ready);
			return;
		}
		
		buffer[len++] = ch;
		
		if((len+1) == sizeof(input_str))
		{
			buffer[len++] = '\0';
			strcpy(input_str, buffer);
			len = 0;
			OS_SemPost(&input_ready);
			return;
		}
	}
}

static int test_counter = 0;
void task_count(void * ptr)
{
	test_counter++;
	user_led_toggle(*(int *)ptr);
}

void task_3(void * ptr)
{
	user_led_toggle(*(int *)ptr);
}

void task_rtc(void * ptr)
{
	OS_DateAndTime dt;
	UINT32 year, mon, date, day, hour, min, sec;
	char bcd_str[20];
	
	while(1)
	{
		// First get the user command
		Syslog("Input any command: \ns - Set Time\np - Print Time\nq - Quit");
		OS_SemWait(&input_ready);
		
		switch(input_str[0])
		{
		case 's':
			Syslog("Please Input Year: ");
			OS_SemWait(&input_ready);
			if(!bcda2bcdi(input_str, &year) && (year < 0 || year > 99))
			{
				goto error_exit;
			}

			Syslog("Please Input Month: ");
			OS_SemWait(&input_ready);
			if(!bcda2bcdi(input_str, &mon) && (mon < 1 || mon > 12))
			{
				goto error_exit;
			}

			Syslog("Please Input Date: ");
			OS_SemWait(&input_ready);
			if(!bcda2bcdi(input_str, &date) && (date < 1 || date > 31))
			{
				goto error_exit;
			}
	
			Syslog("Please Input Day[1:Sunday ... 7:Saturday]: ");
			OS_SemWait(&input_ready);
			if(!bcda2bcdi(input_str, &day) && (day < 1 || day > 7))
			{
				goto error_exit;
			}

			Syslog("Please Input Hour [0 - 23]: ");
			OS_SemWait(&input_ready);
			if(!bcda2bcdi(input_str, &hour) && (day < 0 || day > 23))
			{
				goto error_exit;
			}

			Syslog("Please Input Minutes [0 - 59]: ");
			OS_SemWait(&input_ready);
			if(!bcda2bcdi(input_str, &min) && (min < 0 || min > 59))
			{
				goto error_exit;
			}

			Syslog("Please Input Seconds [0 - 59]: ");
			OS_SemWait(&input_ready);
			if(!bcda2bcdi(input_str, &sec) && (sec < 0 || sec > 59))
			{
				goto error_exit;
			}
	
			// Copy into OS_DateAndTime structure
			dt.year = year;
			dt.mon = mon;
			dt.date = date;
			dt.day = day;
			dt.hour = hour;
			dt.min = min;
			dt.sec = sec;
	
			while(1)
			{
				Syslog("Do you want to set the time ? [y/n]");
				OS_SemWait(&input_ready);
				if((input_str[0]=='y') || (input_str[0]=='n'))
				{
					break;
				}
			}
			if(input_str[0]=='y')
			{
				if(OS_SetDateAndTime(&dt) == SUCCESS)
				{
					Syslog("Date and Time set successfully...");
				}
				else
				{
					Syslog("Date and Time set FAILED...");
					goto error_exit;
				}
			}
			
			break;
			
		case 'p':
			if(OS_GetDateAndTime(&dt) != SUCCESS)
			{
				Syslog("Getting Date and Time FAILED...");
				goto error_exit;
			}
		
			Syslog("\n\n");
			SyslogStr(NULL, weekdays[dt.day - 1]);
			SyslogStr(NULL, "\t");
		
			bcdi2bcda(dt.mon, bcd_str);
			SyslogStr(NULL, bcd_str);
			SyslogStr(NULL, "/");
		
			bcdi2bcda(dt.date, bcd_str);
			SyslogStr(NULL, bcd_str);
			SyslogStr(NULL, "/");
		
			bcdi2bcda(dt.year, bcd_str);
			SyslogStr(NULL, bcd_str);
			SyslogStr(NULL, " ");
		
			bcdi2bcda(dt.hour, bcd_str);
			SyslogStr(NULL, bcd_str);
			SyslogStr(NULL, ":");
		
			bcdi2bcda(dt.min, bcd_str);
			SyslogStr(NULL, bcd_str);
			SyslogStr(NULL, ":");
		
			bcdi2bcda(dt.sec, bcd_str);
			SyslogStr("", bcd_str);
			
			Syslog("\n");
			
#if OS_ENABLE_CPU_STATS==1
			Syslog32("Current SYNC Counter Value - ", sync_intr_counter);
#endif 			
			Syslog32("Current Counter Value - ", test_counter);
			Syslog("\n");
			
			break;
		case 'q':
			goto exit;
		default:
			break;			
		}
	}
	
exit:
	return;
error_exit:
	Syslog("Critical Error. Exiting...");
}

UINT32 stack1 [0x400];
UINT32 stack2 [0x400];
UINT32 stack3 [0x400];
UINT32 stack4 [0x400];

int a = 0;
int b = 1;
int c = 2;
int d = 3;

int main(int argc, char *argv[])
{
	OS_SemInit(&input_ready, 0, 0);

	SyslogStr("Calling - ",  __func__);

	OS_CreatePeriodicTask( 100000, 100000, 20000, 0, stack1, sizeof(stack1), "LED1", &task1, task_input, &a);
	OS_CreatePeriodicTask( 10000, 10000, 3000, 1000, stack2, sizeof(stack2), "LED2", &task2, task_count, &b);
	OS_CreatePeriodicTask( 20000, 20000, 5000, 4000, stack3, sizeof(stack3), "LED3", &task3, task_3, &c);
	OS_CreateAperiodicTask(1, stack4, sizeof(stack4), "Menu", &task4, task_rtc, &d);

	return 0;
}
