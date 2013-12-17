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

extern OS_Process * g_current_process;

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
	OS_Driver_t drv;

	// Validate the arguments
	ASSERT(driver && init);
	
	// Ensure that we are not out of Kernel Driver Quota
	ASSERT(g_kernel_driver_count < MAX_KERNEL_DRIVERS);
	
	// Now ensure that there are no duplicates by the ID (first 4 characters of the driver name)
	if(_OS_DriverLookup(name, &drv) == SUCCESS)
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
	
	driver->owner_process = NULL;
	driver->user_access_mask = ACCESS_EXCLUSIVE;
	driver->admin_access_mask = ACCESS_EXCLUSIVE;
	driver->usage_mask = 0;
	
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

OS_Return _OS_DriverLookup(const INT8 * name, OS_Driver_t * driver)
{
    UINT32 i;
    UINT32 id;
    INT8 name_copy[4];
    OS_Return result = RESOURCE_NOT_FOUND;
    
    if(!name || !driver) 
    {
        return INVALID_ARG;
    }
    
    *driver = INVALID;
    
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
            *driver = i;
            result = SUCCESS;
            break;
        }
    }
    
    return result;
}

OS_Return _OS_DriverOpen(OS_Driver_t driver, OS_DriverAccessMode mode)
{
    if(driver < 0 || driver >= g_kernel_driver_count)
    {
        return ARGUMENT_ERROR;
    }

    OS_Driver * driver_inst = &g_kernel_drivers[driver];
    
    // First check if the process has permissions to open this driver
	if(g_current_process->attributes & ADMIN_PROCESS)
	{
	    if((driver_inst->admin_access_mask & mode) != mode)
	    {
	        return ACCESS_DENIED;
	    }
	}
	else
	{
	    if((driver_inst->user_access_mask & mode) != mode)
	    {
	        return ACCESS_DENIED;
	    }
	}
    
    // If someone has opened this driver in write mode, then we cannot let
    // other clients to open this
    if(driver_inst->usage_mask & ACCESS_WRITE)
    {
        return EXCLUSIVE_ACCESS;
    }
    
    // If someone has opened this in Read mode but we are now requesting write
    // mode, then also we should fail
    if((driver_inst->usage_mask & ACCESS_READ) && (mode & ACCESS_WRITE))
    {
        return EXCLUSIVE_ACCESS;
    }    
    
    // Call the open function on the driver
    OS_Return status = driver_inst->open(driver_inst);
    
    // Update the usage mask and owner_process
    if(status == SUCCESS)
    {
        driver_inst->usage_mask |= mode;
        if(driver_inst->usage_mask & ACCESS_WRITE)
        {
            driver_inst->owner_process = g_current_process;
        }
    }
    
    return status;
}

OS_Return _OS_DriverClose(OS_Driver_t driver)
{
}

OS_Return _OS_DriverRead(OS_Driver_t driver)
{
}

OS_Return _OS_DriverWrite(OS_Driver_t driver)
{
}

OS_Return _OS_DriverConfigure(OS_Driver_t driver)
{
}
