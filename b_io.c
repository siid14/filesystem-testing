/**************************************************************
 * Class:  CSC-415-0# Fall 2021
 * Names:
 * Student IDs:
 * GitHub Name:
 * Group Name:
 * Project: Basic File System
 *
 * File: b_io.c
 *
 * Description: Basic File System - Key File I/O Operations
 *
 **************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h> // for malloc
#include <string.h> // for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "b_io.h"
#include "mfs.h"
#include "fsFree.h"
#include "fsParse.h"
#include "fsDir.h"
#include "fsLow.h"

#define MAXFCBS 20
#define B_CHUNK_SIZE 512
#define extend_block_count 10

typedef struct b_fcb
{
	/** TODO add al the information you need in the file control block **/
	DE *fileInfo;
	// extent * ExtentTable;
	char *buf;	// holds the open file buffer
	int index;	// holds the current position in the buffer
	int buflen; // holds how many valid bytes are in the buffer
	int currentBlock;
	int accessMode;
	char *path; // points to the path passed to b_open()
} b_fcb;

b_fcb fcbArray[MAXFCBS];

int startup = 0; // Indicates that this has not been initialized

// Method to initialize our file system
void b_init()
{
	// init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
	{
		fcbArray[i].buf = NULL; // indicates a free fcbArray
	}

	startup = 1;
}

// Method to get a free FCB element
b_io_fd b_getFCB()
{
	for (int i = 0; i < MAXFCBS; i++)
	{
		if (fcbArray[i].buf == NULL)
			if (fcbArray[i].buf == NULL)
			{
				return i; // Not thread safe (But do not worry about it for this assignment)
			}
	}
	return (-1); // all in use
}

/* File Open Function
 this function initializes a File Control Block (FCB) for the specified file and returns a file descriptor
 it opens the file with the provided filename and flags, allocates a buffer for file I/O, and tracks the current file position
 parameters:
 - filename: The name of the file to open
 - flags: Flags indicating the file access mode (O_RDONLY, O_WRONLY, or O_RDWR)
 returns:
 - a file descriptor (file handle) if the operation is successful
 - returns -1 if there are no available FCBs, errors opening the file, or memory allocation issues.
 */
b_io_fd b_open(char *filename, int flags)
{
	b_io_fd returnFd;

	if (startup == 0)
	{
		b_init(); // initialize our system
		printf("b_open: File system initialized\n");
	}

	returnFd = b_getFCB(); // get the file descriptor

	// check for error (all FCBs in use)
	if (returnFd == -1)
	{
		printf("b_open: All FCBs are in use\n");
		return -1;
	}

	// open the file
	int fileInfo = fs_open(filename, flags);

	// check for opening file errors
	if (fileInfo == -1)
	{
		printf("b_open: Error opening the file\n");
		return -1;
	}

	// allocate a buffer for the file in the FCB
	fcbArray[returnFd].buf = (char *)malloc(B_CHUNK_SIZE);

	// check for errors in malloc
	if (fcbArray[returnFd].buf == NULL)
	{
		printf("b_open: Error allocating memory for file buffer\n");
		return -1;
	}

	// check if file descriptor is valid and init FCB
	if (returnFd != -1)
	{
		fcbArray[returnFd].index = 0;				  // file pointer position to the beginning of the file
		fcbArray[returnFd].buflen = 0;				  // buffer length to 0 (buffer is currently empty)
		fcbArray[returnFd].currentBlock = 0;		  // current block to the first block of the file
		fcbArray[returnFd].fileInfo = (DE *)fileInfo; // file information with the file's directory entry (DE)
		printf("b_open: FCB initialized successfully\n");
	}

	// ? if needed, can add a vraiable to track the access mode (read, write, read/write)
	// check the access mode specified by the flags in a file open operation
	if ((flags & O_ACCMODE) == O_RDONLY)
	{
		printf("b_open: Opening file for read...\n");
	}
	else if ((flags & O_ACCMODE) == O_WRONLY)
	{
		printf("b_open: Opening file for write...\n");
	}
	else if ((flags & O_ACCMODE) == O_RDWR)
	{
		printf("b_open: Opening file for read and write...\n");
	}
	else
	{
		printf("b_open: Invalid access mode\n");
		return -1;
	}

	return (returnFd); // all set
}

/* File Seek Function
 moves the file pointer inside an open file to a new position based on 'whence' and 'offset'
 it updates the file pointer (index) in the File Control Block (FCB) and returns the new position
 parameters:
   - fd: File descriptor
   - offset: Offset to move the file pointer
   - whence: Reference point for the seek operation (SEEK_SET, SEEK_CUR, SEEK_END)
 returns:
 - the new position within the file, or -1 in case of an error
*/
int b_seek(b_io_fd fd, off_t offset, int whence)
{
	if (startup == 0)
		b_init(); // initialize our system

	// check fd between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
	{
		printf("b_seek: Invalid file descriptor\n");
		return (-1); // invalid file descriptor
	}

	return (0); // Change this
}

/*
// Interface to write function
// fd is the destinatation file descriptor
// buffer contains the contents to write
// count is the number of byte needed to write
int b_write(b_io_fd fd, char *buffer, int count)
{
	int bytesWrite;				  // for our writes
	int bytesReturned;			  // what we will return
	int part1, part2, part3;	  // holds the three potential bytes
	int numberOfBlocksToCopy;	  // holds the number of whole blocks
	int remainingBytesInMyBuffer; // holds how many bytes remaining in the buffer

	if (startup == 0)
		b_init(); // Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
	{
		return (-1); // invalid file descriptor
	}

	// check if we have the write mode
	if ((fcbArray[fd].accessMode & (O_RDWR | O_WRONLY)) == 0)
	{
		// Code inside the block will be executed if O_RDONLY is set in accessMode
		printf("[ERROR] does not have access to write!\n");
		return -1;
	}

	// check if the current size is enough for extra count bytes
	off_t fileSize = seek(fd, 0, SEEK_END);

	if (fileSize + count > fcbArray[fd].fileInfo->size)
	{
		printf("\nfile not enough space,reallocate the memory\n");
		int currentBlockNumber = (fcbArray[fd].fileInfo->size +
								  B_CHUNK_SIZE - 1) /
								 B_CHUNK_SIZE;
		int newBlockNumber = currentBlockNumber + extend_block_count;
		int newLocation = allocBlocksCont(newBlockNumber);
		if (newLocation == -1)
		{
			printf("error: no enough space for file");
			return (-1);
		}
		for (int i = 0; i < currentBlockNumber; i++)
		{
			setBitFree(fcbArray[fd].fileInfo->location + 1);
		}

		fcbArray[fd].fileInfo->location = newLocation;
	}

	// number of bytes available to copy from buffer
	remainingBytesInMyBuffer = B_CHUNK_SIZE - fcbArray[fd].index;

	// Part1 is the first copy of data which will be from the buffer
	// it will be the lesser of the requested amount of the number
	if (count <= remainingBytesInMyBuffer)
	{
		part1 = count;
		part2 = 0;
		part3 = 0;
	}
	else
	{
		part1 = remainingBytesInMyBuffer;

		// part 1 is not enough
		part3 = count - remainingBytesInMyBuffer;
		numberOfBlocksToCopy = part3 / B_CHUNK_SIZE;
		part2 = numberOfBlocksToCopy * B_CHUNK_SIZE;

		// reduce part 3 by the number of bytes that can be copied
		// part 3 at this point must be less than the block size
		part3 = part3 - part2;
	}

	if (part1 > 0)
	{
		memcpy(fcbArray[fd].buf + fcbArray[fd].index, buffer, part1);
		fcbArray[fd].index = fcbArray[fd].index + part1;
	}

	if (part2 > 0)
	{
		// printf("[debug] inside part 2\n");
		bytesWrite = LBAwrite(buffer + part1, numberOfBlocksToCopy,
							  fcbArray[fd].currentBlock + fcbArray[fd].fileInfo->location);
		fcbArray[fd].currentBlock += numberOfBlocksToCopy;
		part2 = bytesWrite * B_CHUNK_SIZE;
	}

	if (part3 > 0)
	{
		// printf("[debug] inside part 3\n");
		memcpy(fcbArray[fd].buf + fcbArray[fd].index, buffer + part1 + part2, part3);
		fcbArray[fd].index += part3;
		// printf("[debug] bufIndex: %d\n", fcbArray[fd].bufIndex);
		// printf("[debug] my current buffer: %s\n", fcbArray[fd].buf);
		if (fcbArray[fd].index == B_CHUNK_SIZE - 1)
		{
			// printf("[debug] block is full\n");
			// printf("[debug] buf: %s\n", fcbArray[fd].buf);
			LBAwrite(fcbArray[fd].buf, 1, fcbArray[fd].currentBlock + fcbArray[fd].fileInfo->location);
			fcbArray[fd].currentBlock += 1;
			fcbArray[fd].index = 0;
		}
	}

	bytesReturned = part1 + part2 + part3;
	fcbArray[fd].fileInfo->size += bytesReturned;

	return (bytesReturned); // Change this
}
*/

// Interface to read a buffer

// Filling the callers request is broken into three parts
// Part 1 is what can be filled from the current buffer, which may or may not be enough
// Part 2 is after using what was left in our buffer there is still 1 or more block
//        size chunks needed to fill the callers request.  This represents the number of
//        bytes in multiples of the blocksize.
// Part 3 is a value less than blocksize which is what remains to copy to the callers buffer
//        after fulfilling part 1 and part 2.  This would always be filled from a refill
//        of our buffer.
//  +-------------+------------------------------------------------+--------+
//  |             |                                                |        |
//  | filled from |  filled direct in multiples of the block size  | filled |
//  | existing    |                                                | from   |
//  | buffer      |                                                |refilled|
//  |             |                                                | buffer |
//  |             |                                                |        |
//  | Part1       |  Part 2                                        | Part3  |
//  +-------------+------------------------------------------------+--------+
int b_read(b_io_fd fd, char *buffer, int count)
{
	int bytesRead = 0;		 // Number of bytes read from the file
	int bytesReturned;		 // Total number of bytes returned as the result of b_read
	int part1, part2, part3; // Variables to manage the data reading process in segments
	int copiedBlocks;		 // Number of blocks to be copied during data reading to control the reading process when data spans across multiple blocks
	int remain;

	if (startup == 0)
		b_init(); // Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
	{
		return (-1); // invalid file descriptor
	}

	// Check if file not open for this descriptor
	if (fcbArray[fd].buf == NULL)
	{
		return -1;
	}

	remain = fcbArray[fd].buflen - fcbArray[fd].index;

	// Calculate the total number of bytes already delivered to track the number of bytes processed
	int bytesDelivered = (fcbArray[fd].currentBlock * B_CHUNK_SIZE) - remain;

	// Check if the requested count exceeds the file size
	if ((count + bytesDelivered) > fcbArray[fd].fileInfo->fileSize)
	{
		// If yes, limit the count to the remaining bytes until the end of the file
		count = fcbArray[fd].fileInfo->fileSize - bytesDelivered;
		// printf("count: %d\n", count);
	}

	// Part 1 - The first part reads from the buffer
	// Part 2 - The data reads from additional blocks
	// Part 3 - The remaining part of data reads from the buffer

	// Determine if enough data is available in the buffer to fulfill the requested count of bytes
	if (remain >= count)
	{
		part1 = count; // Count is completely buffered
		part2 = 0;	   // No data from additional blocks is required
		part3 = 0;	   // No need to read more data from the next buffer
	}
	// If not, read data from both the buffer and additional blocks
	else
	{
		part1 = remain;			// Read from the buffer
		part3 = count - remain; // Additional data needed beyond the buffer if part 1 is not enough

		// Calculate the number of blocks to copy for part2
		copiedBlocks = part3 / B_CHUNK_SIZE;
		part2 = copiedBlocks * B_CHUNK_SIZE;
		part3 = part3 - part2; // Recalculate the remaining bytes for part3
	}

	// If there is enough data available to fulfill part 1
	if (part1 > 0)
	{
		// Data from the buffer is transferred to the user's buffer
		memcpy(buffer, fcbArray[fd].buf + fcbArray[fd].index, part1);
		// Update the index to the next unread data
		fcbArray[fd].index = fcbArray[fd].index + part1;
	}

	// If there is additional data that needs to be read from storage
	if (part2 > 0)
	{
		// Use LBAread to read data from storage and store it in the user's buffer
		bytesRead = LBAread(buffer + part1, copiedBlocks, fcbArray[fd].currentBlock + fcbArray[fd].fileInfo->location);
		// Update the current block pointer to track the position in the file
		fcbArray[fd].currentBlock += copiedBlocks;
		// Update part2 based on the number of bytes read
		part2 = bytesRead * B_CHUNK_SIZE;
	}

	// If need to refill the buffer to copy more bytes to the user
	if (part3 > 0)
	{
		bytesRead = LBAread(fcbArray[fd].buf, 1, fcbArray[fd].currentBlock + fcbArray[fd].fileInfo->location);
		bytesRead = bytesRead * B_CHUNK_SIZE;
		fcbArray[fd].currentBlock += 1;

		// Reset the offset and buffer length
		fcbArray[fd].index = 0;
		fcbArray[fd].buflen = bytesRead;

		// Check to ensure part 3 does not exceed the actual number of bytes read
		if (bytesRead < part3)
		{
			part3 = bytesRead;
		}

		memcpy(buffer + part1 + part2, fcbArray[fd].buf + fcbArray[fd].index, part3);
		// Update the index to the next unread data
		fcbArray[fd].index = fcbArray[fd].index + part3;
	}
	bytesReturned = part1 + part2 + part3;
	return bytesReturned;
}

// Interface to Close the file
int b_close(b_io_fd fd)
{
	// write unused buffer
	if (fcbArray[fd].index < B_CHUNK_SIZE)
	{
		char *temp = malloc(B_CHUNK_SIZE);
		memcpy(temp, fcbArray[fd].buf, fcbArray[fd].index + 1);
		LBAwrite(temp, 1, fcbArray[fd].fileInfo->location + fcbArray[fd].currentBlock);
		free(temp);
	}

	// update parent DEs
	int validPath = parsePath(fcbArray[fd].path, ppi);

	if (validPath != 0)
	{
		printf("In b_open(), parsePath() return invalid\n");
		return 1;
	}

	int freeIndex = findFreeDE(ppi->parent);

	strcpy(ppi->parent[freeIndex].fileName, ppi->lastElement);
	ppi->parent[freeIndex].size = fcbArray[fd].fileInfo->size;
	ppi->parent[freeIndex].isDir = fcbArray[fd].fileInfo->isDir;
	ppi->parent[freeIndex].location = fcbArray[fd].fileInfo->location;

	int blockCount = (ppi->parent->size + vcb->blockSize - 1) / vcb->blockSize;

	// write parent dir to disk
	int checkVal = LBAwrite(ppi->parent, blockCount, ppi->parent->location);

	if (checkVal != blockCount)
	{
		printf("\nError: LBAwrite() failed in b_close(), b_io.c\n");
		return 1;
	}

	free(fcbArray[fd].buf);
	fcbArray[fd].buf = NULL;

	fcbArray[fd].fileInfo = NULL;
	return 0;
}
