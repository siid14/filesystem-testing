/**************************************************************
 * Class:  CSC-415-0# Fall 2021
 * Names: Sidney Thomas, Hoang-Anh Tran, Ruxue Jin, Yee-Tsing Yang
 * Student IDs: 918656419, 922617784, 923092817, 922359864
 * GitHub Name: siid14, htran31, RuxueJ, Y-Y1Q
 * Group Name: Alibaba
 * Project: Basic File System
 *
 * File: b_io.c
 *
 * Description: This header file implements a basic file system handling file operations
 *              such as opening, seeking, reading, writing, and closing files.
 *              Functions like `b_open`, `b_seek`, `b_write`, `b_read`, and `b_close`
 *              manage file descriptors, positions, data I/O, and file closure.
 *              The `b_init` function initializes the system,
 * 		        `b_getFCB` gets free File Control Blocks,
 *              and `b_fcb` defines the file control block structure.
 *              The implementation utilizes system libraries for I/O, memory, and system calls.
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

typedef struct b_fcb
{
	/** TODO add al the information you need in the file control block **/

	DE *fileInfo; // Directory entry of the file

	// extent * ExtentTable;
	char *buf;		  // holds the open file buffer
	int index;		  // holds the current position in the buffer
	int buflen;		  // holds how many valid bytes are in the buffer
	int currentBlock; // current block num
	int numOfBlocks;  // num of blocks used by the file
	int accessMode;	  // allowed permission for the file

	int parentLocation; // the starting block num of parent directory
	int indexInParent;	// The index of file in parent
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
				  // printf("b_open: File system initialized\n");
	}

	returnFd = b_getFCB(); // get the file descriptor for the opened file

	// check for error (all FCBs in use)
	if (returnFd == -1)
	{
		printf("b_open: All FCBs are in use\n");
		return -1;
	}

	// validate the path and ppInfo
	int pathValidationResult = parsePath(filename, ppi);

	if (pathValidationResult != 0 || ppi->lastElement == NULL)
	{
		printf("Invalid path\n");
		return -1;
	}

	// Case: path is valid, but it is a directory
	if (ppi->index != -1 && ppi->parent[ppi->index].isDir == 1)
	{
		printf("b_open(): Invalid path for a file\n");
		return -1;
	}

	fcbArray[returnFd].buf = malloc(vcb->blockSize);
	fcbArray[returnFd].fileInfo = malloc(sizeof(DE));
	if (fcbArray[returnFd].buf == NULL || fcbArray[returnFd].fileInfo == NULL)
	{
		printf("malloc failed in b_open()\n");
		return -1;
	}

	// populate needed fields
	fcbArray[returnFd].buflen = 0;
	fcbArray[returnFd].currentBlock = 0;
	fcbArray[returnFd].index = 0;
	fcbArray[returnFd].parentLocation = ppi->parent[0].location;

	fcbArray[returnFd].fileInfo->isDir = 0;
	fcbArray[returnFd].accessMode = flags;

	// Case: file does not exist && flag is O_CREATE
	if (ppi->index == -1)
	{

		if (flags & O_CREAT)
		{
			// printf("inside O_CREAT\n");
			int freeIndex = findFreeDE(ppi->parent);
			if (freeIndex == -1)
			{
				printf("Error: no free entry in parent directory [%s]\n", ppi->parent[0].fileName);
				return (-1);
			}
			// printf("b_open indexInParent: %d\n", fcbArray[returnFd].indexInParent);
			fcbArray[returnFd].indexInParent = freeIndex;
			// printf("b_open indexInParent: %d\n", fcbArray[returnFd].indexInParent);
			// printf("b_open returnFd: %d\n", returnFd);

			// Create DE info

			strcpy(fcbArray[returnFd].fileInfo->fileName, ppi->lastElement);
			fcbArray[returnFd].fileInfo->location = 0;
			fcbArray[returnFd].fileInfo->size = 0;
			fcbArray[returnFd].numOfBlocks = 0;
		}
		else
		{
			return -1;
		}
	}
	else // Get DE info
	{
		strcpy(fcbArray[returnFd].fileInfo->fileName, ppi->lastElement);
		fcbArray[returnFd].indexInParent = ppi->index;

		// Case: O_TRUNC flag
		if (flags & O_TRUNC)
		{
			freeBlocksDE(&ppi->parent[ppi->index]);
			fcbArray[returnFd].fileInfo->location = 0;
			fcbArray[returnFd].fileInfo->size = 0;
			fcbArray[returnFd].numOfBlocks = 0;
		}
		else
		{
			fcbArray[returnFd].fileInfo->location = ppi->parent[ppi->index].location;
			fcbArray[returnFd].fileInfo->size = ppi->parent[ppi->index].size;
			fcbArray[returnFd].numOfBlocks = (fcbArray[returnFd].fileInfo->size + (vcb->blockSize - 1)) / vcb->blockSize;
		}
	}

	// printf("\n\n---- b_open() finished ----\n\n");
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

	int filePositionPtr = 0;

	switch (whence)
	{
	case SEEK_SET: // start from beginning of the file
		filePositionPtr = offset;

		if (filePositionPtr > fcbArray[fd].fileInfo->size)
		{
			filePositionPtr = fcbArray[fd].fileInfo->size;
		}

		break;
	case SEEK_END: // end of file
		filePositionPtr = fcbArray[fd].fileInfo->size;

		break;
	case SEEK_CUR: // move file pointer based on the offset
		filePositionPtr += offset;

		if (filePositionPtr > fcbArray[fd].fileInfo->size)
		{
			filePositionPtr = fcbArray[fd].fileInfo->size;
		}

		break;
	default:
		printf("Invalid parameters for b_seek()\n");
		return -1;
	}

	return filePositionPtr;
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
	int numOfBlocksToWrite;		  // holds the number of whole blocks
	int remainingBytesInMyBuffer; // holds how many bytes remaining in the buffer

	if (startup == 0)
		b_init(); // Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
	{
		return (-1); // invalid file descriptor
	}

	// Check if file not open for this descriptor
	if (fcbArray[fd].buf == NULL || fcbArray[fd].fileInfo == NULL)
	{
		return -1;
	}

	//  check for write permission
	if ((fcbArray[fd].accessMode & O_WRONLY) != O_WRONLY)
	{
		printf("ERROR: no permission to write\n");
		return -1;
	}

	off_t fileSize = b_seek(fd, 0, SEEK_END);

	// check if the current size is enough for count bytes
	if (count > fcbArray[fd].fileInfo->size)
	{
		int newFileSize = count;
		int newNumOfBlocks = (newFileSize + (vcb->blockSize - 1)) / vcb->blockSize;

		// free old blocks that are not enough
		for (int i = fcbArray[fd].fileInfo->location; i < fcbArray[fd].numOfBlocks; i++)
		{
			setBitFree(i);
		}

		int newLocation = allocBlocksCont(newNumOfBlocks);
		if (newLocation == -1)
		{
			printf("error: no enough space for file");
			return (-1);
		}

		fcbArray[fd].fileInfo->location = newLocation;
		fcbArray[fd].fileInfo->size = newFileSize;
		fcbArray[fd].numOfBlocks = newNumOfBlocks;
	}

	// number of bytes available to copy from buffer
	remainingBytesInMyBuffer = vcb->blockSize - fcbArray[fd].index;

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
		numOfBlocksToWrite = part3 / vcb->blockSize;
		part2 = numOfBlocksToWrite * vcb->blockSize;

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
		bytesWrite = LBAwrite(buffer + part1, numOfBlocksToWrite,
							  fcbArray[fd].currentBlock + fcbArray[fd].fileInfo->location);
		fcbArray[fd].currentBlock += numOfBlocksToWrite;
		part2 = bytesWrite;
	}

	if (part3 > 0)
	{
		// write the buf from part1, and then refill
		bytesWrite = LBAwrite(fcbArray[fd].buf, 1,
							  fcbArray[fd].currentBlock + fcbArray[fd].fileInfo->location);

		fcbArray[fd].currentBlock += 1;

		fcbArray[fd].index = 0;
		fcbArray[fd].buflen = vcb->blockSize - bytesWrite;

		if (bytesWrite < part3)
		{
			part3 = bytesWrite;
		}

		if (part3 > 0)
		{
			memcpy(fcbArray[fd].buf + fcbArray[fd].index, buffer + part1 + part2, part3);
			LBAwrite(fcbArray[fd].buf, 1,
					 fcbArray[fd].currentBlock + fcbArray[fd].fileInfo->location);
			fcbArray[fd].index = fcbArray[fd].index + part3;
		}
	}

	bytesReturned = part1 + part2 + part3;

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
	int bytesRead;			 // Number of bytes read from the file
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
	if (fcbArray[fd].buf == NULL || fcbArray[fd].fileInfo == NULL)
	{
		return -1;
	}

	// Check if file is allowed to be read
	if ((fcbArray[fd].accessMode & O_RDONLY) != O_RDONLY)
	{
		printf("ERROR: No permission to read\n");
		return -1;
	}

	remain = fcbArray[fd].buflen - fcbArray[fd].index;

	// Calculate the total number of bytes already delivered to track the number of bytes processed
	int bytesDelivered = (fcbArray[fd].currentBlock * vcb->blockSize) - remain;

	// Check if the requested count exceeds the file size
	if ((count + bytesDelivered) > fcbArray[fd].fileInfo->size)
	{
		// If yes, limit the count to the remaining bytes until the end of the file
		count = fcbArray[fd].fileInfo->size - bytesDelivered;
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
		copiedBlocks = part3 / vcb->blockSize;
		part2 = copiedBlocks * vcb->blockSize;
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
		part2 = bytesRead;
	}

	// If need to refill the buffer to copy more bytes to the user
	if (part3 > 0)
	{
		bytesRead = LBAread(fcbArray[fd].buf, 1, fcbArray[fd].currentBlock + fcbArray[fd].fileInfo->location);
		bytesRead = bytesRead * vcb->blockSize;
		fcbArray[fd].currentBlock += 1;

		// Reset the offset and buffer length
		fcbArray[fd].index = 0;
		fcbArray[fd].buflen = bytesRead;

		// Check to ensure part 3 does not exceed the actual number of bytes read
		if (bytesRead < part3)
		{
			part3 = bytesRead;
		}

		if (part3 > 0)
		{
			memcpy(buffer + part1 + part2, fcbArray[fd].buf + fcbArray[fd].index, part3);
			// Update the index to the next unread data
			fcbArray[fd].index = fcbArray[fd].index + part3;
		}
	}
	bytesReturned = part1 + part2 + part3;
	return bytesReturned;
}

// Interface to Close the file
int b_close(b_io_fd fd)
{
	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
	{
		return (-1); // invalid file descriptor
	}

	DE *tempDir = loadDirLocation(fcbArray[fd].parentLocation);

	// printf("b_close indexInParent: %d\n", fcb->indexInParent);
	// printf("b_ returnFd: %d\n", fd);

	// write unused buffer
	if (fcbArray[fd].fileInfo->size > 0 && fcbArray[fd].index < vcb->blockSize)
	{
		char *temp = malloc(vcb->blockSize);
		int remain = vcb->blockSize - fcbArray[fd].index;
		memcpy(temp, fcbArray[fd].buf, remain);
		LBAwrite(temp, 1, fcbArray[fd].fileInfo->location + fcbArray[fd].currentBlock);
		free(temp);
		temp = NULL;
	}

	// Update parent DE info

	int indexInParent = fcbArray[fd].indexInParent;

	strcpy(tempDir[indexInParent].fileName, fcbArray[fd].fileInfo->fileName);
	tempDir[indexInParent].size = fcbArray[fd].fileInfo->size;
	tempDir[indexInParent].isDir = fcbArray[fd].fileInfo->isDir;
	tempDir[indexInParent].location = fcbArray[fd].fileInfo->location;

	time_t t = time(NULL);
	if (fcbArray[fd].accessMode & O_CREAT)
	{
		tempDir[indexInParent].timeCreated = t;
	}

	tempDir[indexInParent].timeLastAccessed = t;
	tempDir[indexInParent].timeLastModified = t;

	writeDir(tempDir);

	free(fcbArray[fd].buf);
	fcbArray[fd].buf = NULL;

	free(fcbArray[fd].fileInfo);
	fcbArray[fd].fileInfo = NULL;

	free(tempDir);
	tempDir = NULL;

	return 0;
}
