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
		{
			return i; // Not thread safe (But do not worry about it for this assignment)
		}
	}
	return (-1); // all in use
}

// Interface to open a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY, O_WRONLY, or O_RDWR
b_io_fd b_open(char *filename, int flags)
{
	b_io_fd returnFd;

	//*** TODO ***:  Modify to save or set any information needed
	//
	//

	if (startup == 0)
		b_init(); // Initialize our system

	returnFd = b_getFCB(); // get our own file descriptor
						   // check for error - all used FCB's

	return (returnFd); // all set
}

// Interface to seek function
int b_seek(b_io_fd fd, off_t offset, int whence)
{
	if (startup == 0)
		b_init(); // Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
	{
		return (-1); // invalid file descriptor
	}

	return (0); // Change this
}

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
		bytesWrite = LBAWrite(buffer + part1, numberOfBlocksToCopy,
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

	if (startup == 0)
		b_init(); // Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
	{
		return (-1); // invalid file descriptor
	}

	return (0); // Change this
}

// Interface to Close the file
int b_close(b_io_fd fd)
{
}
