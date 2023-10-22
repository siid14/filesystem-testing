/**************************************************************
* Class:  CSC-415-01 Fall 2023
* Names: Ruxue Jin, 
* Student IDs:923092817,
* GitHub Name:
* Group Name:Alibaba
* Project: Basic File System
*
* File: vcb.h
*
* Description: Header file for the freespace system that manages
* that allocated and free blocks on disk.
*
* 
*
**************************************************************/
// definition of an extent
typedef struct extent{
    int start;
    int count;
}extent, * pextent;

// initFreeSpace is called when you initialize the volume
// it returns the block number where the freespace map starts
int initFreeSpace(int blockCount, int bytesPerBlock);

// if the volume is already initialized you need to call loadFreeSpace
// so the system has the freespace system ready to use.
int loadFreeSpace(int blockCount, int bytesPerBlock);


// allocateBlocks is how you obtain disk blocks. 
// the first parameter is the number of blocks the caller requires
// the second parameter is the minimum number of blocks in any one extent
// except the last one.
// it returns an array of extent
extent * allocateBlocks(int required, int minPerExtent);


//This function returns blocks to the freespace system. If the caller wants
// to free all the blocks in a series of extents, they should loop each extent
// calling releaseBlocks for each extent
void releaseBlocks(int start, int count);