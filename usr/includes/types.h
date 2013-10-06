
#ifndef _TPYES_H
#define _TPYES_H

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

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif  // _SIZE_T

#define INVALID	(-1)

///////////////////////////////////////////////////////////////////////////////
// Redefine datatypes so that the OS can be ported easily on
// different platforms
///////////////////////////////////////////////////////////////////////////////

typedef unsigned int BOOL;
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

#endif /* _TPYES_H */
