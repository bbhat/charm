///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	vic.h
//	Author:	Bala B.
//	Description: MINI210S Vector Interrupt Controller
//
///////////////////////////////////////////////////////////////////////////////


#ifndef _VIC_H
#define _VIC_H

#include "os_core.h"

typedef enum {
	VIC0,
	VIC1,
	VIC2,
	VIC3
	} VIC_Index;
	
void _vic_reset_interrupts(void);
void _vic_set_interrupt_vector(OS_InterruptVector isr, UINT32 index);
void _vic_enable_interrupt_vector(UINT32 index);
void _vic_disable_interrupt_vector(UINT32 index);
void _vic_ack_irq(UINT32 index);

#define OS_SetInterruptVector(isr, index)	{ \
		_vic_set_interrupt_vector(isr, index); \
		_vic_enable_interrupt_vector(index); \
	}

#endif // _VIC_H