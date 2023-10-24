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
    unsigned int bytesBitmap = (blockCount + 8 - 1) / 8;

    // store the needed bytes of Bitmap in VCB, since malloc may give extra bytes
    vcb->bitMapSizeBytes = bytesBitmap;

    // calculate the blocks needed for bitmap
    unsigned int blocksBitmap = (bytesBitmap + bytesPerBlock - 1) / bytesPerBlock;
    vcb->bitMapSizeBlocks = blocksBitmap;

    // blocks used by VCB and bit map during initialization
    unsigned int blocksInitUsed = blocksBitmap + 1;

    bitMap = malloc(blocksBitmap * bytesPerBlock);

    // Initialize bit map, mark all bits as free
    for (int i = 0; i < bytesBitmap; i++)
    {
        bitMap[i] = 0x00;
    }

    // Set the block 0 (VCB) and blocks occupied by bitmap as used
    for (int i = 0; i < blocksInitUsed; i++)
    {
        setBitUsed(i);
    }

    // write bitmap to disk
    int checkVal = LBAwrite(bitMap, blocksBitmap, 1);

    if (checkVal != blocksBitmap)
    {
        printf("\nLBAwrite() failed in initFreeSpace()\n");
        return -1;
    }

    // return 1, indicating bitMap starts at block 1
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
    int checkVal = LBAread(bitMap, blocksBitmap, 1);
    if (checkVal != blocksBitmap)
    {
        printf("\nLBAread() failed in loadFreeSpace()\n");
        return -1
    }

    return 1; // return 1 to indicate success
}

// set the bit corresponding to blockNum to 1 (mark the block as used)
void setBitUsed(unsigned int blockNum)
{
    unsigned int byteIndex = blockNum / 8; // calculate the byte index in the bitmap
    unsigned int bitIndex = blockNum % 8;  // calculate the bit index within the byte

    unsigned char mask = 1 << (7 - bitIndex); // create a 1-byte mask with 1 at the bit position
                                              // 0 for other bits

    bitMap[byteIndex] = bitMap[byteIndex] | mask; // set the bit at the specified position to 1
}

// set the bit corresponding to blockNum to 0 (mark the block as free)
void setBitFree(unsigned int blockNum)
{
    unsigned int byteIndex = blockNum / 8;
    unsigned int bitIndex = blockNum % 8;

    unsigned char mask = ~(1 << (7 - bitIndex)); // create a 1-byte mask with 0 at the bit position
                                                 //  1 for other bits

    bitMap[byteIndex] = bitMap[byteIndex] & mask; // set the bit at the specified position to 0
}

// Check if the bit corresponding to blockNum is used
// return value: 1 used  0 free
int isBitUsed(unsigned int blockNum)
{
    unsigned int byteIndex = blockNum / 8;
    unsigned int bitIndex = blockNum % 8;

    unsigned char mask = 1 << (7 - bitIndex);

    return (bitMap[byteIndex] & mask) != 0;
}

// Find the first free block
int getFreeBlockNum()
{
    // track the blockNum corresponding to bit in bitMap
    int blockNum = 0;

    // Iterate over each byte in bitmap
    for (int i = 0; i < vcb->bitMapSizeBytes; i++)
    {
        // all 8 bits in the byte are used
        // set blockNum to the first bit of next byte
        if (bitMap[i] == 0xFF)
        {
            blockNum += 8;
        }
        // Not all bits in the byte are used, check each bit
        else
        {
            while (isBitUsed(blockNum))
            {
                blockNum++;
            }
        }
    }

    // the last byte may have extra bit
    // use the last blockNum to check if blockNum is over limit
    // Last blockNum is numberOfBlocks - 1
    if (blockNum > vcb->numberOfBlocks - 1)
    {
        printf("\nAll blocks in use\n");
        return -1;
    }

    return blockNum;
}

// Take amount of blocks needed and allocate
// return the starting blockNum,
// return -1 if free blocks are not enough, or failed to write updated bitmap to disk
int allocBlocksCont(int blocksNeeded)
{
    int startBlockNum = getFreeBlockNum(1);
    int countBlocksCont = 0; // to track how many contiguous free blocks

    // Check if there are enough free blocks for cont. allocation
    for (int i = startBlockNum; i < startBlockNum + blocksNeeded; i++)
    {
        if (isBitUsed(i) == 0)
        {
            countBlocksCont++;
        }
    }

    // Enough contiguous free blocks
    if (countBlocksCont == blocksNeeded)
    {
        // Mark the bits corresponding to blockNum as used
        for (int i = startBlockNum; i < startBlockNum + blocksNeeded; i++)
        {
            setBitUsed(i);
        }

        // write the updated bitmap to disk
        int checkVal = LBAwrite(bitMap, vcb->bitMapSizeBlocks, 1);

        if (checkVal != vcb->bitMapSizeBlocks)
        {
            printf("\nLBAwrite() failed in allocBlocksCont()\n");
            return -1;
        }

        return startBlockNum;
    }
    else
    {
        printf("\nNot enough free blocks to allocate contiguously\n");
        return -1;
    }
}