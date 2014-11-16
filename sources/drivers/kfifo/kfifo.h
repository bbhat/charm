///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2014 xxxxxxx, xxxxxxx
//	File:	kfifo.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Kernel Driver that just acts as a queue b/w producers and consumers
//				 This is especially useful when exchanging data between two user space 
//				 processes
//	
///////////////////////////////////////////////////////////////////////////////

#ifndef _KFIFO_H_
#define _KFIFO_H_

#include "os_config.h"
#include "os_driver.h"

typedef struct Kfifo_driver {

	// Driver base struct as the first member
	OS_Driver	base;

	// Ring buffer used for queuing data
	UINT32 		*fifo_buffer;				// The FIFO depth in number of UINT32
	UINT32 		fifo_buffer_size;

	// Read and Write Index used by FIFO
	UINT32 		write_index;
	UINT32 		read_index;

} Kfifo_driver;

///////////////////////////////////////////////////////////////////////////////
// Design
//	This driver simply manages a fifo of UINT32 words that can be used to exchange
//	data between producers and consumers. Both producer and consumers are user space
// 	processes. For example, the Fimg2D graphics driver is implemented as a user
//	space demon. If other user processes want to display anything, they can send
//	a command to Fimg2D kfifo instance in the kernel. The Fimg2D user space demon
//	reads this kfifo and executes the command to display graphics
///////////////////////////////////////////////////////////////////////////////

// Global instances of the Kfifo driver
#if ENABLE_2D_GRAPHICS
extern Kfifo_driver g_kfifo_fimg2d;			// We need one for Fimg2D
#endif

///////////////////////////////////////////////////////////////////////////////
// Driver functions
///////////////////////////////////////////////////////////////////////////////
OS_Return _Kfifo_DriverInit(OS_Driver * driver, UINT32 fifo_depth);

#endif // SERIAL_DRIVER_ENABLE
#endif // _SERIAL_H_
