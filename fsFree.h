/**************************************************************
 * Class:  CSC-415-01 Fall 2023
 * Names: Sidney Thomas, Hoang-Anh Tran, Ruxue Jin, Yee-Tsing Yang
 * Student IDs: 918656419, 922617784, 923092817, 922359864
 * GitHub Name: siid14, htran31, RuxueJ, Y-Y1Q
 * Group Name: Alibaba
 * Project: Basic File System
 *
 * File: fsFree.h
 *
 * Description: Interface for free space management
 *
 *
 *
 **************************************************************/
#include "mfs.h"
#include "VCB.h"
#include "DE.h"
#include "fsLow.h"

// create a global char array for bitmap
unsigned char *bitMap;

// initFreeSpace is called when you initialize the volume
// it returns the block number where the freespace map starts
int initFreeSpace(int blockCount, int bytesPerBlock);

// if the volume is already initialized you need to call loadFreeSpace
// so the system has the freespace system ready to use.
int loadFreeSpace(int blockCount, int bytesPerBlock);

// * HELPER FUNCTIONS
// set the bit corresponding to blockNum to 1 (mark the block as used)
void setBitUsed(unsigned int blockNum);

// set the bit corresponding to blockNum to 0 (mark the block as free)
void setBitFree(unsigned int blockNum);

// Check if the bit corresponding to blockNum is used
// return value: 1 used  0 free
int isBitUsed(unsigned int blockNum);

// Find the first free block 
int getFreeBlockNum();

////        Contiguous      ////
// Take amount of blocks needed and allocate
// return the starting blockNum, -1 if free blocks are not enough
int allocBlocksCont(int blocksNeeded);

int releaseBlocksCont(int start, int count); // not needed for M1

////    Extent      ////
////    Prototypes from Lecture
////    TODO after M1

// definition of an extent
typedef struct extent
{
    int start;
    int count;
} extent, *pextent;

// allocateBlocks is how you obtain disk blocks.
// the first parameter is the number of blocks the caller requires
// the second parameter is the minimum number of blocks in any one extent
// except the last one.
// it returns an array of extent
extent *allocateBlocksExt(int required, int minPerExtent);

// This function returns blocks to the freespace system. If the caller wants
//  to free all the blocks in a series of extents, they should loop each extent
//  calling releaseBlocks for each extent
void releaseBlocksExt(int start, int count);