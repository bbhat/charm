///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	serial.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Serial Driver
//	
///////////////////////////////////////////////////////////////////////////////

#ifndef _SERIAL_H_
#define _SERIAL_H_

#if SERIAL_DRIVER_ENABLE

#include "os_config.h"
#include "os_driver.h"

#if SERIAL_LOG_BUFFER_SIZE == 0
#error "SERIAL_LOG_BUFFER_SIZE should be > 0"
#endif

#if SERIAL_READ_ENABLED	
#if SERIAL_READ_BUFFER_SIZE == 0
#error "SERIAL_READ_BUFFER_SIZE should be > 0"
#endif
#endif

typedef struct Serial_driver {

	// Driver base struct as the first member
	OS_Driver	base;

	// Ring buffer used for storing output serial logs
	INT8 		output_buffer[SERIAL_LOG_BUFFER_SIZE];
	UINT32 		output_write_index;
	UINT32 		output_read_index;

#if SERIAL_READ_ENABLED	
	// Ringe buffer used for storing input keystrokes
	INT8 		input_buffer[SERIAL_READ_BUFFER_SIZE];
	UINT32 		input_write_index;
	UINT32 		input_read_index;
#endif
		
	// Serial task
	OS_Task_t serial_task;  
	UINT32 serial_task_stack [SERIAL_TASK_STACK_SIZE];

} Serial_driver;

///////////////////////////////////////////////////////////////////////////////
// Design
//	The serial driver creates a periodic task. Every time this task is invoked,
//	it flushes the write buffer into UART FIFO and reads available bytes in the 
//	UART FIFO into read buffer.
///////////////////////////////////////////////////////////////////////////////

// Global instance of the Serial driver
extern Serial_driver g_serial_driver;

///////////////////////////////////////////////////////////////////////////////
// Driver functions
///////////////////////////////////////////////////////////////////////////////
OS_Return _Serial_DriverInit(OS_Driver * driver);

#endif // SERIAL_DRIVER_ENABLE
#endif // _SERIAL_H_
