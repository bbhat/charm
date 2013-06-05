///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	ramdisk.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: ramdisk header file
//
//	Note: This ramdisk is a Read-Only file system. This is not optimized for writes.
//	
///////////////////////////////////////////////////////////////////////////////

#ifndef _RAMDISK_H
#define _RAMDISK_H

#include "os_types.h"
#include "os_config.h"

#define RAMDISK_IDENT_STRING	("chARMrd")
#define RAMDISK_IDENT_LENGTH	8

#define MAX_FILE_NAME_SIZE		40

// The ramdisk starts with the following structure
typedef struct
{
	INT8	ident[RAMDISK_IDENT_LENGTH];		// Unique identifier for the ramdisk file
	UINT32	size;								// Total size of the ramdisk file including this header
	UINT32	rootOffset;							// Offset of the first file header
	UINT32	crc;								// CRC of the ramdisk file. The CRC computation not include 
												// ramdisk header data. It includes everything after this header
	
} FS_RamdiskHdr;

// There are zero or more file headers in the ramdisk
typedef struct 
{
	INT8 			fileName[MAX_FILE_NAME_SIZE];
	UINT32 			flags;						// file / dir attributes and Permissions
	UINT32			offset;						// Offset where the file contents begin
	union
	{
		UINT32		length;						// Size of the file, if this is not a folder
		UINT32		fileCount;					// Number of files in the folder
	};
	
	UINT32			parent;						// Offset of the parent folder
	
} FS_FileHdr;


enum 
{
	F_DIR_MASK 	= 0x10000000,
	F_FILE 		= 0x00000000,
	F_DIR 		= 0x10000000,
};

#endif // _RAMDISK_H
