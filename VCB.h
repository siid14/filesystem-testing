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
* Description: struct of volume control block
*
* 
*
**************************************************************/
typedef struct VCB
  {
    unsigned int numberOfBlocks;	// Number of blocks in the volume
    unsigned int blockSize;	// Size of each block in bytes

    unsigned long signature;	// Signature for VCB struct

    unsigned int bitMapLocation;	// starting block num of bitMap
    unsigned int bitMapBytesCount; // the needed bytes for bitMap

    unsigned int rootDirLocation;   // starting block num of the root directory
  } VCB;

  VCB * vcb;
