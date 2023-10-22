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
void initBitmap(uint64_t numberOfBlocks, uint64_t blockSize)
{
    // number of blocks = number of bits in bitmap
    // 1 byte = 8 bit, calculate the bytes needed for bitmap, ceiling round up
    unsigned int bytesBitmap = (numberOfBlocks + 8 - 1) / 8;

    // calculate the blocks needed for bitmap
    unsigned int blocksBitmap = (bytesBitmap + blockSize - 1) / blockSize;

    // blocks used by VCB and bit map during initialization
    unsigned int blocksInitUsed = blocksBitmap + 1;

    bitMap = malloc(blocksBitmap * blockSize);

    // Initialize bit map, mark all bits as free
    for (int i = 0; i < sizeof(bitMap); i++)
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

    /*
   TESTING

   Example data 
   numberOfBlocks: 19531
   blockSize: 512

   bitMap[0] is "fc", which is 1111 1100 
   Block 0 is VCB, Block 1 - 5 is bitmap
   */
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

// Find the first free block after blockNum
int getFreeBlockNum(unsigned char *bitMap, unsigned int blockNum)
{
    unsigned int byteIndex = blockNum / 8;
    unsigned int bitIndex = blockNum % 8;

    unsigned char mask = ~(1 << (7 - bitIndex));

    while (bitMap[byteIndex] & mask) // keep searching for the first 0 bit
    {
        bitIndex++;
        if (bitIndex == 8)
        {
            bitIndex = 0;
            byteIndex++;
        }

        if (byteIndex * 8 >= blockNum) // check if we have searched through all blocks
        {
            return -1; // no free block found
        }
        mask = 0xFF; // reset the mask to 0xFF (all 1s) to search for any free bit in the next byte
    }
    return byteIndex * 8 + bitIndex; // calculate the block number of the free block
}