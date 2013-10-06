///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	uart.c
//	Author:	Bala B.
//	Description: S3C2440 UART Serial Driver
//
//	TODO: Use Interrupts instead of delays
//  TODO: Have error handling by reading UERSTATn register
///////////////////////////////////////////////////////////////////////////////

#include "os_config.h"
#include "os_core.h"
#include "soc.h"
#include "uart.h"
#include "target.h"
#include "os_core.h"

static UINT8 Uart_init_status = 0;

#define MIN(a, b)		(((a) > (b)) ? (b) : (a))

void Uart_Init(UART_Channel ch) 
{
	// UART LINE CONTROL REGISTER
	// 8-bits Word Length
	// One stop bit per frame
	// No Parity
	// Normal mode operation
	rULCON(ch) = 3;
	
	// UART CONTROL REGISTER
	// Rx & Tx Interrupt request or polling mode
	// Send Break Signal: Normal transmit
	// Loopback Mode: --
	// Disable receive error status interrupt
	// Disable Rx Time Out
	// Clock Selection - PCLK
	rUCON(ch) = (1 << 0) | (1 << 2) | (UART_LOOPBACK_MODE << 5) | (2 << 10);
	
	// UART FIFO CONTROL REGISTER
	// FIFO Enable
	// TODO: Enable Trigger levels when Interrupts are enabled
	// Rx FIFO Trigger Level - 32-byte
	// Tx FIFO Trigger Level - 16-byte
	rUFCON(ch) = 1; 
	
	// UART MODEM CONTROL REGISTER
	rUMCON(ch) = 0;
	
	// UART BAUD RATE DIVISOR REGISTER
	rUBRDIV(ch) = ((PCLK / UART_BAUD_RATE) >> 4) - 1;
	
	// Rx & Tx FIFO Reset
	rUFCON(ch) |= 0x06;

	// Wait till the reset is complete
	// TODO: Try to replace this with Sleep or something
	while(rUFCON(ch) & 0x06);	//Auto-cleared after resetting FIFO	
	
	Uart_init_status |= (1 << ch);
}

void Uart_Print(UART_Channel ch, const INT8 *buf) 
{
	UINT32 intsts;
	
	ASSERT(buf);
	ASSERT(Uart_init_status & (1 << ch));

	while(*buf)
	{
		// Wait till FIFO is not full
		while(rUFSTAT(ch) & (1 << 14)) {
			// TODO: Do something useful here
		}
		
		OS_ENTER_CRITICAL(intsts);
		UINT32 available = UART_FIFO_SIZE - ((rUFSTAT(ch) >> 8) & 0x3f);
		while(*buf && available--)
		{
			rUTXH(ch) = *buf;
			buf++;
		}
		OS_EXIT_CRITICAL(intsts);
	}
}

void Uart_Write(UART_Channel ch, const INT8 *buf, UINT32 count) 
{
	UINT32 intsts;
	
	ASSERT(buf);
	ASSERT(Uart_init_status & (1 << ch));
	
	while(count)
	{
		// Wait till FIFO is not full
		while(rUFSTAT(ch) & (1 << 14)) {
			// TODO: Do something useful here
		}
	
		OS_ENTER_CRITICAL(intsts);
		UINT32 available = UART_FIFO_SIZE - ((rUFSTAT(ch) >> 8) & 0x3f);
		while(available--)
		{
			rUTXH(ch) = *buf;
			buf++;
			count--;
		}
		OS_EXIT_CRITICAL(intsts);
	}
}

UINT32 Uart_DebugWriteNB(const INT8 *buf, UINT32 count) 
{
	UINT32 intsts;
	
	ASSERT(buf);
	ASSERT(Uart_init_status & (1 << DEBUG_UART));
	
	OS_ENTER_CRITICAL(intsts);
	UINT32 available = UART_FIFO_SIZE - ((rUFSTAT(DEBUG_UART) >> 8) & 0x3f);
	UINT32 actual_count = MIN(count, available);
	UINT32 written = actual_count;

	while(actual_count--)
	{
		rUTXH(DEBUG_UART) = *buf;
		buf++;
	}
	OS_EXIT_CRITICAL(intsts);
	
	return written;
}

void Uart_ReadB(UART_Channel ch, INT8 *buf, UINT32 count) 
{
	UINT32 intsts;
	
	ASSERT(buf);
	ASSERT(Uart_init_status & (1 << ch));
	
	while(count)
	{		
		UINT32 available;
		while((available = (rUFSTAT(ch) & 0x3f)) == 0) {
			// TODO: Do something useful here
		}
	
		OS_ENTER_CRITICAL(intsts);
		available = (rUFSTAT(ch) & 0x3f);
		while(available--)
		{
			*buf = rURXH(ch);
			buf++;
			count--;
		}
		OS_EXIT_CRITICAL(intsts);
	}
}

void Uart_ReadNB(UART_Channel ch, INT8 *buf, UINT32 *count) 
{
	UINT32 intsts;
	
	ASSERT(buf && count);
	
	OS_ENTER_CRITICAL(intsts);
	UINT32 available = rUFSTAT(ch) & 0x3f;
	if (available > *count)
	{
		available = *count;
	}
	else 
	{
		*count = available;
	}

	while(available--)
	{
		*buf = rURXH(ch);
		buf++;
	}
	OS_EXIT_CRITICAL(intsts);
}

// Non Blocking single ASCII character read. When there is no data available, it returns 0
INT8 Uart_GetChar(UART_Channel ch)
{
	UINT32 intsts;
	ASSERT(Uart_init_status & (1 << ch));
	
	OS_ENTER_CRITICAL(intsts);
	UINT32 available = rUFSTAT(ch) & 0x3f;
	if(available)
	{
		UINT8 data = rURXH(ch);
		OS_EXIT_CRITICAL(intsts);
		
		return data;
	}
	OS_EXIT_CRITICAL(intsts);
	
	return 0;
}

INT8 Uart_PutChar(UART_Channel ch, UINT8 data)
{
	UINT32 intsts;
	ASSERT(Uart_init_status & (1 << ch));
	
	OS_ENTER_CRITICAL(intsts);
	// Check if FIFO is full
	if(rUFSTAT(ch) & (1 << 14)) {
		OS_EXIT_CRITICAL(intsts);
		return 0;
	}
	
	// Write the character
	rUTXH(ch) = data;
	OS_EXIT_CRITICAL(intsts);
	
	return 1;
}
