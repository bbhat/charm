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
		if(sdriver->input_read_index == sdriver->input_write_index)
		{
			break;
		}
	
		length = req->size - req->completed;
		if((sdriver->input_write_index < sdriver->input_read_index) && length)
		{
			length = MIN(length, SERIAL_READ_BUFFER_SIZE - sdriver->input_read_index);
			memcpy((INT8 *)req->buffer + req->completed, &sdriver->input_buffer[sdriver->input_read_index], length);

			sdriver->input_read_index = 0;
			req->completed += length;
			length = req->size - req->completed;			
		}
		
		if((sdriver->input_read_index < sdriver->input_write_index) && length)
		{
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
	if(sdriver->output_write_index > sdriver->output_read_index)
	{
		sdriver->output_read_index += Uart_DebugWriteNB(
			&sdriver->output_buffer[sdriver->output_read_index], 
			(sdriver->output_write_index - sdriver->output_read_index));			
	}
	else if(sdriver->output_write_index < sdriver->output_read_index)
	{
		// The ring buffer has wrapped around. Read it in two steps
		sdriver->output_read_index += Uart_DebugWriteNB(
			&sdriver->output_buffer[sdriver->output_read_index], 
			(SERIAL_LOG_BUFFER_SIZE - sdriver->output_read_index));
			
		if(sdriver->output_read_index == SERIAL_LOG_BUFFER_SIZE)
		{
			sdriver->output_read_index = 0;
			sdriver->output_read_index += Uart_DebugWriteNB(
				&sdriver->output_buffer[0], sdriver->output_write_index);			
		}			
	}
	else {
		// The ring buffer is empty when (g_serial_log_write_index == g_serial_log_read_index)
	}
	
#if SERIAL_READ_ENABLED		

	// Now buffer input keystrokes
	INT8 ch;
	UINT32 length;

	do 
	{
		// First read one character
		length = sizeof(ch);
		Uart_ReadNB(DEBUG_UART, &ch, &length);
		
		// If there are no input characters, exit the loop
		if(!length) break;
		
		// Store this character in the input buffer if there is space
		if((sdriver->input_write_index + 1) == sdriver->input_read_index)
		{
			break;	// Buffer is full, ignore the input character
		}
		else if((sdriver->input_read_index == 0) &&
				(sdriver->input_write_index == (SERIAL_READ_BUFFER_SIZE - 1)))
		{
			break;	// Buffer is full, ignore the input character
		}		

		sdriver->input_buffer[sdriver->input_write_index ++] = ch;
		
		// Update input_write_index
		if(sdriver->input_write_index == SERIAL_READ_BUFFER_SIZE)
		{
			sdriver->input_write_index = 0;
		}
			
	} while(TRUE);
	
#endif
}

UINT32 _Driver_SerialLog(Serial_driver * sdriver, const INT8 * str, UINT32 size)
{
	UINT32 free_count;
	UINT32 to_write;
	UINT32 written = 0;
	
	if(sdriver->output_write_index < sdriver->output_read_index)
	{
		// Ring buffer wastes one space so that we can identify queue full Vs queue empty
		free_count = (sdriver->output_read_index - sdriver->output_write_index - 1);
		to_write = MIN(free_count, size);		
		if(to_write)
		{
			memcpy(&sdriver->output_buffer[sdriver->output_write_index], str, to_write);
			sdriver->output_write_index += to_write;
		}
		written = to_write;
	}
	else if(sdriver->output_write_index >= sdriver->output_read_index)
	{
		if(sdriver->output_read_index > 0)
		{
			// In this case, we can write till the end of queue and then continue from the beginning 
			// of the queue again
			free_count = (SERIAL_LOG_BUFFER_SIZE - sdriver->output_write_index);
			to_write = MIN(free_count, size);
			if(to_write > 0)
			{
				memcpy(&sdriver->output_buffer[sdriver->output_write_index], str, to_write);
				sdriver->output_write_index += to_write;
			}
			written = to_write;
			
			if(sdriver->output_write_index == SERIAL_LOG_BUFFER_SIZE)
			{
				sdriver->output_write_index = 0;
				free_count = (sdriver->output_read_index - 1);
				to_write = MIN(free_count, size - written);
				if(to_write)
				{
					memcpy(&sdriver->output_buffer, &str[written], to_write);
					sdriver->output_write_index += to_write;
				}
				written += to_write;
			}
		}
		else	// g_serial_log_read_index == 0
		{
			free_count = (SERIAL_LOG_BUFFER_SIZE - sdriver->output_write_index - 1);
			to_write = MIN(free_count, size);
			if(to_write > 0)
			{
				memcpy(&sdriver->output_buffer[sdriver->output_write_index], str, to_write);
				sdriver->output_write_index += to_write;
			}
			written += to_write;
		}		
	}
	
	return written;
}

#endif // SERIAL_DRIVER_ENABLE
