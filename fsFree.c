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
#include <stdio.h>
#include <stdlib.h>

#include "fsFree.h"
#include "fsLow.h"

unsigned char *bitMap;
int bitMapSizeBlocks;
int bitMapSizeBytes;
int bitMapSizeBits;

// use value provided in fsInit.c to initialize bitmap
int initFreeSpace(int blockCount, int bytesPerBlock)
{
    // printf("\n------ INSIDE initFreeSpace() ------\n");
    bitMapSizeBits = blockCount;
    // number of blocks = number of bits in bitmap
    // 1 byte = 8 bit, calculate the bytes needed for  ceiling round up
    unsigned int bytesBitmap = (blockCount + 8 - 1) / 8;

    // store the needed bytes of Bitmap in VCB, since malloc may give extra bytes
    bitMapSizeBytes = bytesBitmap;

    // calculate the blocks needed for bitmap
    unsigned int blocksBitmap = (bytesBitmap + bytesPerBlock - 1) / bytesPerBlock;
    bitMapSizeBlocks = blocksBitmap;

    // blocks used by VCB and bit map during initialization
    unsigned int blocksInitUsed = blocksBitmap + 1;

    int bitMapBytesMalloc = blocksBitmap * bytesPerBlock;
    // printf("bitMapSizeBytes: %d\n", bitMapSizeBytes);
    // printf("bitMapBytesMalloc: %d\n", bitMapBytesMalloc);
    bitMap = malloc(bitMapBytesMalloc);

    // Initialize bit map
    // mark all bits at each byte free (0000 0000)
    for (int i = 0; i < bitMapSizeBytes; i++)
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
    bitMapSizeBits = blockCount;
    // calculate the number of bytes needed for the bitmap
    unsigned int bytesBitmap = (blockCount + 8 - 1) / 8;
    bitMapSizeBytes = bytesBitmap;

    // calculate the number of blocks needed for the bitmap
    unsigned int blocksBitmap = (bytesBitmap + bytesPerBlock - 1) / bytesPerBlock;
    bitMapSizeBlocks = blocksBitmap;

    // allocate memory for the bitmap
    bitMap = malloc(blocksBitmap * bytesPerBlock);

    // read the bitmap from disk (from a specified block)
    int checkVal = LBAread(bitMap, blocksBitmap, 1);
    if (checkVal != blocksBitmap)
    {
        printf("\nLBAread() failed in loadFreeSpace()\n");
        return -1;
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

    return ((bitMap[byteIndex] & mask) != 0);
}

// Find the first free block
int getFreeBlockNum()
{
    // track the blockNum corresponding to bit in bitMap
    int blockNum = 0;

    // Iterate over each byte in bitmap
    for (int i = 0; i < bitMapSizeBytes; i++)
    {
        // all 8 bits in current byte are used 0xFF = 1111 1111
        // Increase blockNum, continue loop to check next byte
        if (bitMap[i] == 0xFF)
        {
            blockNum += 8;
        }
        // Not all bits in current byte are used, check for first free bit
        else
        {
            while (isBitUsed(blockNum) == 1)
            {
                blockNum++;
            }
            break;
        }
    }

    // the last byte may have extra bit
    // use the last blockNum to check if blockNum is over limit
    // Last blockNum is numberOfBlocks - 1
    if (blockNum > bitMapSizeBits - 1)
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

    int startBlockNum = getFreeBlockNum(bitMap);

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

        // write the updated bitmap to disk, and check error
        int checkVal = LBAwrite(bitMap, bitMapSizeBlocks, 1);

        if (checkVal != bitMapSizeBlocks)
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


extent *allocateBlocksExt(int required, int minPerExtent){

    if(required <= minPerExtent){
        int startBlockNum = allocBlocksCont(minPerExtent);
        if(startBlockNum == -1){
            printf("\nNot enough free blocks to allocate.\n");
            return NULL;
        }else{
            // only one extent is enough to hold the blocks info
            extent * ext = malloc(sizeof(extent));
            if(ext == NULL){
                printf("error: failed to allocate extent");
                return NULL;
            }
            ext->start = startBlockNum;
            ext->count = required;
        }
    }else{
        int extentNumber = required/minPerExtent + 1;

        // add last extent with start and count = 0 to indicate the end of extent table
       
        extent * ext = malloc((extentNumber+1) * sizeof(extent));
        if(ext == NULL){
            printf("error: failed to allocate extent");
            return NULL;
        }
        int startBlockNum;
        // for the first extentNumber-1 extent, count is minPerExtent
        for(int i = 0; i < extentNumber-1; i++){
            startBlockNum= allocBlocksCont(minPerExtent);
            if(startBlockNum == -1){
            printf("\nerror: Not enough free blocks to allocate.\n");
            return NULL;
            ext[i].start = startBlockNum;
            ext[i].count = minPerExtent;
        }
        // for the last extentNumberth extent, count is the remaining block
        startBlockNum= allocBlocksCont(minPerExtent);
        if(startBlockNum == -1){
            printf("\nNot enough free blocks to allocate.\n");
            return NULL;
        ext[extentNumber-1].start = startBlockNum;
        ext[extentNumber-1].count = required - minPerExtent * (extentNumber-1);
        
        // the last extent indicate the end of the array
        startBlockNum= allocBlocksCont(minPerExtent);
        if(startBlockNum == -1){
            printf("\nNot enough free blocks to allocate.\n");
            return NULL;
        ext[extentNumber].start = 0;
        ext[extentNumber].count = 0;
        }
        }
        return ext;
    }
    }
}


void releaseBlockInOneExt(int start, int count){
    for(int i = 0; i < count; i ++){
        setBitFree(start + i);
    }
}