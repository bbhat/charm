#include "vsprintf.h"
#include "string.h"
#include "printf.h"
#include "os_api.h"

extern void putc(unsigned char c);
extern unsigned char getc(void);

#define	OUTBUFSIZE	512
#define	INBUFSIZE	512


static char g_pcOutBuf[OUTBUFSIZE];
static char g_pcInBuf[INBUFSIZE];

int printf(const char *fmt, ...)
{
	int i;
	int len;
	va_list args;

	va_start(args, fmt);
	len = vsprintf(g_pcOutBuf,fmt,args);
	va_end(args);
	
	return PFM_SerialLog(g_pcOutBuf, len);
}

int scanf(const char * fmt, ...)
{
	int i = 0;
	
	// TODO: Implement this
// 	unsigned char c;
// 	va_list args;
// 	
// 	while(1)
// 	{
// 		c = getc();
// 		putc(c);
// 		if((c == 0x0d) || (c == 0x0a))
// 		{
// 			g_pcInBuf[i] = '\0';
// 			break;
// 		}
// 		else
// 		{
// 			g_pcInBuf[i++] = c;
// 		}
// 	}
// 	
// 	va_start(args,fmt);
// 	i = vsscanf(g_pcInBuf,fmt,args);
// 	va_end(args);

	return i;
}

