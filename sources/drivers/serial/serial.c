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
   
   	sdriver->write_index = 0;
   	sdriver->read_index = 0;
    
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
	
	ASSERT(driver && req);

	// At this point, this driver only supports non-blocking read.
	
	length = req->size;
	Uart_ReadNB(DEBUG_UART, (const INT8 *)req->buffer + req->completed, &length);
	req->completed += length;	
	
    return SUCCESS;
}

OS_Return _Serial_DriverWrite(OS_Driver * driver, IO_Request *req)
{
	OS_Return status = DEFER_IO_REQUEST;
	
	ASSERT(driver && req);

	req->completed += _Driver_SerialLog((Serial_driver *) driver, 
							(const INT8 *)req->buffer + req->completed, 
							(req->size - req->completed));
	
	if(req->completed == req->size) {
		status = SUCCESS;
	}
	else {
		panic("Not implemented");
	}
	
    return status;
}

void SerialTaskFn(void * ptr)
{
	Serial_driver * sdriver = (Serial_driver *) ptr;
	ASSERT(sdriver);
		
	if(sdriver->write_index > sdriver->read_index)
	{
		sdriver->read_index += Uart_DebugWriteNB(
			&sdriver->log_buffer[sdriver->read_index], 
			(sdriver->write_index - sdriver->read_index));			
	}
	else if(sdriver->write_index < sdriver->read_index)
	{
		// The ring buffer has wrapped around. Read it in two steps
		sdriver->read_index += Uart_DebugWriteNB(
			&sdriver->log_buffer[sdriver->read_index], 
			(SERIAL_LOG_BUFFER_SIZE - sdriver->read_index));
			
		if(sdriver->read_index == SERIAL_LOG_BUFFER_SIZE)
		{
			sdriver->read_index = 0;
			sdriver->read_index += Uart_DebugWriteNB(
				&sdriver->log_buffer[0], sdriver->write_index);			
		}			
	}
	
	// The ring buffer is empty when (g_serial_log_write_index == g_serial_log_read_index)
}

UINT32 _Driver_SerialLog(Serial_driver * sdriver, const INT8 * str, UINT32 size)
{
	UINT32 free_count;
	UINT32 to_write;
	UINT32 written = 0;
	
	if(sdriver->write_index < sdriver->read_index)
	{
		// Ring buffer wastes one space so that we can identify queue full Vs queue empty
		free_count = (sdriver->read_index - sdriver->write_index - 1);
		to_write = MIN(free_count, size);		
		if(to_write)
		{
			memcpy(&sdriver->log_buffer[sdriver->write_index], str, to_write);
			sdriver->write_index += to_write;
		}
		written = to_write;
	}
	else if(sdriver->write_index >= sdriver->read_index)
	{
		if(sdriver->read_index > 0)
		{
			// In this case, we can write till the end of queue and then continue from the beginning 
			// of the queue again
			free_count = (SERIAL_LOG_BUFFER_SIZE - sdriver->write_index);
			to_write = MIN(free_count, size);
			if(to_write > 0)
			{
				memcpy(&sdriver->log_buffer[sdriver->write_index], str, to_write);
				sdriver->write_index += to_write;
			}
			written = to_write;
			
			if(sdriver->write_index == SERIAL_LOG_BUFFER_SIZE)
			{
				sdriver->write_index = 0;
				free_count = (sdriver->read_index - 1);
				to_write = MIN(free_count, size - written);
				if(to_write)
				{
					memcpy(&sdriver->log_buffer, &str[written], to_write);
					sdriver->write_index += to_write;
				}
				written += to_write;
			}
		}
		else	// g_serial_log_read_index == 0
		{
			free_count = (SERIAL_LOG_BUFFER_SIZE - sdriver->write_index - 1);
			to_write = MIN(free_count, size);
			if(to_write > 0)
			{
				memcpy(&sdriver->log_buffer[sdriver->write_index], str, to_write);
				sdriver->write_index += to_write;
			}
			written += to_write;
		}		
	}
	
	return written;
}

#endif // SERIAL_DRIVER_ENABLE
