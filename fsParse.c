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
 * The 'fsParse.c' file contains functions for parsing paths
 * and managing directory entries in the file system. The 'parsePath' 
 * function takes a path string, parses its components, and updates 
 * the path parsing information (ppInfo). It handles the process of 
 * identifying directories and elements within the path, validating 
 * the path's existence, and updating the ppInfo structure accordingly.
 * Additionally, the 'findEntryInDir' and 'loadDir' functions facilitate 
 * locating entries within a directory and loading directory information 
 * from disk, respectively. Furthermore, the 'loadRootDir' function is 
 * responsible for loading the root directory during system initialization.
 **************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fsParse.h"
#include "fsLow.h"

#define DEFAULT_DE_COUNT 50

// 0: valid path, -1: invalid path or ppi, -2: Dir not found, or parent is not a dir
// Take a path cstring and parse the info
int parsePath(const char *path, ppInfo *ppi)
{

    // update root dir first
    free(rootDir);
    rootDir = NULL;
    rootDir = loadRootDir(DEFAULT_DE_COUNT);

    // update cwd if it at root dir
    if (cwd[0].location == rootDir[0].location)
    {
        free(cwd);
        cwd = NULL;
        cwd = loadRootDir(DEFAULT_DE_COUNT);
    }


    char *mutablePath = strdup(path);
    if (mutablePath == NULL)
    {
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

    if (mutablePath == NULL || ppi == NULL)

    {
        printf("Error: invalid path or ppi in parsePath()");
        return (-1);
    }

    if (mutablePath[0] == '/')
    {
        startDir = rootDir;
    }
    else
    {
        startDir = cwd;
    }

   

    parent = startDir;
    token1 = strtok_r(mutablePath, "/", &savePtr);

    if (token1 == NULL)
    {
        if (strcmp(mutablePath, "/") == 0)
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
      
        token2 = strtok_r(NULL, "/", &savePtr);

        if (token2 == NULL)
        {

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

        DE *temp = loadDir(&parent[index]);

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

    }
}

int findEntryInDir(DE *parent, char *token)
{
   
    //  get the total number of DE in the directory
    int numberOfDE = parent[0].size / sizeof(DE);
  

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

DE *loadDir(DE *parent)
{

    // load new directory from disk into temp directory
    int blockCount = (parent->size + vcb->blockSize - 1) / vcb->blockSize;
    DE *tempDir = (DE *)malloc(blockCount * vcb->blockSize);
    if (tempDir == NULL)
    {
        printf("Error: malloc() failed loadDir()\n");
        return NULL;
    }

    int ret = LBAread(tempDir, blockCount, parent->location);

    if (ret != blockCount)
    {
        printf("Error: LBAread() returned %d in loadDir()\n", ret);
        return NULL;
    }

    return tempDir;
}

// Load root directory, return -1 if failed or 1 if success
DE *loadRootDir(int initialDirEntries)
{

    int bytesNeeded = sizeof(DE) * initialDirEntries;
 
    int blocksNeeded = (bytesNeeded + (vcb->blockSize - 1)) / vcb->blockSize;

    int bytesMalloc = blocksNeeded * vcb->blockSize;
   

    DE *tempDir = (DE *)malloc(bytesMalloc);
    if (tempDir == NULL)
    {
        printf("Error: malloc() failed loadRootDir()\n");
        return NULL;
    }

    int ret = LBAread(tempDir, blocksNeeded, vcb->rootDirLocation);

    if (ret != blocksNeeded)
    {
        printf("Error: LBAread() returned %d in loadRootDir()\n", ret);
        return NULL;
    }

    return tempDir;
}