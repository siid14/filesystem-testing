#include <stdio.h>
#include <stdlib.h>
#define LAST_BLOCKNUM 19530
unsigned char *bitMap;
void setBitUsed(unsigned char *bitMap, unsigned int blockNum);
void setBitFree(unsigned char *bitMap, unsigned int blockNum);
void initBitmap(int numberOfBlocks, int blockSize);
int isBitUsed(unsigned char *bitMap, unsigned int blockNum);
int getFreeBlockNum(unsigned char *bitMap, unsigned int blockNum);

int main()
{
    // using sample date provide by prof
    // 2442 bytes needed for bit map
    // Num of blocks is 19531, Block 0 - Block 19530
    initBitmap(19531, 512);

    // Testing
    // 1111 1100  Hex: fc
    printf("Init bitmap, the bits at bitMap[0] should be 1111 1100, in hex: %x\n", bitMap[0]);
    int freeBlockNum = getFreeBlockNum(bitMap, 2);
    printf("freeBlockNum: %d\n", freeBlockNum);
   

    // Binary: 1111 1111     Hex: ff
     setBitUsed(bitMap, 6);
    setBitUsed(bitMap, 7);
    printf("\nthe bits at bitMap[0] should be 1111 1111, in hex: %x\n", bitMap[0]);
    freeBlockNum = getFreeBlockNum(bitMap, 2);
    printf("freeBlockNum: %d\n", freeBlockNum);

    // Binary: 1000 0000   Hex: 80
    setBitUsed(bitMap, 8);
    printf("\nthe bits at bitMap[1] should be 1000 0000, in hex: %x\n", bitMap[1]);
    freeBlockNum = getFreeBlockNum(bitMap, 2);
    printf("freeBlockNum: %d\n", freeBlockNum);

    // Mark all bits as used, 0xFF = 1111 1111
    printf("\nMark all bits used\n");
    for (int i = 0; i < 2442; i++)
    {
        bitMap[i] = 0xFF;
    }

    freeBlockNum = getFreeBlockNum(bitMap, 2);
    printf("after all bit used, freeBlockNum should return -1: %d\n", freeBlockNum);

    setBitFree(bitMap, 19530);
    freeBlockNum = getFreeBlockNum(bitMap, 2);
    printf("\nthe bits at bitMap[2441] should be 1101 1111, in hex: %x\n", bitMap[2441]);
    printf("last bit(block 19530) free, freeBlockNum: %d\n", freeBlockNum);

    return 0;
}

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

void initBitmap(int numberOfBlocks, int blockSize)
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
    for (int i = 0; i < bytesBitmap; i++)
    {
        bitMap[i] = 0x00;
    }

    // Set the block 0 (VCB) and blocks occupied by bitmap as used

    for (int i = 0; i < blocksInitUsed; i++)
    {
        setBitUsed(bitMap, i);
    }
}

// return 1: used   0 free
int isBitUsed(unsigned char *bitMap, unsigned int blockNum)
{
    unsigned int byteIndex = blockNum / 8;
    unsigned int bitIndex = blockNum % 8;

    unsigned char mask = 1 << (7 - bitIndex);

    return (bitMap[byteIndex] & mask) != 0;
}

// get next free BLOCK starting from BLOCK blockNum
int getFreeBlockNum(unsigned char *bitMap, unsigned int blockNum)
{
    while (isBitUsed(bitMap, blockNum))
    {
        if (blockNum > LAST_BLOCKNUM)
        {
            return -1;
        }
        blockNum++;
    }

    return blockNum;
}