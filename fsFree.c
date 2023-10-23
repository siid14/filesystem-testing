/**************************************************************
 * Class:  CSC-415-01 Fall 2023
 * Names: Sidney Thomas, Hoang-Anh Tran, Ruxue Jin, Yee-Tsing Yang
 * Student IDs: 918656419, 922617784, 923092817, 922359864
 * GitHub Name: siid14, htran31, RuxueJ, Y-Y1Q
 * Group Name: Alibaba
 * Project: Basic File System
 *
 * File: fsFree.c
 *
 * Description: Implementation for free space management
 *
 *
 *
 **************************************************************/
#include "fsFree.h"

// use value provided in fsInit.c to initialize bitmap
int initFreeSpace(int blockCount, int bytesPerBlock)
{
    // number of blocks = number of bits in bitmap
    // 1 byte = 8 bit, calculate the bytes needed for bitmap, ceiling round up
    unsigned int bytesBitmap = (numberOfBlocks + 8 - 1) / 8;
    vcb->bitMapBytesCount = bytesBitmap;

    // calculate the blocks needed for bitmap
    unsigned int blocksBitmap = (bytesBitmap + blockSize - 1) / blockSize;

    // blocks used by VCB and bit map during initialization
    unsigned int blocksInitUsed = blocksBitmap + 1;

    bitMap = malloc(blocksBitmap * blockSize);

    // Initialize bit map, mark all bits as free
    for (int i = 0; i < bytesBitmap; i++)
    {
        bitMap[i] = 0x00;
    }

    // Set the block 0 (VCB) and blocks occupied by bitmap as used
    for (int i = 0; i < blocksInitUsed; i++)
    {
        setBitUsed(bitMap, i);
    }

    // write bitmap to disk
    LBAwrite(bitMap, blocksBitmap, 1);

    // return 1, indicating bitMap start at block 1
    return 1;
}

// * Implemented by Sid but need to be tested
// if the volume is already initialized you need to call loadFreeSpace
// so the system has the freespace system ready to use.
int loadFreeSpace(int blockCount, int bytesPerBlock)
{
    // calculate the number of bytes needed for the bitmap
    unsigned int bytesBitmap = (blockCount + 8 - 1) / 8;

    // calculate the number of blocks needed for the bitmap
    unsigned int blocksBitmap = (bytesBitmap + bytesPerBlock - 1) / bytesPerBlock;

    // allocate memory for the bitmap
    bitMap = malloc(blocksBitmap * bytesPerBlock);

    // read the bitmap from disk (from a specified block)
    LBAread(bitMap, blocksBitmap, 1);

    return 1; // return 1 to indicate success or handle errors accordingly.
}

// set the bit corresponding to blockNum to 1 (mark the block as used)
void setBitUsed(unsigned char *bitMap, unsigned int blockNum)
{
    unsigned int byteIndex = blockNum / 8; // calculate the byte index in the bitmap
    unsigned int bitIndex = blockNum % 8;  // calculate the bit index within the byte

    unsigned char mask = 1 << (7 - bitIndex); // create a 1-byte mask with 1 at the bit position
                                              // 0 for other bits

    bitMap[byteIndex] = bitMap[byteIndex] | mask; // set the bit at the specified position to 1
}

// set the bit corresponding to blockNum to 0 (mark the block as free)
void setBitFree(unsigned char *bitMap, unsigned int blockNum)
{
    unsigned int byteIndex = blockNum / 8;
    unsigned int bitIndex = blockNum % 8;

    unsigned char mask = ~(1 << (7 - bitIndex)); // create a 1-byte mask with 0 at the bit position
                                                 //  1 for other bits

    bitMap[byteIndex] = bitMap[byteIndex] & mask; // set the bit at the specified position to 0
}

// Check if the bit corresponding to blockNum is used
// return value: 1 used  0 free
int isBitUsed(unsigned char *bitMap, unsigned int blockNum)
{
    unsigned int byteIndex = blockNum / 8;
    unsigned int bitIndex = blockNum % 8;

    unsigned char mask = 1 << (7 - bitIndex);

    return (bitMap[byteIndex] & mask) != 0;
}

// Find the first free block after blockNum
int getFreeBlockNum(unsigned char *bitMap, unsigned int blockNum)
{

    while (isBitUsed(bitMap, blockNum))
    {
        // if blockNum exceed the number of blocks in the volume
        // return -1 to indicate that all blocks are used
        if (blockNum > vcb->blockCount)
        {
            return -1;
        }

        blockNum++;
    }

    return blockNum;
}