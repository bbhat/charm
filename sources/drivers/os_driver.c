///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	driver.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Header file for common driver infrastructure
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_driver.h"
#include "os_config.h"
#include "util.h"

typedef struct 
{
    UINT32        id;
    OS_Driver     *driver;
    
} KernelDriverEntry;

static KernelDriverEntry g_kernel_drivers[MAX_KERNEL_DRIVERS];
static UINT32 g_kernel_driver_count = 0;

#ifdef _USE_STD_LIBS
	#define FAULT(x, ...) printf(x, ...);
#else
	#define FAULT(x, ...)
#endif

///////////////////////////////////////////////////////////////////////////////
// The following function is called to register a driver the the OS. 
// The name should be at least 4 characters and those 4 characters should be unique.
// This is because we use the first 4 characters as a number so that we can easily search
// for a diver using its name without using expensive string search operations.
///////////////////////////////////////////////////////////////////////////////
OS_Return _OS_DriverInit(OS_Driver *driver, const INT8 name[], OS_Return (*init)(OS_Driver *))
{
	// Validate the arguments
	ASSERT(driver && init);
	
	// Ensure that we are not out of Kernel Driver Quota
	ASSERT(g_kernel_driver_count < MAX_KERNEL_DRIVERS);
	
	// Now ensure that there are no duplicates by the ID (first 4 characters of the driver name)
	if(_OS_DriverLookup(name) != NULL)
	{
	    FAULT("Duplicate driver name(%s). Only first 4 characters are considered.", name);
		return INVALID_ARG;	
	}
	
	// Set the driver Init function
	driver->init = init;
	
	// Reset the remaining function pointers
	driver->start = NULL;
	driver->stop = NULL;
	driver->open = NULL;
	driver->close = NULL;
	driver->read = NULL;
	driver->write = NULL;
	driver->configure = NULL;
	driver->primary_int_handler = NULL;
	driver->secondary_int_handler = NULL;
	driver->driver_functions = NULL;
	driver->driver_functions_count = 0;

    // Copy the driver name	
	strncpy(driver->name, name, DRIVER_NAME_SIZE - 1);
	driver->name[DRIVER_NAME_SIZE - 1] = '\0';
	
	// Call the driver 'init' function. This function should initialize all
	// other function pointers in the driver obect
	OS_Return result = driver->init(driver);
	
	if(result == SUCCESS)
	{
	    // Copy the driver id by converting the first 4 characters of the driver name
	    g_kernel_drivers[g_kernel_driver_count].id = *(UINT32 *)driver->name;
	    g_kernel_drivers[g_kernel_driver_count].driver = driver;
	    g_kernel_driver_count++;
	}
	
	return result;
}

OS_Return _OS_DriverStart(OS_Driver *driver)
{
	// Validate the arguments
	ASSERT(driver);
	
	// Also ensure that the 'start' method is provided
	ASSERT(driver->start);
	
	// Call the driver start method
	return driver->start(driver);
}

OS_Return _OS_DriverStop(OS_Driver *driver)
{
	// Validate the arguments
	ASSERT(driver);
	
	// Also ensure that the 'stop' method is provided
	ASSERT(driver->stop);
	
	// Call the driver start method
	return driver->stop(driver);
}

OS_Driver *_OS_DriverLookup(const INT8 name[])
{
    UINT32 i;
    UINT32 id;
    INT8 name_copy[4];
    OS_Driver *driver = NULL;
    
    // First copy the name into name_copy so that we are guaranteed to have 4 bytes
    strncpy(name_copy, name, 4);
    
    // Then extract the ID from it
    id = *(UINT32 *)name_copy;
    
    // Now look up for the driver by this id
    // Since we have a small number of Kernel drivers, it is OK to search linearly
    for(i = 0; i < g_kernel_driver_count; i++)
    {
        if(g_kernel_drivers[i].id == id)
        {
            driver = g_kernel_drivers[i].driver;
            break;
        }
    }
    
    return driver;
}
