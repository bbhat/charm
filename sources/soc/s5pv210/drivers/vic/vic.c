///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	vic.c
//	Author:	Bala B.
//	Description: MINI210S Vector Interrupt Controller
//
///////////////////////////////////////////////////////////////////////////////

#include "vic.h"
#include "soc.h"

///////////////////////////////////////////////////////////////////////////////
// VIC Helper functions
///////////////////////////////////////////////////////////////////////////////
void _vic_reset_interrupts(void)
{
	*VIC0INTENCLEAR = 0xffffffff;
	*VIC1INTENCLEAR = 0xffffffff;
	*VIC2INTENCLEAR = 0xffffffff;
	*VIC3INTENCLEAR = 0xffffffff;

	*VIC0INTSELECT = 0x0;
	*VIC1INTSELECT = 0x0;
	*VIC2INTSELECT = 0x0;
	*VIC3INTSELECT = 0x0;

	*VIC0ADDR = 0;
	*VIC1ADDR = 0;
	*VIC2ADDR = 0;
	*VIC3ADDR = 0;
}

void _vic_set_interrupt_vector(OS_InterruptVector isr, UINT32 index)
{
	// VIC0
	if(index < 32)			
	{
		*VIC0VECTADDR(index) = (UINT32) isr;
	}
	// VIC1
	else if(index < 64) 		
	{
		*VIC1VECTADDR(index - 32) = (UINT32) isr;
	}
	// VIC2
	else if(index < 96) 
	{
		*VIC2VECTADDR(index - 64) = (UINT32) isr;
	}
	// VIC3
	else if(index < 128) 
	{
		*VIC3VECTADDR(index - 96) = (UINT32) isr;
	}
	else
	{
		ASSERT_ALWAYS("Invalid interrupt index");
	}
	
	return;
}

void _vic_enable_interrupt_vector(UINT32 index)
{
	if(index < 32)
	{
		*VIC0INTENABLE |= (1 << index);
	}
	else if(index < 64)
	{
		*VIC1INTENABLE |= (1 << (index - 32));
	}
	else if(index < 96)
	{
		*VIC2INTENABLE |= (1 << (index - 64));
	}
	else if(index < 128)
	{
		*VIC3INTENABLE |= (1 << (index - 96));
	}
	else 
	{
		ASSERT_ALWAYS("Invalid interrupt index");
	}
}

void _vic_disable_interrupt_vector(UINT32 index)
{
	if(index < 32)
	{
		*VIC0INTENCLEAR |= (1 << index);
	}
	else if(index < 64)
	{
		*VIC1INTENCLEAR |= (1 << (index - 32));
	}
	else if(index < 96)
	{
		*VIC2INTENCLEAR |= (1 << (index - 64));
	}
	else if(index < 128)
	{
		*VIC3INTENCLEAR |= (1 << (index - 96));
	}
	else 
	{
		ASSERT_ALWAYS("Invalid interrupt index");
	}
}

void _vic_ack_irq(UINT32 index)
{
	static volatile unsigned long * vic_addr_regs [] = { VIC0ADDR, VIC1ADDR, VIC2ADDR, VIC3ADDR };
	
	ASSERT(index < 128);
	
	*vic_addr_regs[index >> 5] = 0;
}
