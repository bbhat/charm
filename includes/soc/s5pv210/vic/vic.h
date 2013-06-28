///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	vic.h
//	Author:	Bala B.
//	Description: MINI210S Vector Interrupt Controller
//
///////////////////////////////////////////////////////////////////////////////

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
void _vic_ack_interrupt(void);
