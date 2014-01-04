///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	os_rtc.c
//	Author:	Bala bhat (bhat.balasubramanya@gmail.com)
//	Description: OS Test file
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_api.h"
#include "printf.h"


OS_Task_t task1;
UINT32 stack1 [0x400];
OS_Driver_t rtcd;

char input_str[64];

char weekdays[7][12] = { 
						"Sunday", 
						"Monday",
						"Tuesday",
						"Wednesday",
						"Thursday",
						"Friday",
						"Saturday"
					};

void task_rtc(void * ptr)
{
	OS_DateAndTime dt;
	UINT32 year, mon, date, day, hour, min, sec;
	int i;
	UINT32 length;
	
	// Get a handle to RTC driver used for console output
	if(OS_DriverLookup("RTC Driver", &rtcd) != SUCCESS) {
		printf("OS_DriverLookup failed\n");
		return;
	}
	
	// Open the driver in write mode
	if(OS_DriverOpen(rtcd, ACCESS_READ | ACCESS_WRITE | ACCESS_EXCLUSIVE) != SUCCESS) {
		printf("OS_DriverOpen failed\n");
		return;
	}

	while(1)
	{
		// First get the user command
		printf("\nInput command: \ns - Set Time\np - Print Time\nq - Quit\n# ");
		scanf("%s", input_str);
		
		// Skip all white spaces
		i = 0;
		while(input_str[i] == ' ' || input_str[i] == '\t') i++;
		
		switch(input_str[i])
		{
		case '\r':
		case '\n':
						
			break;
			
		case 's':
			printf("Please Input Year: ");
			scanf("%d", &year);
			if((year < 0 || year > 99))
			{
				goto error_exit;
			}

			printf("Please Input Month: ");
			scanf("%d", &mon);
			if(mon < 1 || mon > 12)
			{
				goto error_exit;
			}

			printf("Please Input Date: ");
			scanf("%d", &date);
			if(date < 1 || date > 31)
			{
				goto error_exit;
			}
	
			printf("Please Input Day[1:Sunday ... 7:Saturday]: ");
			scanf("%d", &day);
			if(day < 1 || day > 7)
			{
				goto error_exit;
			}

			printf("Please Input Hour [0 - 23]: ");
			scanf("%d", &hour);
			if(day < 0 || day > 23)
			{
				goto error_exit;
			}

			printf("Please Input Minutes [0 - 59]: ");
			scanf("%d", &min);
			if(min < 0 || min > 59)
			{
				goto error_exit;
			}

			printf("Please Input Seconds [0 - 59]: ");
			scanf("%d", &sec);
			if(sec < 0 || sec > 59)
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
				printf("Do you want to set the time ? [y/n]");
				scanf("%s", input_str);
				if((input_str[0]=='y') || (input_str[0]=='n'))
				{
					break;
				}
			}
			if(input_str[0]=='y')
			{
				length = sizeof(dt);
				if(OS_DriverWrite(rtcd, &dt, &length) == SUCCESS)
				{
					printf("Date and Time set successfully...\n");
				}
				else
				{
					printf("Date and Time set FAILED...\n");
					goto error_exit;
				}
			}
			
			break;
			
		case 'p':
			length = sizeof(dt);
			if(OS_DriverRead(rtcd, &dt, &length) != SUCCESS)
			{
				printf("Getting Date and Time FAILED...\n");
				goto error_exit;
			}
	
			printf("\n%s\t", weekdays[dt.day - 1]);
			printf("%d/%d/%d", dt.mon, dt.date, dt.year);
			printf("\t%d:%d:%d\n", dt.hour, dt.min, dt.sec);		
			
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
	printf("Critical Error. Exiting...\n");
}

int main(int argc, char *argv[])
{
/*
	printf("Calling test_rtc:%s\n",  __func__);
	
	// Get a handle to RTC driver used for console output
	if(OS_DriverLookup("RTC Driver", &rtcd) != SUCCESS) {
		return -1;
	}
	
	// Open the driver in write mode
	if(OS_DriverOpen(rtcd, ACCESS_READ | ACCESS_WRITE | ACCESS_EXCLUSIVE) != SUCCESS) {
		return -1;
	}
*/
		
	OS_CreateAperiodicTask(1, stack1, sizeof(stack1), "RTC", &task1, task_rtc, NULL);

	return 0;
}
