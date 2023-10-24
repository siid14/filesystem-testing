#include <stdio.h>
#include <stdlib.h>
#define LAST_BLOCKNUM 19530
unsigned char *bitMap;
void setBitUsed(unsigned int blockNum);
void setBitFree(unsigned int blockNum);
void initBitmap(int numberOfBlocks, int blockSize);
int isBitUsed(unsigned int blockNum);
int getFreeBlockNum();
int allocBlocksCont(int blocksNeeded);
// int loadFreeSpace(int blockCount, int bytesPerBlock);

// Any line with LBAread() or LBAwrite() should be commented out or deleted in test c file
// otherwise, the test c file will have compile errors
// Test those in fsInit.c or via Hexdump

int main()
{
    // using sample date provide by prof
    // 2442 bytes needed for bit map
    // Num of blocks is 19531, Block 0 - Block 19530
    int numberOfBlocks = 19531;
    int blockSize = 512;
    initBitmap(numberOfBlocks, blockSize);

    // testing loadFreeSpace
    // testLoadFreeSpace(numberOfBlocks, blockSize);

    // Testing
    // 1111 1100  Hex: fc
    printf("Init  the bits at bitMap[0] should be 1111 1100, in hex: %x\n", bitMap[0]);
    int freeBlockNum = getFreeBlockNum();
    printf("freeBlockNum: %d\n", freeBlockNum);

    // Binary: 1111 1111     Hex: ff
    setBitUsed(6);
    setBitUsed(7);
    printf("\nthe bits at bitMap[0] should be 1111 1111, in hex: %x\n", bitMap[0]);
    freeBlockNum = getFreeBlockNum();
    printf("freeBlockNum: %d\n", freeBlockNum);

    // Binary: 1000 0000   Hex: 80
    setBitUsed(8);
    printf("\nthe bits at bitMap[1] should be 1000 0000, in hex: %x\n", bitMap[1]);
    freeBlockNum = getFreeBlockNum();
    printf("freeBlockNum: %d\n", freeBlockNum);

    // Mark all bits as used, 0xFF = 1111 1111
    // Total bytes in bitmap: 2442  Total bits in bitmap: 2442 * 8 = 19536
    // Total Blocks: 19531
    // Extra bit at last byte of bitmap: 5, ignore 5 bits from right to left
    printf("\nMark all bits used\n");
    for (int i = 0; i < 2442; i++)
    {
        bitMap[i] = 0xFF;
    }

    freeBlockNum = getFreeBlockNum();
    printf("after all bit used, freeBlockNum should be -1: %d\n", freeBlockNum);

    setBitFree(19530);
    freeBlockNum = getFreeBlockNum();
    printf("\nLast bit(block 19530) free, the bits at bitMap[2441] should be 1101 1111, in hex: %x\n", bitMap[2441]);
    printf("freeBlockNum: %d\n", freeBlockNum);

    // Test allocBlocksCont()
    printf("\n\nTesting allocBlocksCont(), compare outputs with comments\n");
    for (int i = 0; i < 2442; i++)
    {
        bitMap[i] = 0x00;
    }

    setBitUsed(0);
    setBitUsed(1);
    // bitMap[0]    Binary: 1100 0000    Hex: c0

    int testAlloc = allocBlocksCont(3);
    // Before   bitMap[0]    Binary: 1100 0000    Hex: c0

    // Allocate 3 blocks
    // After    bitMap[0]    Binary: 1111 1000    Hex: f8     start block: 2
    printf("Hex at bitMap[0]: %x\n", bitMap[0]);
    printf("start block: %d\n", testAlloc);

    testAlloc = allocBlocksCont(6);
    // Before       bitMap[0]    Binary: 1111 1000    Hex: f8

    // Allocate 6 blocks
    // After        bitMap[0]    Binary: 1111 1111    Hex: ff     start block: 5
    // After        bitMap[1]    Binary: 1110 0000    Hex: e0
    printf("\nHex at bitMap[0]: %x\n", bitMap[0]);
    printf("Hex at bitMap[1]: %x\n", bitMap[1]);
    printf("start block: %d\n", testAlloc);

    printf("\n\n");
    return 0;
}

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

void initBitmap(int numberOfBlocks, int blockSize)
{
    // number of blocks = number of bits in bitmap
    // 1 byte = 8 bit, calculate the bytes needed for  ceiling round up
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
        setBitUsed(i);
    }
}

// return 1: used   0 free
int isBitUsed(unsigned int blockNum)
{
    unsigned int byteIndex = blockNum / 8;
    unsigned int bitIndex = blockNum % 8;

    unsigned char mask = 1 << (7 - bitIndex);

    return (bitMap[byteIndex] & mask) != 0;
}

// get next free BLOCK starting from BLOCK blockNum
int getFreeBlockNum()
{
    int blockNum = 0;
    for (int i = 0; i < 2442; i++)
    {
        if (bitMap[i] == 0xFF)
        {
            blockNum += 8;
        }
        else
        {
            while (isBitUsed(blockNum))
            {
                blockNum++;
            }
            break;
        }
    }

    if (blockNum > LAST_BLOCKNUM)
    {
        return -1;
    }

    return blockNum;
}

int allocBlocksCont(int blocksNeeded)
{
    int startBlockNum = getFreeBlockNum();
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
        // LBAwrite lines deleted
        // should be tested via hexdump

        return startBlockNum;
    }
    else
    {
        printf("\nNot enough free blocks to allocate contiguously\n");
        return -1;
    }
}

// int loadFreeSpace(int blockCount, int bytesPerBlock)
// {
//     // calculate the number of bytes needed for the bitmap
//     unsigned int bytesBitmap = (blockCount + 8 - 1) / 8;

//     // calculate the number of blocks needed for the bitmap
//     unsigned int blocksBitmap = (bytesBitmap + bytesPerBlock - 1) / bytesPerBlock;

//     // allocate memory for the bitmap
//     bitMap = malloc(blocksBitmap * bytesPerBlock);

//     // read the bitmap from disk (from a specified block)
//     LBAread( blocks 1);

//     return 1; // return 1 to indicate success or handle errors accordingly.
// }

// // function to test loadFreeSpace
// void testLoadFreeSpace(int numberOfBlocks, int blockSize)
// {
//     // simulate writing a bitmap to disk
//     // file system mechanism -  to save the bitmap to disk
//     // simulate it by marking some blocks as used and then loading it back
//     setBitUsed( 2);
//     setBitUsed( 3);

//     // simulate saving the bitmap to disk
//     // file system, - use LBA write
//     // updatethe bitmap to simulate saving it to disk
//     // LBAwrite( blocks 1);

//     // load the free space information from "disk" (simulated by the bitmap)
//     loadFreeSpace(numberOfBlocks, blockSize);

//     // test if the loaded information matches the previous state
//     int isUsed = isBitUsed( 2);
//     printf("Block 2 is used: %d\n", isUsed);

//     isUsed = isBitUsed( 3);
//     printf("Block 3 is used: %d\n", isUsed);
// }