///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	serial.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Serial logging functions
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_config.h"
#include "os_core.h"
#include "uart.h"
#include "util.h"

#if WITH_SERIAL_LOGGING_TASK==1

// Ring buffer used for storing the serial logs
static UINT8 g_serial_log_buffer[SERIAL_LOG_BUFFER_SIZE];
static UINT32 g_serial_log_write_index;
static UINT32 g_serial_log_read_index;

OS_AperiodicTask * g_serial_task;  // A TCB for the serial task
static UINT32 g_serial_task_stack [SERIAL_TASK_STACK_SIZE];
static void SerialTaskFn(void * ptr);

#define MIN(a, b)		(((a) > (b)) ? (b) : (a))

void init_serial()
{
    // Create the Serial task 
    OS_CreatePeriodicTask(SERIAL_TASK_PERIOD, SERIAL_TASK_PERIOD, 
        SERIAL_TASK_PERIOD / 40, 0, g_serial_task_stack, sizeof(g_serial_task_stack), 
        "SERIAL", 
        &g_serial_task, SerialTaskFn, 0);
}

void SerialTaskFn(void * ptr)
{
	if(g_serial_log_write_index > g_serial_log_read_index)
	{
		g_serial_log_read_index += Uart_DebugWriteNB(
			&g_serial_log_buffer[g_serial_log_read_index], 
			(g_serial_log_write_index - g_serial_log_read_index));			
	}
	else if(g_serial_log_write_index < g_serial_log_read_index)
	{
		// The ring buffer has wrapped around. Read it in two steps
		g_serial_log_read_index += Uart_DebugWriteNB(
			&g_serial_log_buffer[g_serial_log_read_index], 
			(SERIAL_LOG_BUFFER_SIZE - g_serial_log_read_index));
			
		if(g_serial_log_read_index == SERIAL_LOG_BUFFER_SIZE)
		{
			g_serial_log_read_index = 0;
			g_serial_log_read_index += Uart_DebugWriteNB(
				&g_serial_log_buffer, g_serial_log_write_index);			
		}			
	}
	
	// The ring buffer is empty when (g_serial_log_write_index == g_serial_log_read_index)
}

UINT32 _PFM_SerialLog(const INT8 * str, UINT32 size)
{
	UINT32 free_count;
	UINT32 to_write;
	UINT32 written;
	
	if(g_serial_log_write_index < g_serial_log_read_index)
	{
		// Ring buffer wastes one space so that we can identify queue full Vs queue empty
		free_count = (g_serial_log_read_index - g_serial_log_write_index - 1);
		to_write = MIN(free_count, size);		
		if(to_write)
		{
			memcpy(&g_serial_log_buffer[g_serial_log_write_index], str, to_write);
			g_serial_log_write_index += to_write;
		}
		written = to_write;
	}
	else if(g_serial_log_write_index >= g_serial_log_read_index)
	{
		if(g_serial_log_read_index > 0)
		{
			// In this case, we can write till the end of queue and then continue from the beginning 
			// of the queue again
			free_count = (SERIAL_LOG_BUFFER_SIZE - g_serial_log_write_index);
			to_write = MIN(free_count, size);
			if(to_write > 0)
			{
				memcpy(&g_serial_log_buffer[g_serial_log_write_index], str, to_write);
				g_serial_log_write_index += to_write;
			}
			written = to_write;
			
			if(g_serial_log_write_index == SERIAL_LOG_BUFFER_SIZE)
			{
				g_serial_log_write_index = 0;
				free_count = (g_serial_log_read_index - 1);
				to_write = MIN(free_count, size - written);
				if(to_write)
				{
					memcpy(&g_serial_log_buffer, &str[written], to_write);
					g_serial_log_write_index += to_write;
				}
				written += to_write;
			}
		}
		else	// g_serial_log_read_index == 0
		{
			free_count = (SERIAL_LOG_BUFFER_SIZE - g_serial_log_write_index - 1);
			to_write = MIN(free_count, size);
			if(to_write > 0)
			{
				memcpy(&g_serial_log_buffer[g_serial_log_write_index], str, to_write);
				g_serial_log_write_index += to_write;
			}
			written += to_write;
		}		
	}
	
	return written;
}

#endif // WITH_SERIAL_LOGGING_TASK
