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

void _FIQHandler_(void)
{
	panic("Unandled FIQ");
}

///////////////////////////////////////////////////////////////////////////////
// Install primary ARM Exception Handler addresses
///////////////////////////////////////////////////////////////////////////////
void _OS_install_exception_handlers(void)
{
	volatile UINT32 * vector_table_ptr = &_interrupt_vector_table;
	
	vector_table_ptr[0] = (UINT32)_reset_handler;
	vector_table_ptr[1] = (UINT32)_undefined_instr_handler;
	vector_table_ptr[2] = (UINT32)_software_interrupt_handler;
	vector_table_ptr[3] = (UINT32)_prefetch_abort_handler;
	vector_table_ptr[4] = (UINT32)_data_abort_handler;
	vector_table_ptr[5] = (UINT32)_reserved_interrupt;
	vector_table_ptr[6] = (UINT32)_IRQHandler_;
	vector_table_ptr[7] = (UINT32)_FIQHandler_;
}
