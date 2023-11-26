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
 * Description: 
 * This header file contains declarations for managing free space
 * within the basic file system. It provides methods for initializing
 * and loading free space, setting bits to mark blocks as used or free,
 * finding free blocks, and allocating contiguous or extent-based blocks.
 * The functions defined here enable the efficient handling of block allocation 
 * and deallocation, contributing to the effective management of available 
 * free space in the file system.
 **************************************************************/
#ifndef _FS_FREE_H
#define _FS_FREE_H
#include "mfs.h"

// share global char array for bitmap
// malloc() may assign extra bytes, but don't use those extra bytes
// there are maybe some extra bits at last byte
// use numOfBlocks in vcb to check how many bits are necessary
extern unsigned char *bitMap;

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


#endif