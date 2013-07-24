///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	exceptions.c
//	Author:	Bala B.
//	Description: S5PV210 Exception handlers
///////////////////////////////////////////////////////////////////////////////

#include "os_core.h"
#include "vic.h"
#include "soc.h"

extern volatile UINT32 _interrupt_vector_table;

void _IRQHandler_(void);
void _reset_handler(void);

///////////////////////////////////////////////////////////////////////////////
// Primary ARM Exception Handlers
///////////////////////////////////////////////////////////////////////////////
void _undefined_instr_handler(void)
{
	panic("Unandled undefined instruction exception");
}

void _software_interrupt_handler(void)
{
	panic("Unandled software interrupt");
}

void _prefetch_abort_handler(void)
{
	panic("Unandled prefetch abort");
}

void _data_abort_handler(void)
{
	panic("Unandled data abort");
}

void _reserved_interrupt(void)
{
	panic("Unandled reserved interrupt");
}

void _FIQHandler_(void)
{
	panic("Unandled FIQ");
}


///////////////////////////////////////////////////////////////////////////////
// Function to set interrupt vector table
///////////////////////////////////////////////////////////////////////////////
static UINT32 _set_interrupt_vector(volatile UINT32 * vector_table_ptr)
{
	asm("mcr	p15, 0, r0, c12, c0, 0");
	asm("mrc	p15, 0, r0, c12, c0, 0");
}

///////////////////////////////////////////////////////////////////////////////
// Install primary ARM Exception Handler addresses
///////////////////////////////////////////////////////////////////////////////
void _OS_install_exception_handlers(void)
{
	volatile UINT32 * vector_table_ptr = &_interrupt_vector_table;

	// Update the interrupt vector address
	_set_interrupt_vector(vector_table_ptr);
}
