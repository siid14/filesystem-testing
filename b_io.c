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
// * burning comment
// * burning comment 2
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h> // for malloc
#include <string.h> // for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "b_io.h"

#include "mfs.h"
#include "fsLow.h"

#define MAXFCBS 20
#define B_CHUNK_SIZE 512

typedef struct b_fcb
{
	/** TODO add al the information you need in the file control block **/
	char *buf;	// holds the open file buffer
	int index;	// holds the current position in the buffer
	int buflen; // holds how many valid bytes are in the buffer

	DE *fileInfo;
	int currentBlock; // current block number being read from the file
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

	// Check for an error (all FCBs in use)
    if (returnFd == -1)
        return -1;

    // Open the file
    int fileInfo = fs_open(filename, flags);

    // Check for errors in opening the file
    if (fileInfo == -1)
        return -1;

    // Allocate a buffer for the file in the FCB
    fcbArray[returnFd].buf = (char *)malloc(B_CHUNK_SIZE);
    // Check for errors in malloc
    if (fcbArray[returnFd].buf == NULL)
        return -1;

    // Initialize other fields of the FCB
    fcbArray[returnFd].index = 0;
    fcbArray[returnFd].buflen = 0;
	fcbArray[returnFd].currentBlock = 0; 
    //

	// Check the access mode specified by the flags in a file open operation
	if (flags & O_ACCMODE == O_RDONLY)
		{
		printf("b_open: Opening file for read...\n");
		}
	else if (flags & O_ACCMODE == O_WRONLY)
		{
		printf("b_open: Opening file for write...\n");
		}
	else if (flags & O_ACCMODE == O_RDWR)
		{
		printf("b_open: Opening file for read and write...\n");
		}
	else
		{
		return -1;
		}

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
int b_write(b_io_fd fd, char *buffer, int count)
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
	int bytesRead = 0;   // Number of bytes read from the file
    int bytesReturned;   // Total number of bytes returned as the result of b_read
    int part1, part2, part3;  // Variables to manage the data reading process in segments
    int copiedBlocks;   // Number of blocks to be copied during data reading to control the reading process when data spans across multiple blocks
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
		//printf("count: %d\n", count);
    }

    // Part 1 - The first part reads from the buffer
    // Part 2 - The data reads from additional blocks
    // Part 3 - The remaining part of data reads from the buffer

    // Determine if enough data is available in the buffer to fulfill the requested count of bytes
    if (remain >= count)
    {
        part1 = count; // Count is completely buffered
        part2 = 0;     // No data from additional blocks is required
        part3 = 0;     // No need to read more data from the next buffer
    }
    // If not, read data from both the buffer and additional blocks
    else
    {
        part1 = remain;  // Read from the buffer
        part3 = count - remain;  // Additional data needed beyond the buffer if part 1 is not enough

        // Calculate the number of blocks to copy for part2
        copiedBlocks = part3 / B_CHUNK_SIZE;
        part2 = copiedBlocks * B_CHUNK_SIZE;
        part3 = part3 - part2;  // Recalculate the remaining bytes for part3
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
}
