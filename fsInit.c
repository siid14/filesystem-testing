/**************************************************************
* Class:  CSC-415-01 Fall 2023
* Names: Ruxue Jin, 
* Student IDs:923092817,
* GitHub Name:
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
#include "DE.h"
#include "VCB.h"


#define SIGNATURE 1234

int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */
	
	// determin if you need to format the volume of not

	// create a VCB pointer of blockSize and LBAread block 0
	VCB * blockBuf = malloc(blockSize);
	LBAread(blockBuf,1,0);


	// look at the signature to in the VCB struct to see if it matches
	// if it does not match, we need to initialize it	
	if(blockBuf->signature != SIGNATURE){

		//initialize the values in vcb
		strcpy(blockBuf ->signature,SIGNATURE);
		blockBuf->numberOfBlocks = numberOfBlocks;
		blockBuf->blockSize = blockSize;
		blockBuf->bitMapLocation = 1;
		blockBuf->rootDirLocation = 6;

		//initialize free space

		//initialize root directory

		// write vcb to block 0
		if (LBAwrite(blockBuf, 1, 0) != 1)
		{
			printf("[ERROR] LBAwrite() failed...\n");
		}

	}

	free(blockBuf);
	blockBuf = NULL;

	return 0;
	}
	
	
void exitFileSystem ()
	{
	printf ("System exiting\n");
	}