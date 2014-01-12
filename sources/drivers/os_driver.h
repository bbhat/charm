///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	driver.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Header file for common driver infrastructure
//	
///////////////////////////////////////////////////////////////////////////////

#ifndef _OS_DRIVER_H
#define _OS_DRIVER_H

///////////////////////////////////////////////////////////////////////////////
// Driver common structure
///////////////////////////////////////////////////////////////////////////////

#include "os_core.h"
#include "os_types.h"
#include "os_task.h"

// Max name size for kernel drivers. This should be at least 4
#define DRIVER_NAME_SIZE           16   

struct OS_Driver;
typedef OS_Return (*DriverFunction)(struct OS_Driver * driver, const void * argv[], 
									UINT32 argc, void * retv[], UINT32 retc);

// Following structure encapsulates an IO Request from the client. If there are multiple
// outstanding IO requests, they will be queued and processed in the order in which they arrive
typedef struct IO_Request
{
	struct IO_Request * next;	
	void * buffer;
	UINT32 size;
	UINT32 completed;				// Number of bytes completed transfer
	UINT32 attributes;
	
} IO_Request;

#define IO_TYPE_MASK			   0x80000000
#define WRITE_IO			       0x80000000
#define READ_IO			   		   0x00000000

typedef struct OS_Driver
{
	// Routines for Setting up the driver
	OS_Return (*init)(struct OS_Driver * driver);
	OS_Return (*start)(struct OS_Driver * driver);
	OS_Return (*stop)(struct OS_Driver * driver);
	
	// Routines called by driver clients	
	OS_Return (*open)(struct OS_Driver * driver);
	OS_Return (*close)(struct OS_Driver * driver);
	OS_Return (*read)(struct OS_Driver * driver, IO_Request * req);
	OS_Return (*write)(struct OS_Driver * driver, IO_Request * req);
	OS_Return (*configure)(struct OS_Driver * driver, const void * buffer, UINT32 size);
	
	// Interrupt Routines
	void (*primary_int_handler)(struct OS_Driver * driver);
	void (*secondary_int_handler)(struct OS_Driver * driver);
	
	// Custom functions for each driver
	DriverFunction *driver_functions;
	UINT32 driver_functions_count;
	
	// IO task
	OS_Task * io_task;
	
	// Queue for outstanding IOs
	IO_Request *io_queue_head;
	IO_Request *io_queue_tail;
	
	// Queue for free IO requests
	IO_Request *free_io_queue;
	
    // Driver Name
    INT8 name[DRIVER_NAME_SIZE];
    OS_Process *owner_process;          // Process that has currently opened this driver in exclusive mode
    UINT8 user_access_mask;
    UINT8 admin_access_mask;
    UINT8 usage_mode;
    UINT8 open_clients;					// Number of clients who has opened this driver
	
} OS_Driver;

// The following function is called to register a driver the the OS. 
// The name should be at least 4 characters and those 4 characters should be unique.
// This is because we use the first 4 characters as a number so that we can easily search
// for a diver using its name without using expensive string search operations.
// The 'max_io_count' corresponds to the maximum number of IOs that can be pending in this driver
OS_Return _OS_DriverInit(OS_Driver *driver, const INT8 name[], OS_Return (*init)(OS_Driver *), UINT32 max_io_count);

// Functions to start and stop the driver
OS_Return _OS_DriverStart(OS_Driver *driver);
OS_Return _OS_DriverStop(OS_Driver *driver);

// Client interfacing functions
OS_Return _OS_DriverLookup(const INT8 * name, OS_Driver_t * driver);
OS_Return _OS_DriverOpen(OS_Driver_t driver, OS_DriverAccessMode mode);
OS_Return _OS_DriverClose(OS_Driver_t driver);

// Driver Read/Write routines. The framework supports only non-blocking calls (for now).
// The call always returns immediately. If the driver is busy / needs more time,
// the IO request will be automatically queued. The driver picks up each pending request 
// and completes them one by one.
// The objective is to encourage non-blocking design as much as possible.
// Blocking read/writes interferes with corresponding task's periodic behavior
OS_Return _OS_DriverRead(OS_Driver_t driver, void * buffer, UINT32 * size);
OS_Return _OS_DriverWrite(OS_Driver_t driver, const void * buffer, UINT32 * size);
OS_Return _OS_DriverConfigure(OS_Driver_t driver, const void * buffer, UINT32 size);

// Functions called by the IO Task of the driver
void _Driver_ProcessNextIORequest(OS_Driver * driver);

#endif // _OS_DRIVER_H
