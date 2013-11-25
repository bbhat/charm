///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	platform.c
//	Author:	Bala B.
//	Description: S5PV210 platform specific functions
///////////////////////////////////////////////////////////////////////////////

#include "vic.h"

void _OS_install_exception_handlers(void);

void _OS_PlatformInit(void)
{	
	// Install exception handlers
	_OS_install_exception_handlers();
	
	// Initialize Vectored Interrupt Controller
	_vic_initialize();	
}
