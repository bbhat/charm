/* 
 * This file adds a 16 Byte header to the input binary file so that it can be read by iROM.
 * 
 * Header Info
 * 0x0c: Reserved (should be 0)
 * 0x08: Image CheckSum
 * 0x04: Reserved (should be 0)
 * 0x00: Image size
 * 
 * For more information, please refer to S5PV210 RISC Microprocessor
 * IROM(Internal ROM) Booting Application Note
 *
 */
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFSIZE                 (16*1024)
#define IMG_SIZE                (16*1024)
#define SPL_HEADER_SIZE         16
#define SPL_HEADER              "S5PV210 HEADER  "

int main (int argc, char *argv[])
{
	FILE	*fp;
	char	*Buf, *a;
	int		BufLen;
	int		nbytes, fileLen;
	unsigned int	checksum, count;
	int		i;
	
	// Validate input arguments
	if (argc != 3)
	{
		printf("Usage: %s <source file> <destination file>\n", argv[0]);
		return -1;
	}

	// Allocate a 16K buffer
	BufLen = BUFSIZE;
	Buf = (char *)malloc(BufLen);
	if (!Buf)
	{
		printf("Alloc buffer failed!\n");
		return -1;
	}

	memset(Buf, 0x00, BufLen);

	// Reading source file to buffer
	fp = fopen(argv[1], "rb");
	if( fp == NULL)
	{
		printf("source file open error\n");
		free(Buf);
		return -1;
	}
	
	// Calculate the input filesize
	fseek(fp, 0L, SEEK_END);
	fileLen = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	
	// Validate that file size is <= 16K-16byte
	count = (fileLen < (IMG_SIZE - SPL_HEADER_SIZE))
		? fileLen : (IMG_SIZE - SPL_HEADER_SIZE);
		
	// buffer [0 ~ 15] store "S5PC110 HEADER"
	memcpy(&Buf[0], SPL_HEADER, SPL_HEADER_SIZE);
	
	// Read the source bin to buffer [16]
	nbytes = fread(Buf + SPL_HEADER_SIZE, 1, count, fp);
	if ( nbytes != count )
	{
		printf("source file read error\n");
		free(Buf);
		fclose(fp);
		return -1;
	}
	
	fclose(fp);

	// Calculate the checksum
	a = Buf + SPL_HEADER_SIZE;
	for(i = 0, checksum = 0; i < IMG_SIZE - SPL_HEADER_SIZE; i++)
		checksum += (0x000000FF) & *a++;
		
	// Checksum is stored in the buffer [8 ~ 15]
	a = Buf + 8;
	*((unsigned int *)a) = checksum;

	// Copies the contents of the buffer to the destination file
	fp = fopen(argv[2], "wb");
	if (fp == NULL)
	{
		printf("destination file open error\n");
		free(Buf);
		return -1;
	}
	
	a = Buf;
	nbytes	= fwrite( a, 1, BufLen, fp);
	if ( nbytes != BufLen )
	{
		printf("destination file write error\n");
		free(Buf);
		fclose(fp);
		return -1;
	}

	free(Buf);
	fclose(fp);

	return 0;
}
