#include "vsprintf.h"
#include "string.h"
#include "printf.h"
#include "os_api.h"

#define	OUTBUFSIZE	512
#define	INBUFSIZE	512

extern OS_Driver_t __console_serial_driver__;

static char g_pcOutBuf[OUTBUFSIZE];
static char g_pcInBuf[INBUFSIZE];

int printf(const char *fmt, ...)
{
	int len;
	va_list args;

	va_start(args, fmt);
	len = vsprintf(g_pcOutBuf,fmt,args);
	va_end(args);
	
	return OS_DriverWrite(__console_serial_driver__, g_pcOutBuf, &len, FALSE);
}

int puts (const char * str)
{
	int len = strlen(str);
	return OS_DriverWrite(__console_serial_driver__, str, &len, FALSE);
}

int scanf(const char * fmt, ...)
{
	int eol = 0;
	int i = 0;
	int j;
	
	unsigned char str[64];
	va_list args;
	
	do
	{
		int len = sizeof(str) - 1;
		OS_Return ret = OS_DriverRead(__console_serial_driver__, str, &len, TRUE);
		for(j = 0; ((j < len) && !eol); j++)
		{
			eol = ((str[j] == 0x0d) || (str[j] == 0x0a));
			g_pcInBuf[i++] = str[j];
		}
	}
	while (!eol);
		
	g_pcInBuf[i-1] = '\0';
	
	va_start(args,fmt);
	i = vsscanf(g_pcInBuf,fmt,args);
	va_end(args);

	return i;
}
