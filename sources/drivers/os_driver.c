///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	driver.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Header file for common driver infrastructure
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_core.h"
#include "os_sched.h"
#include "os_config.h"
#include "os_driver.h"
#include "os_config.h"
#include "os_memory.h"
#include "util.h"

typedef struct 
{
    UINT32        id;
    OS_Driver     *driver;
    
} KernelDriverEntry;

static KernelDriverEntry g_kernel_drivers[MAX_KERNEL_DRIVERS];
static UINT32 g_kernel_driver_count = 0;

static void _Driver_FreeIORequest(OS_Driver * driver, IO_Request * req);
static IO_Request * _Driver_GetFreeIORequest(OS_Driver * driver);
static IO_Request * _Driver_EnqueueWriteRequest(OS_Driver * driver, IO_Request * req);
static IO_Request * _Driver_EnqueueReadRequest(OS_Driver * driver, IO_Request * req);

extern OS_Process * g_current_process;
extern OS_Task * g_current_task;

extern _OS_Queue g_ready_q;
extern _OS_Queue g_wait_q;
extern _OS_Queue g_ap_ready_q;
extern _OS_Queue g_completed_task_q;

extern void _OS_SchedulerSuspendTask(OS_Task *);
extern void _OS_SchedulerResumeTask(OS_Task *);
extern void _OS_Schedule(void);

#ifdef _USE_STD_LIBS
	#define FAULT(x, ...) printf(x, ...);
#else
	#define FAULT(x, ...)
#endif

#define MAX(a, b) ((a) > (b) ? (a) : (b))

///////////////////////////////////////////////////////////////////////////////
// The following function is called to register a driver the the OS. 
// The name should be at least 4 characters and those 4 characters should be unique.
// This is because we use the first 4 characters as a number so that we can easily search
// for a diver using its name without using expensive string search operations.
// The 'max_io_count' corresponds to the maximum number of IOs that can be pending in this driver
///////////////////////////////////////////////////////////////////////////////
OS_Return _OS_DriverInit(OS_Driver *driver, const INT8 name[], OS_Return (*init)(OS_Driver *), UINT32 max_io_count)
{
	OS_Driver_t drv;
	UINT32 i;
	UINT32 intsts;

	// Validate the arguments
	ASSERT(driver && init);
	
	// Ensure that we are not out of Kernel Driver Quota
	ASSERT(g_kernel_driver_count < MAX_KERNEL_DRIVERS);
	
	// Now ensure that there are no duplicates by the ID (first 4 characters of the driver name)
	if(_OS_DriverLookup(name, &drv) == SUCCESS) {
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
	driver->user_access_mask = ACCESS_READ | ACCESS_WRITE;
	driver->admin_access_mask = ACCESS_READ | ACCESS_WRITE;
	driver->usage_mode = 0;
	driver->open_clients = 0;
	driver->write_io_queue_head = NULL;
	driver->write_io_queue_tail = NULL;
	driver->read_io_queue_head = NULL;
	driver->read_io_queue_tail = NULL;

	// IO Handler task
	driver->io_task = NULL;

	// The max_io_count should be at least 1
	max_io_count = MAX(1, max_io_count);
	
	// Allocate Free IO Requests queue
	for(i = 0; i < max_io_count; i++) {
		IO_Request * req = (IO_Request *) kmalloc(sizeof(IO_Request));
		_Driver_FreeIORequest(driver, req);
	}
	
	// Call the driver 'init' function. This function should initialize all
	// other function pointers in the driver obect
	OS_Return result = driver->init(driver);

	OS_ENTER_CRITICAL(intsts);	
	if(result == SUCCESS) 
	{
	    // Copy the driver id by converting the first 4 characters of the driver name
	    g_kernel_drivers[g_kernel_driver_count].id = *(UINT32 *)driver->name;
	    g_kernel_drivers[g_kernel_driver_count].driver = driver;
	    g_kernel_driver_count++;
	}
	OS_EXIT_CRITICAL(intsts);	
	
	return result;
}

//////////////////////////////////////////////////////////////////////////////////////////
// _OS_DriverStart
//////////////////////////////////////////////////////////////////////////////////////////
OS_Return _OS_DriverStart(OS_Driver *driver)
{
	OS_Return result = SUCCESS;
	
	// Validate the arguments
	ASSERT(driver);
	
	// Call the driver start method
	if(driver->start) {
		result =  driver->start(driver);
	}
	
	return result;
}

//////////////////////////////////////////////////////////////////////////////////////////
// _OS_DriverStop
//////////////////////////////////////////////////////////////////////////////////////////
OS_Return _OS_DriverStop(OS_Driver *driver)
{
	OS_Return result = SUCCESS;
	
	// Validate the arguments
	ASSERT(driver);
	
	// Call the driver start method
	if(driver->stop) {	
		result = driver->stop(driver);
	}
	
	return result;
}

//////////////////////////////////////////////////////////////////////////////////////////
// _OS_DriverLookup
//////////////////////////////////////////////////////////////////////////////////////////
OS_Return _OS_DriverLookup(const INT8 * name, OS_Driver_t * driver)
{
    UINT32 i;
    UINT32 id;
    INT8 name_copy[4];
    OS_Return result = RESOURCE_NOT_FOUND;
    
    if(!name || !driver)  {
        return INVALID_ARG;
    }
    
    *driver = INVALID;
    
    // First copy the name into name_copy so that we are guaranteed to have 4 bytes
    strncpy(name_copy, name, 4);
    
    // Then extract the ID from it
    id = *(UINT32 *)name_copy;
    
    // Now look up for the driver by this id
    // Since we have a small number of Kernel drivers, it is OK to search linearly
    for(i = 0; i < g_kernel_driver_count; i++) {
        if(g_kernel_drivers[i].id == id) {
            *driver = i;
            result = SUCCESS;
            break;
        }
    }
    
    return result;
}

//////////////////////////////////////////////////////////////////////////////////////////
// _OS_DriverOpen
//////////////////////////////////////////////////////////////////////////////////////////
OS_Return _OS_DriverOpen(OS_Driver_t driver, OS_DriverAccessMode mode)
{
    if(driver < 0 || driver >= g_kernel_driver_count) {
        return BAD_ARGUMENT;
    }

    OS_Driver * driver_inst = g_kernel_drivers[driver].driver;
    
    // First check if the process has permissions to open this driver
	if(g_current_process->attributes & ADMIN_PROCESS) {
	    if((driver_inst->admin_access_mask & mode) != mode) {
	        return ACCESS_DENIED;
	    }
	}
	else {
	    if((driver_inst->user_access_mask & mode) != mode) {
	        return ACCESS_DENIED;
	    }
	}
    
    // If someone has opened this driver in exclusive mode, then we cannot let
    // other clients to open this
    if(driver_inst->usage_mode & ACCESS_EXCLUSIVE) {
        return EXCLUSIVE_ACCESS;
    }
        
    // Ensure that number of clients opening this driver do not exceed 255
    ASSERT(driver_inst->open_clients < 0xFF);
	    
    // Call the open function on the driver
    OS_Return status = SUCCESS;
    
    // If the driver has provided an open function, call it
    if(driver_inst->open) {
    	status = driver_inst->open(driver_inst);
    }
    
    // Update the usage mask and owner_process
    if(status == SUCCESS) {
        driver_inst->usage_mode |= mode;
        
        // If we are opening in exclusive mode, then store the owner process
        // Else if we are opening the driver in read / write mode, then count them
        if(driver_inst->usage_mode & ACCESS_EXCLUSIVE) {
            driver_inst->owner_process = g_current_process;
            driver_inst->open_clients++;
        }
		else if(driver_inst->usage_mode & (ACCESS_READ | ACCESS_WRITE)) {
            driver_inst->open_clients++;
        }
    }
    
    return status;
}

//////////////////////////////////////////////////////////////////////////////////////////
// _OS_DriverClose
//////////////////////////////////////////////////////////////////////////////////////////
OS_Return _OS_DriverClose(OS_Driver_t driver)
{
    if(driver < 0 || driver >= g_kernel_driver_count) {
        return BAD_ARGUMENT;
    }

    OS_Driver * driver_inst = g_kernel_drivers[driver].driver;
    
    // If there are any outstanding requests, return error
    if(driver_inst->write_io_queue_head || driver_inst->read_io_queue_head) {
    	return RESOURCE_BUSY;
    }
    
    if(driver_inst->usage_mode & ACCESS_EXCLUSIVE) {
    	// Ensure that this is the process which has opened the driver
    	if(driver_inst->owner_process == g_current_process) {
    		driver_inst->owner_process = NULL;
    		driver_inst->open_clients = 0;
    		driver_inst->usage_mode = 0;
    	}
    	else {
    		return RESOURCE_NOT_OWNED;
    	}
    }
    else if(driver_inst->usage_mode & (ACCESS_READ | ACCESS_WRITE)) {
		if(driver_inst->open_clients > 0) {
			driver_inst->open_clients--;
			
			// If the last reader/writer is closed, then clear the usage_mode flag
			if(driver_inst->open_clients == 0) {
				driver_inst->usage_mode = 0;
			}
		}
		else {
			return RESOURCE_NOT_OWNED;
		}
	}
	
	// If the driver has provided a close function, call it		
	return (driver_inst->close) ? driver_inst->close(driver_inst) : SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////
// _OS_DriverRead
//////////////////////////////////////////////////////////////////////////////////////////
OS_Return _OS_DriverRead(OS_Driver_t driver, void * buffer, UINT32 * size, BOOL waitOK)
{	
	// Validate the inputs
    if(driver < 0 || driver >= g_kernel_driver_count) {
        return BAD_ARGUMENT;
    }
    
    if(!buffer || !size) {
    	return BAD_ARGUMENT;
    }

    OS_Driver * driver_inst = g_kernel_drivers[driver].driver;
    
    // Ensure that the driver is opened in READ mode. Otherwise it is an error.
    // Note the currently the driver framework does not guarantee that the current client
    // has opened the driver in read mode as we store the ownership info 
    // only in exclusive access mode
    // This is done in order to support multiple readers/writes (without exclusive access) 
    // and yet to save space & time in implementation
    if(!(driver_inst->usage_mode & ACCESS_READ)) {
    	return RESOURCE_NOT_OPEN;
    }
    
    // If the driver is opened in exclusive mode, then ensure that the current process owns it
    if((driver_inst->usage_mode & ACCESS_EXCLUSIVE) && (driver_inst->owner_process != g_current_process)) {
		return EXCLUSIVE_ACCESS;
    }
    
    // We just know that some client has opened this driver in read mode
    // but not sure if it is this client. So check for access rights of the process again
    // just to increase the protection.
	if(g_current_process->attributes & ADMIN_PROCESS) {
	    if(!(driver_inst->admin_access_mask & ACCESS_READ)) {
	        return ACCESS_DENIED;
	    }
	}
	else {
	    if(!(driver_inst->user_access_mask & ACCESS_READ)) {
	        return ACCESS_DENIED;
	    }
	}

	// Create an IO_Request instance for this request
	IO_Request * io_request = _Driver_GetFreeIORequest(driver_inst);
	if(!io_request) {
		return RESOURCE_EXHAUSTED;
	}
	
	io_request->buffer = buffer;
	io_request->size = *size;
	io_request->completed = 0;
	io_request->attributes = READ_IO;
	io_request->blocked_task = NULL;
	io_request->return_size = size;		// Pointer where the return size needs to be updated

	OS_Return status = DEFER_IO_REQUEST;
	
	if(!driver_inst->read_io_queue_head && driver_inst->read)
	{
		status = driver_inst->read(driver_inst, io_request);
	}

	// Update the size
	*size = io_request->completed;
	
	// Update the task remaining and accumulated budgets
	_OS_UpdateCurrentTaskBudget();
	
	if(status == DEFER_IO_REQUEST)
	{
		// Enqueue this IO in the pending queue
		_Driver_EnqueueReadRequest(driver_inst, io_request);
		
		// If we are OK to wait, block this task
		if(waitOK) 
		{	
			// Update the IO Request 'blocked_task' so that this task can be resumed later
			io_request->blocked_task = g_current_task;
			
			// Suspend scheduling for this task
			_OS_SchedulerBlockCurrentTask();
		}
	}
	else
	{
		// We are done with this IO Request. Free it.
		_Driver_FreeIORequest(driver_inst, io_request);
	}

	// The return path is through _OS_Schedule, so it is important to
	// update the result in the syscall_result	
	if(g_current_task->syscall_result) 
		g_current_task->syscall_result[0] = status;

	// Call OS Scheduler to schedule another task. We do not return from this call
	_OS_Schedule();
	
	return status;
}

//////////////////////////////////////////////////////////////////////////////////////////
// _OS_DriverWrite
//////////////////////////////////////////////////////////////////////////////////////////
OS_Return _OS_DriverWrite(OS_Driver_t driver, const void * buffer, UINT32 * size, BOOL waitOK)
{
	// Validate the inputs
    if(driver < 0 || driver >= g_kernel_driver_count) {
        return BAD_ARGUMENT;
    }
    
    if(!buffer || !size) {
    	return BAD_ARGUMENT;
    }

    OS_Driver * driver_inst = g_kernel_drivers[driver].driver;

    // Ensure that the driver is opened in WRITE mode. Otherwise it is an error.
    // Note the currently the driver framework does not guarantee that the current client
    // has opened the driver in write mode as we store the ownership info 
    // only in exclusive access mode
    // This is done in order to support multiple readers/writes (without exclusive access) 
    // and yet to save space & time in implementation
    if(!(driver_inst->usage_mode & ACCESS_WRITE)) {
    	return RESOURCE_NOT_OPEN;
    }
    
    // If the driver is opened in exclusive mode, then ensure that the current process owns it
    if((driver_inst->usage_mode & ACCESS_EXCLUSIVE) && (driver_inst->owner_process != g_current_process)) {
		return EXCLUSIVE_ACCESS;
    }
    
    // We just know that some client has opened this driver in write mode
    // but not sure if it is this client. So check for access rights of the process again
    // just to increase the protection.
	if(g_current_process->attributes & ADMIN_PROCESS) {
	    if(!(driver_inst->admin_access_mask & ACCESS_WRITE)) {
	        return ACCESS_DENIED;
	    }
	}
	else {
	    if(!(driver_inst->user_access_mask & ACCESS_WRITE)) {
	        return ACCESS_DENIED;
	    }
	}

	// Create an IO_Request instance for this request
	IO_Request * io_request = _Driver_GetFreeIORequest(driver_inst);
	if(!io_request) {
		return RESOURCE_EXHAUSTED;
	}
	
	io_request->buffer = (void *) buffer;
	io_request->size = * size;
	io_request->completed = 0;
	io_request->attributes = WRITE_IO;
	io_request->blocked_task = NULL;
	io_request->return_size = size;		// Pointer where the return size needs to be updated

	OS_Return status = DEFER_IO_REQUEST;
	
	if(!driver_inst->write_io_queue_head && driver_inst->write) 
	{
		status = driver_inst->write(driver_inst, io_request);
	}

	// Update the size
	*size = io_request->completed;
	
	// Update the task remaining and accumulated budgets
	_OS_UpdateCurrentTaskBudget();
	
	if(status == DEFER_IO_REQUEST)
	{
		// Enqueue this IO in the pending queue
		_Driver_EnqueueWriteRequest(driver_inst, io_request);
		
		// If we are OK to wait block this task
		if(waitOK) 
		{	
			// Update the IO Request 'blocked_task' so that this task can be resumed later
			io_request->blocked_task = g_current_task;

			// Suspend scheduling for this task
			_OS_SchedulerBlockCurrentTask();
		}
	}
	else 
	{
		// We are done with this IO Request. Free it.
		_Driver_FreeIORequest(driver_inst, io_request);
	}
	
	// The return path is through _OS_Schedule, so it is important to
	// update the result in the syscall_result	
	if(g_current_task->syscall_result) 
		g_current_task->syscall_result[0] = status;

	// Call OS Scheduler to schedule another task. We do not return from this call
	_OS_Schedule();
	
	return status;
}

//////////////////////////////////////////////////////////////////////////////////////////
// _OS_DriverConfigure
//////////////////////////////////////////////////////////////////////////////////////////
OS_Return _OS_DriverConfigure(OS_Driver_t driver, const void * buffer, UINT32 size)
{
	// Validate the inputs
    if(driver < 0 || driver >= g_kernel_driver_count) {
        return BAD_ARGUMENT;
    }
    
    if(!buffer || !size) {
    	return BAD_ARGUMENT;
    }

    OS_Driver * driver_inst = g_kernel_drivers[driver].driver;
    
    // Ensure that the driver is opened in WRITE mode for configure function
    if(!(driver_inst->usage_mode & ACCESS_WRITE)) {
    	return RESOURCE_NOT_OPEN;
    }
    
    // If the driver is opened in exclusive mode, then ensure that the current process owns it
    if((driver_inst->usage_mode & ACCESS_EXCLUSIVE) && (driver_inst->owner_process != g_current_process)) {
		return EXCLUSIVE_ACCESS;
    }
    
    // We just know that some client has opened this driver in write mode
    // but not sure if it is this client. So check for access rights of the process again
    // just to increase the protection.
	if(g_current_process->attributes & ADMIN_PROCESS) {
	    if(!(driver_inst->admin_access_mask & ACCESS_WRITE)) {
	        return ACCESS_DENIED;
	    }
	}
	else {
	    if(!(driver_inst->user_access_mask & ACCESS_WRITE)) {
	        return ACCESS_DENIED;
	    }
	}

    OS_Return status = NOT_SUPPORTED;
    
	if(driver_inst->configure) {
		status = driver_inst->configure(driver_inst, buffer, size);
	}
	
	return status;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Function called by IO Task of the driver to resume a read request when there is some
// data is available
//////////////////////////////////////////////////////////////////////////////////////////
void _Driver_ResumeReadRequest(OS_Driver * driver)
{
	ASSERT(driver);
	IO_Request * req;
	OS_Return result;

	req = driver->read_io_queue_head;
	if(req && driver->read) {
		result = driver->read(driver, req);		
		if(result != DEFER_IO_REQUEST) {
			_Driver_CompleteReadRequest(driver, result);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Function called by IO Task of the driver to resume a read request when there is some
// data is available
//////////////////////////////////////////////////////////////////////////////////////////
void _Driver_ResumeWriteRequest(OS_Driver * driver)
{
	ASSERT(driver);
	IO_Request * req;
	OS_Return result;
	
	req = driver->write_io_queue_head;
	if(req && driver->write) {
		result = driver->write(driver, req);	
		if(result != DEFER_IO_REQUEST) {
			_Driver_CompleteWriteRequest(driver, result);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Function called by the IO Task of the driver
// Whenever a driver finishes processing a deferred IO write request, it should call this function
// to inform the driver framework about it. This function dequeues the current request from the
// pending queue and if there is any task blocked on it, it will be resumed.
//////////////////////////////////////////////////////////////////////////////////////////
void _Driver_CompleteWriteRequest(OS_Driver * driver, OS_Return result)
{
	UINT32 intsts;
	IO_Request * req;
	ASSERT(driver);
	
	OS_ENTER_CRITICAL(intsts);
	if(driver->write_io_queue_head) {
		req = driver->write_io_queue_head;
		driver->write_io_queue_head = req->next;
		req->next = NULL;
		
		if(!driver->write_io_queue_head) {
			driver->write_io_queue_tail = NULL;
		}
		
		// Update the return size so that it will be visible to the callers
		*req->return_size = req->completed;
		
		// If there is a task blocked on this request, resume the same
		if(req->blocked_task) {
			if(req->blocked_task->syscall_result)
				req->blocked_task->syscall_result[0] = result;
			
			// Insert this back into the scheduler queue
			_OS_SchedulerUnblockTask(req->blocked_task);
		}		

		// We are done with this IO Request. Free it.
		_Driver_FreeIORequest(driver, req);
	}
	OS_EXIT_CRITICAL(intsts);	
}

//////////////////////////////////////////////////////////////////////////////////////////
// Function called by the IO Task of the driver
// Whenever a driver finishes processing a deferred IO read request, it should call this function
// to inform the driver framework about it. This function dequeues the current request from the
// pending queue and if there is any task blocked on it, it will be resumed.
//////////////////////////////////////////////////////////////////////////////////////////
void _Driver_CompleteReadRequest(OS_Driver * driver, OS_Return result)
{
	UINT32 intsts;
	IO_Request * req;
	ASSERT(driver);
	
	OS_ENTER_CRITICAL(intsts);
	if(driver->read_io_queue_head) {
		req = driver->read_io_queue_head;
		driver->read_io_queue_head = req->next;
		req->next = NULL;
		
		if(!driver->read_io_queue_head) {
			driver->read_io_queue_tail = NULL;
		}
		
		// Update the return size so that it will be visible to the callers
		*req->return_size = req->completed;
		
		// If there is a task blocked on this request, resume the same
		if(req->blocked_task) {		
			if(req->blocked_task->syscall_result) {
				req->blocked_task->syscall_result[0] = result;
			}
			
			// Insert this back into the scheduler queue
			_OS_SchedulerUnblockTask(req->blocked_task);
		}
		
		// We are done with this IO Request. Free it.
		_Driver_FreeIORequest(driver, req);
	}
	OS_EXIT_CRITICAL(intsts);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Function called by the IO Task of the driver
// This function returns the next pending IO request in the driver. It can be used to get 
// the current IO Request that is being processed by the driver. It moves to the next request
// when the driver calls _Driver_IORequestComplete
//////////////////////////////////////////////////////////////////////////////////////////
IO_Request * _Driver_GetNextReadRequest(OS_Driver * driver)
{
	ASSERT(driver);
	return driver->read_io_queue_head;
}

IO_Request * _Driver_GetNextWriteRequest(OS_Driver * driver)
{
	ASSERT(driver);
	return driver->write_io_queue_head;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Function for IO Request management
//////////////////////////////////////////////////////////////////////////////////////////
IO_Request * _Driver_EnqueueWriteRequest(OS_Driver * driver, IO_Request * req)
{
	UINT32 intsts;
	ASSERT(driver);
	
	OS_ENTER_CRITICAL(intsts);
	if(driver->write_io_queue_tail) {
		req->next = NULL;
		driver->write_io_queue_tail->next = req;
	}
	else {
		driver->write_io_queue_head = req;
	}
	driver->write_io_queue_tail = req;
	OS_EXIT_CRITICAL(intsts);	

	return req;
}

IO_Request * _Driver_EnqueueReadRequest(OS_Driver * driver, IO_Request * req)
{
	UINT32 intsts;
	ASSERT(driver);
	
	OS_ENTER_CRITICAL(intsts);
	if(driver->read_io_queue_tail) {
		req->next = NULL;
		driver->read_io_queue_tail->next = req;
	}
	else {
		driver->read_io_queue_head = req;
	}
	driver->read_io_queue_tail = req;
	OS_EXIT_CRITICAL(intsts);	

	return req;
}

void _Driver_FreeIORequest(OS_Driver * driver, IO_Request * req)
{
	UINT32 intsts;
	ASSERT(driver && req);
	
	req->next = NULL;
	
	OS_ENTER_CRITICAL(intsts);
	if(driver->free_io_queue) {
		req->next = driver->free_io_queue;
	}
	driver->free_io_queue = req;
	OS_EXIT_CRITICAL(intsts);
}

IO_Request * _Driver_GetFreeIORequest(OS_Driver * driver)
{
	UINT32 intsts;
	IO_Request * req = NULL;
	
	ASSERT(driver);
	
	OS_ENTER_CRITICAL(intsts);
	
	if(driver->free_io_queue) {
		req = driver->free_io_queue;
		driver->free_io_queue = req->next;
		req->next = NULL;
	}
	
	OS_EXIT_CRITICAL(intsts);
	return req;
}
