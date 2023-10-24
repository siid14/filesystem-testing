/**************************************************************
 * Class:  CSC-415-01  Fall 2023
 * Names: Sidney Thomas, Hoang-Anh Tran, Ruxue Jin, Yee-Tsing Yang
 * Student IDs: 918656419, 922617784, 923092817, 922359864
 * GitHub Name: siid14, htran31, RuxueJ, Y-Y1Q
 * Group Name: Alibaba
 * Project: Basic File System
 *
 * File: fsDir.c
 *
 * Description:  init directory.
 *
 **************************************************************/

#include "fsDir.h"
#include "fsLow.h"
#include "mfs.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// this function init directory
// it returns the number of first block of the directory in the disk
// the first parameter is the number of directory entry in the directory
// the second parameter is the pointer to its parent directory entry.
// for root directory, the parent should be null.

int initDir(int initialDirEntries, DE *parent)
{

    // printf("\n--------- INSIDE THE initDir function ---------\n");

    // allocate memory

    int bytesNeeded = sizeof(DE) * initialDirEntries;

    int blocksNeeded = (bytesNeeded + (blockSize - 1)) / blockSize;

    bytesNeeded = blocksNeeded * blockSize;

    int actualDirEntries = blocksNeeded / sizeof(DE);

    DE *directory = malloc(bytesNeeded);
    int startBlock = allocBlock(blocksNeeded);
    // check error
    if (startBlock == -1)
    {
        printf("Error: allocBlock() failed...\n");
        free(directory);
        directory = NULL;
        return -1;
    }

    // mark every entry as unused
    for (int i = 0; i < actualDirEntries; i++)
    {
        directory[i].filename = '\0';
        directory[i].size = 0;
        directory[i].location = 0;
        directory[i].isDir = 0;
        directory[i].createTime = 0;
        directory[i].lastModTime = 0;
        directory[i].lastAccessTime = 0;
    }

    // set the first directory entry points to itself
    strcpy(directory[0].filename, ".");
    directory[0].size = actualDirEntries * sizeof(DE);
    directory[0].location = startBlock;
    directory[0].isDir = 1;

    time_t t = time();
    directory[0].timeCreated = t;
    directory[0].timeLastModified = t;
    directory[0].timeLastAccessed = t;

    // check if the directory is root directory
    // if it is root directory, set the second directory entry to itself
    // otherwise, set the second directory entru to its parent

    DE *p;
    if (parent != NULL)
    {
        p = parent;
    }
    else
    {
        p = &directory[0];
    }

    strcpy(directory[1].filename, "..");

    directory[1].size = p->size;
    directory[1].location = p->location;
    directory[1].isDir = p->isDir;

    directory[1].timeCreated = p->timeCreated;
    directory[1].timeLastModified = p->timeLastModified;
    directory[1].timeLastModified = p->timeLastModified;

    int ret = LBAwrite(directory, blocksNeeded, startBlock);
    // check error
    if (ret != numOfBlockNeeded)
    {
        printf("Error: LBAwrite() returned %d\n", ret);
        return -1;
    }
    free(directory);
    return (startBlock);
}
