/**************************************************************
 * Class:  CSC-415-01 Fall 2023
 * Names: Sidney Thomas, Hoang-Anh Tran, Ruxue Jin, Yee-Tsing Yang
 * Student IDs: 918656419, 922617784, 923092817, 922359864
 * GitHub Name: siid14, htran31, RuxueJ, Y-Y1Q
 * Group Name:Alibaba
 * Project: Basic File System
 *
 * File: fsInit.c
 *
 * Description: Main driver for file system assignment.
 *
 * This file is where you will start and initialize your system
 *
 **************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "fsLow.h"
#include "mfs.h"
#include "fsDir.h"
#include "fsFree.h"

// initial number of directory entries in each directory
#define initialDirEntries 50
#define SIGNATURE 1234

int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize)
{
	printf("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */

	// determin if you need to format the volume of not

	// create a VCB pointer of blockSize and LBAread block 0
	VCB *vcb = malloc(blockSize);
	if (vcb == NULL)
	{
		printf("vcb malloc() failed in fsInit.c\n");
		return -1;
	}

	LBAread(vcb, 1, 0);

	// look at the signature to in the VCB struct to see if it matches
	// if it does not match, we need to initialize it
	if (vcb->signature != SIGNATURE)
	{
		printf("\nSignature not found,  start initializing\n\n");

		// initialize the values in vcb
		vcb->signature = SIGNATURE;
		printf("vcb->signature: %ld\n\n", vcb->signature);

		vcb->numberOfBlocks = numberOfBlocks;
		vcb->blockSize = blockSize;

		// initialize free space
		vcb->bitMapLocation = initFreeSpace(numberOfBlocks, blockSize);
		printf("\n------ OUTSIDE initFreeSpace() ------\n\n\n");
		printf("vcb->bitMapLocation: %d\n\n", vcb->bitMapLocation);
		printf("After init free space \n");
		printf("bitMap[0]: %x\n", bitMap[0]);
		printf("bitMap[1]: %x\n", bitMap[1]);
		printf("isBitUsed[1], 0 free, 1 used: %d\n", isBitUsed(1));

		// initialize root directory
		vcb->rootDirLocation = initDir(initialDirEntries, NULL, blockSize);
		printf("\n------ OUTSIDE THE initDir function------\n\n\n");
		printf("vcb->rootDirLocation: %d\n", vcb->rootDirLocation);
		printf("isBitUsed[6], 0 free, 1 used: %d\n", isBitUsed(6));

		//write vcb to block 0
		if (LBAwrite(vcb, 1, 0) != 1)
		{
			printf("In fsInit.c:  LBAwrite() failed on vcb\n");
		}
	}
	// signature matched reload the free space
	else
	{

		printf("\nSignature found,  reloading free space\n\n");
		vcb->bitMapLocation = loadFreeSpace(numberOfBlocks, blockSize);
	}

	free(vcb);
	vcb = NULL;

	return 0;
}

void exitFileSystem()
{
	free(bitMap);
	bitMap = NULL;
	printf("System exiting\n");
}