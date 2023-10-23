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
#include "fsFree.h"
#include "mfs.h"
#include "DE.h"
#include "VCB.h"

#define SIGNATURE 1234

int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize)
{
	printf("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */

	// determin if you need to format the volume of not

	// create a VCB pointer of blockSize and LBAread block 0
	vcb = malloc(blockSize);
	LBAread(vcb, 1, 0);

	// look at the signature to in the VCB struct to see if it matches
	// if it does not match, we need to initialize it
	if (vcb->signature != SIGNATURE)
	{

		// initialize the values in vcb
		strcpy(vcb->signature, SIGNATURE);
		vcb->numberOfBlocks = numberOfBlocks;
		vcb->blockSize = blockSize;

		// initialize free space
		vcb->bitMapLocation = initFreeSpace(numberOfBlocks, blockSize);

		// return next free block after block 1
		// or you can use the return value from function that initializes root directory
		vcb->rootDirLocation = getFreeBlockNum(bitMap, 1);

		// initialize root directory

		// write vcb to block 0
		if (LBAwrite(vcb, 1, 0) != 1)
		{
			printf("LBAwrite() failed...\n");
		}
	}
	else
	{
		// if the signature matches,
		// the volume has been initialized
		// * Implemented by Sid but need to be tested
		loadFreeSpace(numberOfBlocks, blockSize);
	}
	return 0;
}

void exitFileSystem()
{
	printf("System exiting\n");
	// TODO: any cleanup - freeing allocated memory - closing files
	// free(vcb);
	// vcb = NULL;
}
