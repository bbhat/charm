///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	Uart.c
//	Author:	Bala B.
//	Description: MINI210S UART Serial Driver
//
//	TODO: Use Interrupts instead of delays
//  TODO: Have error handling by reading UERSTATn register
///////////////////////////////////////////////////////////////////////////////

#include "os_config.h"
#include "os_core.h"
#include "soc.h"
#include "uart.h"
#include "os_core.h"

static UINT8 Uart_init_status = 0;

#define GPA0CON 		( *((volatile unsigned long *)(ELFIN_GPIO_BASE + GPA0CON_OFFSET)) )

#define ULCON(ch) 		( *((volatile unsigned long *)(ELFIN_UART_BASE + ULCON_OFFSET + (ch << 10))) )
#define UCON(ch)			( *((volatile unsigned long *)(ELFIN_UART_BASE + UCON_OFFSET + (ch << 10))) )
#define UFCON(ch) 		( *((volatile unsigned long *)(ELFIN_UART_BASE + UFCON_OFFSET + (ch << 10))) )
#define UMCON(ch) 		( *((volatile unsigned long *)(ELFIN_UART_BASE + UMCON_OFFSET + (ch << 10))) )
#define UTRSTAT(ch) 	( *((volatile unsigned long *)(ELFIN_UART_BASE + UTRSTAT_OFFSET + (ch << 10))) )
#define UERSTAT(ch) 	( *((volatile unsigned long *)(ELFIN_UART_BASE + UERSTAT_OFFSET + (ch << 10))) )
#define UFSTAT(ch) 		( *((volatile unsigned long *)(ELFIN_UART_BASE + UFSTAT_OFFSET + (ch << 10))) )
#define UMSTAT(ch) 		( *((volatile unsigned long *)(ELFIN_UART_BASE + UMSTAT_OFFSET + (ch << 10))) )
#define UTXH(ch) 			( *((volatile unsigned long *)(ELFIN_UART_BASE + UTXH_OFFSET + (ch << 10))) )
#define URXH(ch) 			( *((volatile unsigned long *)(ELFIN_UART_BASE + URXH_OFFSET + (ch << 10))) )
#define UBRDIV(ch) 		( *((volatile unsigned long *)(ELFIN_UART_BASE + UBRDIV_OFFSET + (ch << 10))) )
#define UDIVSLOT(ch) 	( *((volatile unsigned long *)(ELFIN_UART_BASE + UDIVSLOT_OFFSET + (ch << 10))) )
#define UINTP (ch)			( *((volatile unsigned long *)(ELFIN_UART_BASE + UINTP_OFFSET + (ch << 10))) )
#define UINTSP(ch) 		( *((volatile unsigned long *)(ELFIN_UART_BASE + UINTSP_OFFSET + (ch << 10)) )
#define UINTM(ch) 		( *((volatile unsigned long *)(ELFIN_UART_BASE + UINTM_OFFSET + (ch << 10))) )

void Uart_Init(UART_Channel ch) 
{
	// Configure appropriate GPIO pins
	if(ch == UART0) {
		GPA0CON = (GPA0CON & 0xFFFF0000) | 0x00002222;
	}
	else (ch == UART0) {
		GPA0CON = (GPA0CON & 0x0000FFFF) | 0x22220000;
	}
	else {
		return;	// Invalid augument
	}

	// UART FIFO CONTROL REGISTER
	// FIFO Enable
	// TODO: Enable Trigger levels when Interrupts are enabled
	// Rx FIFO Trigger Level - 32-byte
	// Tx FIFO Trigger Level - 16-byte
	UFCON(ch) = 0x1; 

	// UART MODEM CONTROL REGISTER
	UMCON(ch) = 0;
	
	// UART LINE CONTROL REGISTER
	// 8-bits Word Length
	// One stop bit per frame
	// No Parity
	// Normal mode operation
	ULCON(ch) = 3;
	
	// UART CONTROL REGISTER
	// Rx & Tx Interrupt request or polling mode
	// Send Break Signal: Normal transmit
	// Loopback Mode: --
	// Disable receive error status interrupt
	// Disable Rx Time Out
	// Clock Selection - PCLK
	UCON(ch) = (1 << 0) | (1 << 2) | (UART_LOOPBACK_MODE << 5) | (0 << 10);
	
	// UART BAUD RATE DIVISOR REGISTER
	UBRDIV(ch) = ((PCLK / UART_BAUD_RATE) >> 4) - 1;
	UDIVSLOT(ch) = 1;
	
	// Rx & Tx FIFO Reset
	UFCON(ch) |= 0x06;

	// Wait till the reset is complete
	// TODO: Try to replace this with Sleep or something
	while(UFCON(ch) & 0x06);	//Auto-cleared after resetting FIFO	
	
	Uart_init_status |= (1 << ch);
}

void Uart_Print(UART_Channel ch, const INT8 *buf) 
{
	ASSERT(buf);
	ASSERT(Uart_init_status & (1 << ch));
	
	while(*buf)
	{
		// Wait till FIFO is not full
		while(UFSTAT(ch) & (1 << 24)) {
			// TODO: Do something useful here
		}
	
		UINT32 available = UART_FIFO_SIZE(ch) - ((UFSTAT(ch) >> 16) & 0xff);
		while(*buf && available--) {
			UTXH(ch) = *buf;
			buf++;
		}
	}
}

void Uart_Write(UART_Channel ch, const INT8 *buf, UINT32 count) 
{
	ASSERT(buf);
	ASSERT(Uart_init_status & (1 << ch));
	
	while(count) {
		// Wait till FIFO is not full
		while(UFSTAT(ch) & (1 << 24)) {
			// TODO: Do something useful here
		}
	
		UINT32 available = UART_FIFO_SIZE(ch) - ((UFSTAT(ch) >> 16) & 0xff);
		while(available--) {
			UTXH(ch) = *buf;
			buf++;
			count--;
		}
	}
}

void Uart_ReadB(UART_Channel ch, INT8 *buf, UINT32 count) 
{
	ASSERT(buf);
	ASSERT(Uart_init_status & (1 << ch));
	
	while(count) {		
		UINT32 available;
		while((available = (UFSTAT(ch) & 0xff)) == 0) {
			// TODO: Do something useful here
		}
	
		while(available--) {
			*buf = URXH(ch);
			buf++;
			count--;
		}
	}
}

void Uart_ReadNB(UART_Channel ch, INT8 *buf, UINT32 *count) 
{
	ASSERT(buf && count);
		
	UINT32 available = UFSTAT(ch) & 0xff;
	if (available > *count) {
		available = *count;
	}
	else {
		*count = available;
	}

	while(available--) {
		*buf = URXH(ch);
		buf++;
	}
}

// Non Blocking single ASCII character read. When there is no data available, it returns 0
INT8 Uart_GetChar(UART_Channel ch)
{
	ASSERT(Uart_init_status & (1 << ch));

	UINT32 available = UFSTAT(ch) & 0xff;
	if(available) {
		return URXH(ch);
	}
	
	return 0;
}

INT8 Uart_PutChar(UART_Channel ch, UINT8 data)
{
	ASSERT(Uart_init_status & (1 << ch));

	// Check if FIFO is full
	if(UFSTAT(ch) & (1 << 24)) {
		return 0;
	}
	
	// Write the character
	UTXH(ch) = data;
	
	return 1;
}
