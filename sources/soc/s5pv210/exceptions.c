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
