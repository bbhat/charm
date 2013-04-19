///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	elfmerge.c
//	Author:	Bala bhat (bhat.balasubramanya@gmail.com)
//
//	Description: This tool creates a ramdisk image and adds a new file to the ramdisk
//	If the ramdisk file already exists, then it just appends a new file to te ramdisk.
//	Otherwise it creates a ramdisk file containing the file to be added.
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>

#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stddef.h>

#include "ramdisk.h"

Node_Ramdisk ramdisk;

typedef enum {
	CMD_INVALID,
	CMD_ADD_FILE,
	CMD_DELETE_FILE,
	CMD_ADD_FOLDER,
	CMD_PRINT,
	CMD_SAVE,
	CMD_SAVE_AND_QUIT,
	CMD_QUIT
	
} User_command;

char fileAccessFlags [][4] = { 	
						"---",
						"--x",
						"-w-",
						"-wx",
						"r--",
						"r-x",
						"rw-",
						"rwx" };


User_command getUserOption(void);
int makeRamdiskNode(int rdfile, Node_Ramdisk *rd);
int ramdiskWriteFile(int rdfile, Node_Ramdisk *rd);
int readRamdiskHdr(int rdfile, FS_RamdiskHdr *rd);
int readFileData(int rdfile, off_t off, size_t size, char *buf);
int readFileHeader(int rdfile, off_t off, FS_FileHdr *fileHdr);
int fixFileOffsets(Node_File *file, int * offset);
void handleUserCommand(User_command cmd);

static int dirty = FALSE;
char * rdFileName = NULL;

//////////////////////////////////////////////////////////////////////////////////////////
// Functions to read the Ramdisk file
//////////////////////////////////////////////////////////////////////////////////////////
int readRamdiskHdr( int rdfile, FS_RamdiskHdr *rd )
{
	// Rewind the file first
	if( lseek(rdfile,0,SEEK_SET) == (off_t)-1 ) {
		fprintf(stderr,"lseek(elf,0,SEEK_SET): %s\n",
						strerror(errno));
		return -1;
	}
	
	// Now read in our structure
	if( read(rdfile, rd, sizeof(FS_RamdiskHdr)) == -1 ) {
		fprintf(stderr,"read(rd, rd, sizeof(FS_RamdiskHdr)): %s\n",
						strerror(errno));
		return -2;
	}
	
	return 0;
}

int readFileHeader( int rdfile, off_t off, FS_FileHdr *fileHdr )
{
	// Rewind the file first
	if( lseek(rdfile, off, SEEK_SET) == (off_t)-1 ) {
		fprintf(stderr,"lseek(elf,%lld,SEEK_SET): %s\n",
						off,
						strerror(errno));
		return -1;
	}
	
	// now read in our structure
	if( read(rdfile, fileHdr, sizeof(*fileHdr)) == -1 ) {
		fprintf(stderr,"read(rdfile, fileHdr, sizeof(*fileHdr)): %s\n",
						strerror(errno));
		return -2;
	}
	
	return 0;
}

// read in a block of data from file to specified buffer
int readFileData( int rdfile, off_t off, size_t size, char *buf )
{
	// Rewind the file first
	if( lseek(rdfile,off,SEEK_SET) == (off_t)-1 ) {
		fprintf(stderr,"lseek(elf,%lld,SEEK_SET): %s\n",
						off,
						strerror(errno));
		return -1;
	}
	
	// now read in our structure
	if( read(rdfile, buf, size) == -1 ) {
		fprintf(stderr,"read(rdfile, buf, %zu): %s\n",
						size,
						strerror(errno));
		return -2;
	}	
	
	return 0;
}

Node_File * readFileNode(int rdfile, int offset, Node_File *parent)
{
	Node_File * file = (Node_File *)malloc(sizeof(Node_File));
	if(!file)
	{
		fprintf(stderr,"malloc(%ld): %s\n", sizeof(Node_File), strerror(errno));
	}
		
	// Clear the Elf32_Segment
	memset(file, 0, sizeof(Node_File));
		
	// Attach the file to the parent
	if(parent)
	{
		if(parent->child)
		{
			file->next = parent->child;
		}
		
		parent->child = file;
		file->parent = parent;					
	}
	
	if(readFileHeader(rdfile, offset, &file->fileHdr) < 0 ) {
		return NULL;
	}
	
	if((file->fileHdr.flags & F_DIR_MASK) == F_DIR) 
	{
		for(int i = 0; i < file->fileHdr.fileCount; i++)
		{
			Node_File * child = readFileNode(rdfile, file->fileHdr.offset + i * sizeof(Node_File), 	file);
			
			if(!child) 
			{
				fprintf(stderr,"ERROR: Error while reading folder - %s\n", file->fileHdr.fileName);
				break;
			}
			
			if(file->child) 
			{
				child->next = file->child;
			}
			
			file->child = child;
		}
	}
	else if(file->fileHdr.length > 0)
	{

		file->data = (INT8 *) malloc(file->fileHdr.length);
		if(readFileData(rdfile, file->fileHdr.offset, file->fileHdr.length, file->data) < 0)
		{
			fprintf(stderr,"ERROR: Reading data for file - %s\n", file->fileHdr.fileName);
		}
	}
	
	return file;
}

int makeRamdiskNode(int rdfile, Node_Ramdisk *rd)
{
	if(rdfile < 0) return -1;
	if(rd == NULL) return -1;
	
	// Read ramdisk file header
	if(readRamdiskHdr(rdfile, &rd->rdHdr) < 0) {
		return -1;
	}

	// Validate the ramdisk file
	if(strcmp(rd->rdHdr.ident, RAMDISK_IDENT_STRING) != 0) {
		fprintf(stderr,"The input file is not a valid ramdisk image\n");
		return -1;
	}
	
	// Validate crc
	// TODO: Right now I am just using file size as the CRC
	if(rd->rdHdr.crc != rd->rdHdr.size) 
	{
		fprintf(stderr,"The CRC of the ramdisk image is not valid. \
						Expected 0x%x Actual 0x%x\n", rd->rdHdr.size, rd->rdHdr.crc);
		return -1;
	}
	
	// Read the root folder
	rd->root = readFileNode(rdfile, rd->rdHdr.rootOffset, NULL);
	
	if(!rd->root)
	{
		fprintf(stderr,"ERROR: Root file system is empty\n");
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Functions to update Ramdisk file
//////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
// This function fixes the offsets of the current file based on all the files in the 
// current and sub folders. 
// The input argument offset is the offset of the first byte of this file
// The argument offset will be updated to return the current offset after writing this 
// file and all sub files (if this is a folder).
//////////////////////////////////////////////////////////////////////////////////////////
int fixFileOffsets(Node_File *file, int * offset)
{
	if(!file) {
		fprintf(stderr,"ERROR: fixFileOffsets invalid arguments\n");
		goto Error;
	}
	
	if(!offset) {
		fprintf(stderr,"ERROR: fixFileOffsets invalid arguments - %s\n", file->fileHdr.fileName);
		goto Error;
	}
	
	// Update the offset in the current file
	file->fileHdr.offset = *offset;
	
	// Check if this is a folder
	if((file->fileHdr.flags & F_DIR_MASK) == F_DIR) 
	{
		// Create space for all the file headers
		*offset += (file->fileHdr.fileCount * sizeof(FS_FileHdr));
	
		// If this is a folder, update offsets for all files in this filder
		Node_File *child = file->child;
		while(child)
		{
			if(fixFileOffsets(child, offset) < 0) {
				goto Error;
			}
			
			child = child->next;
		}
	}
	else
	{
		// This is not a folder. Create space for the file data
		*offset += file->fileHdr.length;
	}
	
	return 0;
	
Error:
	return -1;
	
}

//////////////////////////////////////////////////////////////////////////////////////////
// Functions to write back the Ramdisk file
//////////////////////////////////////////////////////////////////////////////////////////
int saveRamdisk(Node_Ramdisk *rd, const char * rdFileName)
{
	int status;
	int rdfile;
	
	// Open ramdisk file for output
	if((rdfile = open(rdFileName,O_WRONLY)) < 0) {
		fprintf(stderr,"open(\"%s\",O_WRONLY): %s\n",
						rdFileName,
						strerror(errno));
		status = rdfile;
		goto Exit;
	}
	
	status = ramdiskWriteFile(rdfile, &ramdisk);
	
Exit:
	if(rdfile >= 0) {
		close(rdfile);
		rdfile = 0;
	}
	
	return status;
}

int writeFileNode(int rdfile, Node_File *file)
{
	if(!file)
	{
		fprintf(stderr,"writeFileNode: Argument error\n");
		return -1;
	}
		
	if((file->fileHdr.flags & F_DIR_MASK) == F_DIR) 
	{
		// This is a folder
		Node_File * child = file->child;
		
		// Write file headers for all files in this folder
		while(child)
		{ 
			if(write(rdfile, &child->fileHdr, sizeof(FS_FileHdr)) < 0)
			{
				fprintf(stderr,"EROOR: file write error - %s\n", strerror(errno));
				return -1;
			}
			
			child = child->next;
		}
		
		// Make another loop and call writeFileNode on each file
		child = file->child;
		
		while(child)
		{ 
			writeFileNode(rdfile, child);
			child = child->next;
		}
	}
	else if(file->fileHdr.length > 0)
	{
		if(write(rdfile, &file->data, file->fileHdr.length) < 0)
		{
			fprintf(stderr,"EROOR: file write error - %s\n", strerror(errno));
			return -1;
		}	
	}
	
	return 0;
}

int ramdiskWriteFile(int rdfile, Node_Ramdisk *rd)
{
	int offset = sizeof(FS_RamdiskHdr);
	
	if(!rd) 
	{
		fprintf(stderr,"ERROR: ramdiskWriteFile invalid arguments\n");
		return -1;
	}
	
	// First fix the offsets from the root folder
	if(fixFileOffsets(rd->root, &offset) < 0)
	{
		return -1;
	}
	
	// Update the size in the ramdisk header
	rd->rdHdr.rootOffset = sizeof(FS_RamdiskHdr);
	rd->rdHdr.size = offset;
	
	// TODO: Calculate the actual CRC for this field. Right now I am just using the length 
	// as CRC
	rd->rdHdr.crc = offset;
	
	// Now it is time to write Ramdisk file
	// Start with the ramdisk header
	if(write(rdfile, &rd->rdHdr, sizeof(FS_RamdiskHdr)) < 0)
	{
		fprintf(stderr,"EROOR: file write error - %s\n", strerror(errno));
		return -1;
	}
	
	// Write root folder
	return (writeFileNode(rdfile, rd->root));
}

//////////////////////////////////////////////////////////////////////////////////////////
// Functions to free all file nodes in the ramdisk
//////////////////////////////////////////////////////////////////////////////////////////
int freeFileNode(Node_File *file)
{
	if(!file)
	{
		fprintf(stderr,"freeFileNode: Argument error\n");
		return -1;
	}
		
	if((file->fileHdr.flags & F_DIR_MASK) == F_DIR) 
	{
		// This is a folder
		Node_File * child = file->child;
		
		// Write file headers for all files in this folder
		while(child)
		{ 
			Node_File * next_child = child->next;
			freeFileNode(child);
			child = next_child;
		}		
	}
	else if(file->data)
	{
		free(file->data);
		file->data = NULL;
	}
		
	// Now free the file node itself
	free(file);
	
	return 0;
}

int freeRamdisk()
{
	if(ramdisk.root)
	{
		freeFileNode(ramdisk.root);
		ramdisk.root = NULL;
	}
	
	memset(&ramdisk, 0, sizeof(Node_Ramdisk));
	
	return 0;	
}

//////////////////////////////////////////////////////////////////////////////////////////
// Functions parse the file and folder names
//////////////////////////////////////////////////////////////////////////////////////////

// Function to extract the base name given the path
// Returns the number of characters consumed
int getToken(char * dst, const char * path)
{
	int i = 0;
	int j = 0;
	if(!dst || ! path)
	{
		fprintf(stderr,"ERROR: getBaseFolderName: Argument error\n");
		return -1;	
	}
	
	// Ignore the first '/' if one exists
	if(path[0] == '/') i++;
	
	// Allow only alpha numeric characters, '.', '_' in the file/folder names
	while(path[i] != '/' && path[i] != '\x0')
	{
		if(((path[i] >= '0') && (path[i] <= '9')) ||
			((path[i] >= 'A') && (path[i] <= 'Z')) ||
			((path[i] >= 'a') && (path[i] <= 'z')) ||
			((path[i] == '.') || (path[i] == '_')))
		{
			dst[j++] = path[i++];
		}
		else
		{
			fprintf(stderr,"ERROR: getBaseFolderName: Invalid character in the file name: %c\n", path[i]);
			break;
		}
	}
	
	dst[j++] = '\x0';
	
	return i;	// Return the number of characters consumed;	
}

// Function to extract address where the file name begins, given its path
const char * extractFileName(const char * path)
{
	int i = 0;
	const char * filename;
	
	if(!path)
	{
		fprintf(stderr,"ERROR: extractFileName: Argument error\n");
		return NULL;	
	}
	
	filename = &path[i];
	while(path[i] != '\x0')
	{
		if(path[i] == '/')
		{
			filename = &path[i+1];
		}
		
		i++;
	}
		
	return filename;
}

// Given the path, get the file node representing the given path
// The path could be either a file or a folder
Node_File * getFile(Node_Ramdisk *rd, const char * path)
{
	Node_File *cur_dir;
	char name[MAX_FILE_NAME_SIZE];
	int i = 0;
	int found = FALSE;
	
	if(!rd || !path)
	{
		fprintf(stderr,"getFolder: Argument error\n");
		return NULL;	
	}
	if(!rd->root)
	{
		fprintf(stderr,"getFolder: Root file system is not mounted\n");
		return NULL;	
	}
	
	// Start searching from the root folder
	cur_dir = rd->root;

	// Start parsing the path
	while(cur_dir)
	{
		i += getToken(name, &path[i]);
		if(i < 0) {
			// There was an error
			break;
		}
		
		if(!strcmp(name, "")) {
			found = TRUE;
			break;
		}
		
		// Look for name in the current folder
		Node_File * file = cur_dir->child;
		while(file) 
		{
			if(!strcmp(file->fileHdr.fileName, name)) {
				break;
			}
			file = file->next;
		}
		
		// Did we find the folder with the given name?
		if(!file) {
			// We could not find the folder named 'name' in the current folder
			fprintf(stderr,"getFolder: Could not find folder '%s' inside '%s'\n",
							name, cur_dir->fileHdr.fileName);
							break;
		}
		
		cur_dir = file;
	}
	
	return (found ? cur_dir : NULL);
}

// Searches for a file/folder within the current directoty
Node_File * searchFile(Node_File * pwd, const char * filename)
{
	if(!pwd || !filename)
	{
		fprintf(stderr,"searchFile: Argument error\n");
		return NULL;	
	}
	
	// Check if the pwd is a folder or a file. We expect a folder.
	if((pwd->fileHdr.flags & F_DIR_MASK) != F_DIR)
	{
		// We were looking for a folder
		fprintf(stderr,"searchFile: '%s' is not a folder\n", pwd->fileHdr.fileName);
		return NULL;
	}

	// Now we need to validate that a node with this name does not exist already
	Node_File *child = pwd->child;
	while(child) {
		if(!strcmp(child->fileHdr.fileName, filename)) {
			break;
		}
		child = child->next;
	}
	
	return child;		
}

Node_File * ramdiskAddFolderToParent(Node_Ramdisk *rd, Node_File * parent, const char * foldername)
{
	if(!rd || !parent || !foldername)
	{
		fprintf(stderr,"ramdiskAddFolderToParent: Argument error\n");
		return NULL;	
	}
	if(!rd->root)
	{
		fprintf(stderr,"ramdiskAddFolderToParent: Root file system is not mounted\n");
		return NULL;	
	}
	
	// Now we need to validate that a node with this name does not exist already
	Node_File *child = parent->child;
	while(child) {
		if(!strcmp(child->fileHdr.fileName, foldername)) {
			fprintf(stderr,"ERROR: File/Folder '%s' already exists\n", foldername);
			return NULL;
		}
		child = child->next;
	}
		
	// Create a new file node for the new folder
	Node_File * newFile = (Node_File *) malloc(sizeof(Node_File));
	
	memset(newFile, 0, sizeof(Node_File));
	
	strncpy(newFile->fileHdr.fileName, foldername, MAX_FILE_NAME_SIZE);
	newFile->fileHdr.flags = F_DIR | (S_IRUSR | S_IWUSR | S_IXUSR) | S_IRGRP | S_IROTH;
	newFile->fileHdr.fileCount = 0;
	
	// Add the new folder to the ramdisk
	newFile->parent = parent;
	if(parent->child) {
		newFile->next = parent->child;
		parent->next = newFile;
	}
	else {
		parent->child = newFile;
	}
	parent->fileHdr.fileCount++;
	
	return newFile;
}

int ramdiskAddFolder(Node_Ramdisk *rd, const char * folderpath)
{
	char name[MAX_FILE_NAME_SIZE];
	
	if(!rd || !folderpath)
	{
		fprintf(stderr,"ramdiskAddFolder: Argument error\n");
		return -1;	
	}
	if(!rd->root)
	{
		fprintf(stderr,"ramdiskAddFolder: Root file system is not mounted\n");
		return -1;	
	}
	
	// Now scan through each folder in the folderpath and create folders if it does not already exist
	// Start searching from the root folder
	int i = 0;
	Node_File * cur_dir = rd->root;

	// Start parsing the path
	while(cur_dir)
	{
		i += getToken(name, &folderpath[i]);
		if(i < 0) {
			return -1;	// Error Occurred
		}
		else if(i == 0) {
			fprintf(stderr,"ERROR: Parsing '%s'\n",	folderpath);
			return -1;
		}

		// Check if this is the last token or not
		if(folderpath[i] == '\x0') {
			break;
		}
		
		// Get the folder corresponding to this token
		Node_File * next_dir = searchFile(cur_dir, name);
		
		// If the next_dir is NULL, then the folder does not exist. 
		// We can create a folder with the token name
		if(!next_dir) {
			next_dir = ramdiskAddFolderToParent(rd, cur_dir, name);
		}
		
		cur_dir = next_dir;
	}
	
	if(!cur_dir) {
		fprintf(stderr,"ERROR: Could not create all folders in the path '%s'\n", folderpath);
		return -1;
	}
	
	// Check if the destination path is a folder or a file. We expect a folder.
	if((cur_dir->fileHdr.flags & F_DIR_MASK) != F_DIR)
	{
		// We were looking for a folder
		fprintf(stderr,"ramdiskAddFolder: The path '%s' is not a folder\n", folderpath);
		return -1;
	}
	
	if(!ramdiskAddFolderToParent(rd, cur_dir, folderpath))
	{
		return -1;
	}
	
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Functions to add files / folders to ramdisk
// dst: is the destination path
//////////////////////////////////////////////////////////////////////////////////////////
int ramdiskAddFile(Node_Ramdisk *rd, const char * filepath)
{
	int addfile = -1;
	struct stat stat_buf;
	int status = -1;
	char name[MAX_FILE_NAME_SIZE];

	if(!rd || !filepath)
	{
		fprintf(stderr,"ramdiskAddFile: Argument error\n");
		return -1;	
	}
	if(!rd->root)
	{
		fprintf(stderr,"ramdiskAddFile: Root file system is not mounted\n");
		return -1;	
	}
	
	// Add the new file to ramdisk
	if((addfile = open(filepath,O_RDONLY)) < 0) {
		fprintf(stderr,"open(\"%s\",O_RDONLY): %s\n",
						filepath,
						strerror(errno));
		status = addfile;
		goto Exit;
	}

	if((status = stat(filepath, &stat_buf)) < 0)
	{
		fprintf(stderr,"stat(\"%s\"): %s\n",
						filepath,
						strerror(errno));
		goto Exit;		
	}

	// Now scan through each folder in the filepath and create folders if it does not already exist
	// Start searching from the root folder
	int i = 0;
	Node_File * cur_dir = rd->root;

	// Start parsing the path
	while(cur_dir)
	{
		i += getToken(name, &filepath[i]);
		if(i < 0) {
			// Error Occurred
			status = -1;
			goto Exit;
		}
		else if(i == 0) {
			fprintf(stderr,"ERROR: Parsing '%s'\n",	filepath);
			status = -1;
			goto Exit;
		}

		// Check if this is the last token (file name) or not
		if(filepath[i] == '\x0') {
			break;
		}
		
		// Get the folder corresponding to this token
		Node_File * next_dir = searchFile(cur_dir, name);
		
		// If the next_dir is NULL, then the folder does not exist. We can create a folder with 
		// the token name
		if(!next_dir) {
			next_dir = ramdiskAddFolderToParent(rd, cur_dir, name);
		}
		
		cur_dir = next_dir;
	}
	
	if(!cur_dir) {
		fprintf(stderr,"ERROR: Could not create all folders in the path '%s'\n", filepath);
		status = -1;
		goto Exit;	
	}
	
	// Check if the destination path is a folder or a file. We expect a folder.
	if((cur_dir->fileHdr.flags & F_DIR_MASK) != F_DIR)
	{
		// We were looking for a folder
		fprintf(stderr,"ramdiskAddFile: The path '%s' is not a folder\n", cur_dir->fileHdr.fileName);
		status = -1;
		goto Exit;
	}
	
	// Now we need to validate that a node with this name does not exist already
	if(!searchFile(cur_dir, name))
	{
		fprintf(stderr,"ERROR: File/Folder '%s' already exists in '%s'\n", name, cur_dir->fileHdr.fileName);
		status = -1;
		goto Exit;
	}
	
	// Create a new file node for the new file
	Node_File * newFile = (Node_File *) malloc(sizeof(Node_File));
	
	memset(newFile, 0, sizeof(Node_File));
	
	strncpy(newFile->fileHdr.fileName, name, MAX_FILE_NAME_SIZE);
	newFile->fileHdr.flags = F_FILE | (stat_buf.st_mode & 0x777);
	newFile->fileHdr.length = stat_buf.st_size;
	
	// Add the file to the folder
	newFile->parent = cur_dir;
	if(cur_dir->child) {
		newFile->next = cur_dir->child;
		cur_dir->next = newFile;
	}
	else {
		cur_dir->child = newFile;
	}
	cur_dir->fileHdr.fileCount++;
	status = 0;
	
Exit:

	if(addfile >= 0) {
		close(addfile);
		addfile = 0;
	}		
	
	return status;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Functions to print ramdisk
//////////////////////////////////////////////////////////////////////////////////////////
void printFileName(Node_File * file, int depth)
{
	if(!file) return;
	
	// First create proper indentation for the file / folder name
	printf("\n");
	for(int i = 0; i < depth; i++)
	{
		printf("   ");
	}

	// First print file permissions
	printf("%s%s%s ", fileAccessFlags[file->fileHdr.flags & 7],
					  fileAccessFlags[(file->fileHdr.flags >> 4) & 7],
					  fileAccessFlags[(file->fileHdr.flags >> 8) & 7]);

	if((file->fileHdr.flags & F_DIR_MASK) == F_DIR)
	{
		// Print folder name and the number of files in the folder
		printf("%s [%d]", file->fileHdr.fileName, file->fileHdr.fileCount);

		// Now we need to print each child node
		Node_File *child = file->child;
		while(child) {
			printFileName(child, depth+1);
			child = child->next;
		}
	}
	else		
	{
		// Print file name
		printf("%s", file->fileHdr.fileName);
	}
	
}


//////////////////////////////////////////////////////////////////////////////////////////
// Main function
//////////////////////////////////////////////////////////////////////////////////////////
int main( int argc, char *argv[] ) 
{
	int rdfile = -1;
	int status = 0;
	
	if(argc < 2) {
		fprintf(stderr,"\nSYNTAX:\n%s <ramdisk file> [new file to be added]\n",argv[0]);
		fprintf(stderr,"\n \
			This tool creates a ramdisk image and adds a new file to the ramdisk \
			If the ramdisk file already exists, then it just appends a new file to te ramdisk. \
			Otherwise it creates a ramdisk file containing the file to be added. \n\n");
		return -1;
	}
	
	// Open input files
	rdFileName = argv[1];
	if((rdfile = open(rdFileName,O_RDONLY)) < 0) {
		fprintf(stderr,"open(\"%s\",O_RDONLY): %s\n",
						rdFileName,
						strerror(errno));
		status = rdfile;
		goto Exit;
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// Process the first file
	//////////////////////////////////////////////////////////////////////////////////////
	
	if(rdfile < 0) {	
		fprintf(stderr,"Ramdisk file \"%s\" does not exist. Creating a new ramdisk file.\n", argv[1]);
		
		memset(&ramdisk, 0, sizeof(Node_Ramdisk));
		strncpy(ramdisk.rdHdr.ident, RAMDISK_IDENT_STRING, RAMDISK_IDENT_LENGTH);
		ramdisk.rdHdr.rootOffset = sizeof(FS_RamdiskHdr);
		ramdisk.rdHdr.size = sizeof(FS_RamdiskHdr);
		
		// Add a root folder
		ramdisk.root = (Node_File *) malloc(sizeof(Node_File));
		memset(&ramdisk.root, 0, sizeof(Node_File));
		
		strcpy(ramdisk.root->fileHdr.fileName, "/");
		ramdisk.root->fileHdr.flags = F_DIR | (S_IRUSR | S_IWUSR | S_IXUSR) | S_IRGRP | S_IROTH;

		ramdisk.root->fileHdr.fileCount = 0;		
		ramdisk.root->parent = NULL;
	}
	else { 
		if((status = makeRamdiskNode(rdfile, &ramdisk)) < 0) {
			goto Exit;
		}
		else {
			// We are done reading the ramdisk. Close the file
			close(rdfile);
			rdfile = -1;
		}
	}
	
	if(argc == 2) 
	{
		User_command cmd;
		do 
		{
			cmd = getUserOption();
			handleUserCommand(cmd);
		}
		while((cmd != CMD_QUIT) && (cmd != CMD_SAVE_AND_QUIT));
		
		status = 0;
		goto Exit;
	}
	
	if((status = ramdiskAddFile(&ramdisk, argv[2])) < 0)
	{
		goto Exit;
	}

	// Open ramdisk file for output
	if((rdfile = open(rdFileName,O_WRONLY)) < 0) {
		fprintf(stderr,"open(\"%s\",O_WRONLY): %s\n",
						rdFileName,
						strerror(errno));
		status = rdfile;
		goto Exit;
	}
	
	if((status = ramdiskWriteFile(rdfile, &ramdisk)) < 0)
	{
		goto Exit;
	}
	
Exit:
	if(rdfile >= 0) {
		close(rdfile);
		rdfile = 0;
	}

	// Free ramdisk
	freeRamdisk();

	return status;
}

void handleUserCommand(User_command cmd)
{
	char str[100];
	 
	switch(cmd)
	{
		case CMD_ADD_FILE:
			scanf("Please input the relative path of the file: %s\n", str);
			ramdiskAddFile(&ramdisk, str);
			break;
		case CMD_DELETE_FILE:
			scanf("Not Implemented\n");
			break;
		case CMD_ADD_FOLDER:
			scanf("Please input relative path of the new folder: %s\n", str);
			ramdiskAddFolder(&ramdisk, str);
			break;
		case CMD_PRINT:
			printFileName(ramdisk.root, 0);
			break;
		case CMD_SAVE:
			saveRamdisk(&ramdisk, rdFileName);
			break;
		case CMD_SAVE_AND_QUIT:
			saveRamdisk(&ramdisk, rdFileName);
			// Fall through
		case CMD_QUIT:
			break;
		default:
			break;
	}
}

User_command getUserOption(void)
{
	User_command choice = CMD_INVALID;
	
	do
	{
		printf("Choose one of the following options:\n");
		printf("a: Add file\n");
		printf("d: delete file or folder\n");
		printf("f: Add folder\n");
		printf("p: print ramdisk\n");
		printf("s: save ramdisk\n");
		printf("q: quit\n");
		printf("Please make input your choice: ");
		char ch = getchar();
	
		switch(ch)
		{
			case 'a':
				choice = CMD_ADD_FILE;				
				break;
			case 'd':
				choice = CMD_DELETE_FILE;
				break;		
			case 'f':
				choice = CMD_ADD_FOLDER;
				break;		
			case 'p':
				choice = CMD_PRINT;
				break;
			case 's':
				choice = CMD_SAVE;
				break;
			case 'q':
				if(dirty) 
				{
					printf("There are unsaved changes. Do you want to save? [Y/n]");
					ch = getchar();
					if(ch == 'n' || ch == 'N')
					{
						choice = CMD_QUIT;
					}
					else if(ch == 'y' || ch == 'Y' || ch == '\n')
					{
						choice = CMD_SAVE_AND_QUIT;
					}
					else
					{
						fprintf(stderr,"Invalid choice '%c'. ", ch);
						break;
					}
				}				
				else
				{
					choice = CMD_QUIT;
				}
				break;		
			default:
				fprintf(stderr,"Invalid choice '%c'. ", ch);
				break;
		}
	}
	while(choice == CMD_INVALID);
	
	return choice;
}
