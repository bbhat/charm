///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	uart.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: S3C2440 UART Serial Driver
//	
///////////////////////////////////////////////////////////////////////////////

#ifndef _UART_H_
#define _UART_H_

#include "os_types.h"

typedef enum {
	UART0,
	UART1,
	UART2
	} UART_Channel;
	
#define UART_BAUD_RATE	115200
#define UART_LOOPBACK_MODE	0

#if defined(SOC_S5PV210)
	#define UART_FIFO_SIZE(ch)	((ch == UART0) ? 256 : 64)
#elif defined(SOC_ARM920T)
	#define UART_FIFO_SIZE	64
#endif

void Uart_Init(UART_Channel ch);
void Uart_Print(UART_Channel ch, const INT8 *buf);
void Uart_Write(UART_Channel ch, const INT8 *buf, UINT32 count);
void Uart_ReadB(UART_Channel ch, INT8 *buf, UINT32 count);	// Blocking read
void Uart_ReadNB(UART_Channel ch, INT8 *buf, UINT32 * count);	// Non Blocking Read

// Non Blocking single ASCII character read
INT8 Uart_GetChar(UART_Channel ch);	

// Non Blocking single ASCII character write. Returns the number of characters written (0/1)
INT8 Uart_PutChar(UART_Channel ch, UINT8 data);	

#endif // _UART_H_
