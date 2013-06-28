///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	int_vectors.c
//	Author:	Bala B.
//	Description: S5PV210 Interrupt handlers
///////////////////////////////////////////////////////////////////////////////

#include "os_core.h"
#include "vic.h"

///////////////////////////////////////////////////////////////////////////////
// Primary ARM Interrupt Handlers
///////////////////////////////////////////////////////////////////////////////
void _undefined_instr_handler(void)
{
	panic("Unandled FIQ");
}

void _software_interrupt_handler(void)
{
	panic("Unandled FIQ");
}

void _prefetch_abort_handler(void)
{
	panic("Unandled FIQ");
}

void _data_abort_handler(void)
{
	panic("Unandled FIQ");
}

void _reserved_interrupt(void)
{
	panic("Unandled FIQ");
}

void _IRQ_handler(void)
{
	static volatile unsigned long * vic_status_regs[4] = { VIC0IRQSTATUS, VIC1IRQSTATUS, VIC2IRQSTATUS, VIC3IRQSTATUS };
	static volatile unsigned long * vic_addr_regs[4] = { VIC0ADDR, VIC1ADDR, VIC2ADDR, VIC3ADDR };
	
	OS_InterruptVector isr;

	if(*vic_status_regs[VIC0])
	{
		isr = (OS_InterruptVector) (*vic_addr_regs[VIC0]);
	}
	else if(*vic_status_regs[VIC1])
	{
		isr = (OS_InterruptVector) (*vic_addr_regs[VIC1]);
	}
	else if(*vic_status_regs[VIC2])
	{
		isr = (OS_InterruptVector) (*vic_addr_regs[VIC2]);
	}
	else if(*vic_status_regs[VIC3])
	{
		isr = (OS_InterruptVector) (*vic_addr_regs[VIC3]);
	}
	
	if(isr) (*isr)();		// Invoke the 	 ISR
}

void _FIQ_handler(void)
{
	panic("Unandled FIQ");
}
