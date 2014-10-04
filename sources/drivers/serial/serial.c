///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	serial.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Serial Driver
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_config.h"
#include "os_core.h"
#include "uart.h"
#include "util.h"
#include "serial.h"

#if SERIAL_DRIVER_ENABLE

// Create a global instance of the RTC driver
Serial_driver g_serial_driver;

// Driver functions
static OS_Return _Serial_DriverRead(OS_Driver * driver, IO_Request * req);
static OS_Return _Serial_DriverWrite(OS_Driver * driver, IO_Request * req);
static UINT32 _Driver_SerialLog(Serial_driver * sdriver, const INT8 * str, UINT32 size);

static void SerialTaskFn(void * ptr);

#define MIN(a, b)		(((a) > (b)) ? (b) : (a))

///////////////////////////////////////////////////////////////////////////////
// Basic driver functions
///////////////////////////////////////////////////////////////////////////////
OS_Return _Serial_DriverInit(OS_Driver * driver)
{
	Serial_driver * sdriver = (Serial_driver *) driver;
	
    // Validate the input
    ASSERT(sdriver);
    
    // Override the necessary functions
    sdriver->base.read = _Serial_DriverRead;
    sdriver->base.write = _Serial_DriverWrite;
   
   	sdriver->output_write_index = 0;
   	sdriver->output_read_index = 0;

#if SERIAL_READ_ENABLED		
   	sdriver->input_write_index = 0;
   	sdriver->input_read_index = 0;
#endif
    
    // Create the Serial IO task 
    _OS_CreatePeriodicTask(SERIAL_TASK_PERIOD, SERIAL_TASK_PERIOD, 
        SERIAL_TASK_PERIOD / 40, 0, sdriver->serial_task_stack, 
        sizeof(sdriver->serial_task_stack), 
        "Serial", SYSTEM_TASK,
        &sdriver->serial_task, SerialTaskFn, sdriver);
    
    return SUCCESS;
}

OS_Return _Serial_DriverRead(OS_Driver * driver, IO_Request *req)
{
	UINT32 length;
	Serial_driver * sdriver = (Serial_driver *) driver;
	
	ASSERT(sdriver && req);

#if SERIAL_READ_ENABLED
	do
	{
		// Check if the buffer is empty
		if(sdriver->input_read_index == sdriver->input_write_index) {
			break;
		}
	
		length = req->size - req->completed;
		if((sdriver->input_write_index < sdriver->input_read_index) && length) {
			length = MIN(length, SERIAL_READ_BUFFER_SIZE - sdriver->input_read_index);
			memcpy((INT8 *)req->buffer + req->completed, &sdriver->input_buffer[sdriver->input_read_index], length);

			sdriver->input_read_index = 0;
			req->completed += length;
			length = req->size - req->completed;			
		}
		
		if((sdriver->input_read_index < sdriver->input_write_index) && length) {
			length = MIN(length, sdriver->input_write_index - sdriver->input_read_index);
			memcpy((INT8 *)req->buffer + req->completed, &sdriver->input_buffer[sdriver->input_read_index], length);
		
			// Update input_read_index. In this case, it will never exceed max buffer length
			sdriver->input_read_index += length;
			req->completed += length;
		
			// Update the length
			length = req->size - req->completed;
		}	
	} while(0);
#else
	panic("Serial read not configured. Enable SERIAL_READ_ENABLED");
#endif

	// For serial input we want to return data to the caller as and when it comes without waiting for
	// the entire buffer to be received
    return (req->completed > 0) ? SUCCESS : DEFER_IO_REQUEST;
}

OS_Return _Serial_DriverWrite(OS_Driver * driver, IO_Request *req)
{
	ASSERT(driver && req);

	req->completed += _Driver_SerialLog((Serial_driver *) driver, 
							(const INT8 *)req->buffer + req->completed, 
							(req->size - req->completed));
	
	return (req->completed < req->size) ? DEFER_IO_REQUEST : SUCCESS;
}

void SerialTaskFn(void * ptr)
{
	Serial_driver * sdriver = (Serial_driver *) ptr;
	ASSERT(sdriver);

	// First take care of output logs
	
	if(sdriver->output_write_index < sdriver->output_read_index) {
		// The ring buffer has wrapped around. Read it in two steps
		sdriver->output_read_index += Uart_DebugWriteNB(
			&sdriver->output_buffer[sdriver->output_read_index], 
			(SERIAL_LOG_BUFFER_SIZE - sdriver->output_read_index));
			
		if(sdriver->output_read_index == SERIAL_LOG_BUFFER_SIZE) {
			sdriver->output_read_index = 0;
		}			
	}

	if(sdriver->output_write_index > sdriver->output_read_index) {
		sdriver->output_read_index += Uart_DebugWriteNB(
			&sdriver->output_buffer[sdriver->output_read_index], 
			(sdriver->output_write_index - sdriver->output_read_index));
		
		_Driver_ResumeWriteRequest(&sdriver->base);
	}
	
#if SERIAL_READ_ENABLED		

	// Now buffer input keystrokes
	INT8 ch;
	UINT32 length;
	UINT32 total_length = 0;
	
	do {
		// Check if there is space
		if((sdriver->input_write_index + 1) == sdriver->input_read_index) {
			break;	// Buffer is full, ignore the input character
		}
		else if((sdriver->input_read_index == 0) &&
				(sdriver->input_write_index == (SERIAL_READ_BUFFER_SIZE - 1))) {
			break;	// Buffer is full, ignore the input character
		}		

		// Read one character
		length = sizeof(ch);
		Uart_ReadNB(DEBUG_UART, &ch, &length);
		
		// If there are no input characters, exit the loop
		if(!length) break;
		
		total_length++;
		sdriver->input_buffer[sdriver->input_write_index ++] = ch;
				
		// Echo the character back immediately. 
		Uart_DebugWriteNB(&ch, 1);

		// Update input_write_index
		if(sdriver->input_write_index == SERIAL_READ_BUFFER_SIZE) {
			sdriver->input_write_index = 0;
		}
	} while(TRUE);
	
	if(total_length) {	
		_Driver_ResumeReadRequest(&sdriver->base);
	}
	
#endif
}

UINT32 _Driver_SerialLog(Serial_driver * sdriver, const INT8 * str, UINT32 size)
{
	UINT32 available;
	UINT32 length;
	UINT32 remaining = size;
	
	if(sdriver->output_write_index >= sdriver->output_read_index) {
		available = (SERIAL_LOG_BUFFER_SIZE - sdriver->output_write_index);
		length = MIN(remaining, available);
		if(length) {
			memcpy(&sdriver->output_buffer[sdriver->output_write_index], str, length);
			sdriver->output_write_index += length;
			remaining -= length;
			str += length;
			
			// Wrap around if necessary
			if(sdriver->output_write_index == SERIAL_LOG_BUFFER_SIZE) {
				sdriver->output_write_index = 0;
			}
		}
	}
	
	if(sdriver->output_write_index < sdriver->output_read_index) {
		// Ring buffer wastes one space so that we can identify queue full Vs queue empty
		available = (sdriver->output_read_index - sdriver->output_write_index - 1);
		length = MIN(remaining, available);
		
		if(length) {
			memcpy(&sdriver->output_buffer[sdriver->output_write_index], str, length);
			sdriver->output_write_index += length;
			remaining -= length;
			str += length;
		}
	}

	return (size - remaining);
}

#endif // SERIAL_DRIVER_ENABLE
