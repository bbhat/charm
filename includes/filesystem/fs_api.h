///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	fs_api.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Header file for the APIs
//	
///////////////////////////////////////////////////////////////////////////////

#ifndef _FS_API_H
#define _FS_API_H

///////////////////////////////////////////////////////////////////////////////
// File system related functions
///////////////////////////////////////////////////////////////////////////////

INT32 ramdisk_open(const INT8 * filepath, INT32 flags);
INT32 ramdisk_close(INT32 fd);
INT32 ramdisk_read(INT32 fd, void * ptr, INT32 numbytes);
INT32 ramdisk_lseek(INT32 fd, INT32 position, INT32 startpoint);
INT32 ramdisk_GetFileSize(INT32 fd);
void * ramdisk_GetDataPtr(INT32 fd, UINT32 * length);

enum 
{
	S_ISUID   	= 0x0000200,    // set user id on execution
	S_IRUSR   	= 0x0000100,    // protection: readable by owner
	S_IWUSR   	= 0x0000080,    // writable by owner
	S_IXUSR   	= 0x0000040,    // executable by owner
	S_IRGRP   	= 0x0000020,    // readable by group
	S_IWGRP   	= 0x0000010,    // writable by group
	S_IXGRP   	= 0x0000008,    // executable by group
	S_IROTH   	= 0x0000004,    // readable by all
	S_IWOTH   	= 0x0000002,    // writable by all
	S_IXOTH   	= 0x0000001     // executable by all
};
enum
{
	// File access modes for open()
	O_RDONLY          = 0,    // open(name, O_RDONLY) opens read only
	O_WRONLY          = 1,    // open(name, O_WRONLY) opens write only
	O_RDWR            = 2     // open(name, O_RDWR) opens read/write
};

enum 
{
	SEEK_SET = 0,
	SEEK_CUR = 1,
	SEEK_END = 2
};

typedef struct
{
	void * data;
	UINT32 cur_offset;
	UINT32 length;
	
} FILE;


#endif // _FS_API_H
