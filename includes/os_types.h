///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
// File: os_types.h
// Author: Bala B. (bhat.balasubramanya@gmail.com)
// Description: Header file for the data types used in the OS
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _OS_TYPES_H
#define _OS_TYPES_H

//macro definitions
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif // NULL

#define INVALID	(-1)

///////////////////////////////////////////////////////////////////////////////
// Redefine datatypes so that the OS can be ported easily on
// different platforms
///////////////////////////////////////////////////////////////////////////////

typedef unsigned char BOOL;
typedef unsigned char UINT8; // Unsigned 8 bit data
typedef char INT8; // Singned 8 bit data
typedef unsigned short UINT16; // Unsigned 16 bit data
typedef short INT16; // Signed 16 bit data
typedef unsigned int UINT32; // Unsigned 32 bit data
typedef int INT32; // Signed 32 bit data
typedef unsigned long long UINT64; // Unsigned 64 bit data
typedef long long INT64; // Signed 64 bit data
typedef float FP32; // 32 bit Floating point data
typedef double FP64; // 64 bit Floating point data

typedef INT32 _OS_KernelObj_Handle;

typedef _OS_KernelObj_Handle 	OS_Task;
typedef _OS_KernelObj_Handle 	OS_Process;
//typedef _OS_KernelObj_Handle	OS_Sem;
//typedef _OS_KernelObj_Handle	OS_Mutex;

#endif // _OS_TYPES_H
