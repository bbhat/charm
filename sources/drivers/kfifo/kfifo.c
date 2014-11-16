///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2014 xxxxxxx, xxxxxxx
//	File:	kfifo.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Kernel Fifo Driver
//	
///////////////////////////////////////////////////////////////////////////////

#include "kfifo.h"

#if ENABLE_KFIFO_DRIVER

// Create a global instance of the RTC driver
Kfifo_driver g_kfifo_driver;

// Driver functions
static OS_Return _Kfifo_DriverRead(OS_Driver * driver, IO_Request * req);
static OS_Return _Kfifo_DriverWrite(OS_Driver * driver, IO_Request * req);

///////////////////////////////////////////////////////////////////////////////
// Basic driver functions
///////////////////////////////////////////////////////////////////////////////
OS_Return _Kfifo_DriverInit(OS_Driver * driver, UINT32 fifo_depth)
{
	Kfifo_driver * kdriver = (Kfifo_driver *) driver;
	
    // Validate the inputs
    ASSERT(kdriver);
    ASSERT(fifo_depth >= 4);	// Minimum of 4 UINT32
    
    // Allocate buffer for the fifo
    fifo_buffer = (UINT32 *) kmalloc(fifo_depth * sizeof(UINT32));
    
    if(!fifo_buffer)
    {
    	return OUT_OF_SPACE;
    }
    
    // Override the necessary functions
    kdriver->base.read = _Kdriver_DriverRead;
    kdriver->base.write = _Kdriver_DriverWrite;
   
   	kdriver->write_index = 0;
   	kdriver->read_index = 0;
    
    return SUCCESS;
}

OS_Return _Serial_DriverRead(OS_Driver * driver, IO_Request *req)
{
	UINT32 length;
	Kfifo_driver * kdriver = (Kfifo_driver *) driver;
	
	ASSERT(kdriver && req);

	do
	{
		// Check if the buffer is empty
		if(kdriver->read_index == kdriver->write_index) {
			break;
		}
	
	    // Calculate the length in terms of number of words 
		length = (req->size - req->completed) >> 2;
		
		
		
		
		
		
		
		
		if((kdriver->write_index < kdriver->read_index) && length) {
			length = MIN(length, kdriver->fifo_buffer[kdriver->read_index++]);
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
