/**************************************************************
 * Class:  CSC-415-01 Fall 2023
 * Names: Sidney Thomas, Hoang-Anh Tran, Ruxue Jin, Yee-Tsing Yang
 * Student IDs: 918656419, 922617784, 923092817, 922359864
 * GitHub Name: siid14, htran31, RuxueJ, Y-Y1Q
 * Group Name: Alibaba
 * Project: Basic File System
 *
 * File: fsParse.c
 *
 * Description:
 *
 *
 *
 **************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fsParse.h"
#include "fsLow.h"

int parsePath(const char *path, ppInfo *ppi)
{

    printf("\n\n--------------   INSIDE parsePath() ---------------\n");
    printf("\npath: %s\n", path);

    char *mutablePath = strdup(path);
    if (mutablePath == NULL) {
        printf("error: failed to copy the path");
        return -1;
    }


    DE *startDir;
    DE *parent;
    DE *temp;
    char *token1;
    char *token2;
    char *savePtr;
    int index;

    if (path == NULL || ppi == NULL)
    {
        printf("Error: invalid path or ppi in parsePath()");
        return (-1);
    }

    if (path[0] == '/')
    {
        startDir = rootDir;
    }
    else
    {
        startDir = cwd;
    }

    printf("startDir[0]: %s\n", startDir[0].fileName);

    parent = startDir;
    token1 = strtok_r(mutablePath, "/", &savePtr);

    printf("parent[0]: %s\n", parent[0].fileName);
    printf("token1: %s\n", token1);

    free(mutablePath);
    mutablePath = NULL;

    if (token1 == NULL)
    {
        if (strcmp(path, "/") == 0)
        {

            ppi->parent = parent;
            ppi->index = -1;
            ppi->lastElement = NULL;
            return (0);
        }
        return (-1);
    }

    while (token1 != NULL)
    {
        index = findEntryInDir(parent, token1);
        printf("\n---- OUTSIDE findEntryDir() ----\n");

        token2 = strtok_r(NULL, "/", &savePtr);

        printf("\n\ntoken1: %s\n", token1);
        printf("index: %d\n", index);
        printf("token2: %s\n", token2);
        printf("parent[index].isDir: %d\n", parent[index].isDir);

      

        if (token2 == NULL)
        {
            printf("\nINSIDE if (token2 == NULL)\n");

            ppi->parent = parent;
            ppi->lastElement = strdup(token1);
            ppi->index = index;

            return (0);
        }

        if (index == -1)
        {
            printf("Cannot find [%s] in directory[%s]\n", token1, parent[0].fileName);

            return (-2);
        }

        if (parent[index].isDir != 1) // not a dir
        {
            printf("[%s] in Parent[%s] is not a directory", parent[index].fileName, parent[0].fileName);

            return (-2);
        }

        loadDir(&temp, &parent[index]);

        if (temp == NULL)
        {
            return (-1);
        }

        if (parent != startDir)
        {
            free(parent);
            parent = NULL;
        }

        parent = temp;
        token1 = token2;

        printf("parent[0]: %s\n", parent[0].fileName);
        printf("\n\ntoken1: %s\n", token1);
    }
}

int findEntryInDir(DE *parent, char *token)
{
    printf("\n---- INSIDE findEntryDir() ----\n");
    // get the total number of DE in the directory
    int numberOfDE = parent[0].size / sizeof(DE);
    printf("parent[0].size: %ld\n", parent[0].size);
    printf("numberOfDE: %d\n", numberOfDE);

    for (int i = 0; i < numberOfDE; i++)
    {
        if (strcmp(parent[i].fileName, token) == 0)
        {
            return i;
        }
    }

    // not found in the directory
    return (-1);
}

int loadDir(DE **temp, DE *parent)
{

    // load new directory from disk into temp directory
    int blockCount = (parent->size + vcb->blockSize - 1) / vcb->blockSize;
    *temp = (DE *)malloc(blockCount * vcb->blockSize);
    if (*temp == NULL)
    {
        printf("Error: malloc() failed loadDir()\n");
        return -1;
    }

    int ret = LBAread(*temp, blockCount, parent->location);
    
    if (ret != blockCount)
    {
        printf("Error: LBAread() returned %d in loadDir()\n", ret);
        return -1;
    }

    return 1;
}

// Load root directory, return -1 if failed or 1 if success
int loadRootDir(DE **rootDir, int initialDirEntries)
{

    //printf("\n--- in loadRootDir() ---\n");

    int bytesNeeded = sizeof(DE) * initialDirEntries;
    // printf("Size of one entry: %ld\n", sizeof(DE));
    // printf("Bytes needed for %d entris: %d\n", initialDirEntries, bytesNeeded);

    int blocksNeeded = (bytesNeeded + (vcb->blockSize - 1)) / vcb->blockSize;
    //printf("blocksNeeded in loadRootDir(): %d\n", blocksNeeded);

    int bytesMalloc = blocksNeeded * vcb->blockSize;
    //printf("Bytes malloc: %d\n", bytesMalloc);

    *rootDir = (DE *)malloc(bytesMalloc);

    int ret = LBAread(*rootDir, blocksNeeded, vcb->rootDirLocation);

    //printf("block read: %d\n", ret);

    if (ret != blocksNeeded)
    {
        printf("Error: LBAread() returned %d in loadRootDir()\n", ret);
        return -1;
    }

    //printf("\n--- out loadRootDir() ---\n");
    return 1;
}