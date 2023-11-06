/**************************************************************
 * Class:  CSC-415-01 Fall 2023
 * Names: Sidney Thomas, Hoang-Anh Tran, Ruxue Jin, Yee-Tsing Yang
 * Student IDs: 918656419, 922617784, 923092817, 922359864
 * GitHub Name: siid14, htran31, RuxueJ, Y-Y1Q
 * Group Name: Alibaba
 * Project: Basic File System
 *
 * File: fsParse.h
 *
 * Description:
 *
 *
 *
 **************************************************************/

#ifndef _FS_PARSE_H
#define _FS_PARSE_H
#include "mfs.h"

typedef struct ppInfo
{
    DE *parent;        // point to parent directory
    int index;         // index of lastElement, -1 if it does not exist
    char *lastElement; // point to name of last element
} ppInfo;

extern DE *rootDir; // root directory
extern DE *cwd;     // current working directory
extern ppInfo *ppi; // parse path info

// 0: valid path, -1: error, -2: file or path not found
// Take a path cstring and parse the info
int parsePath(char *path, ppInfo *ppi);

//// Helper function ////
// find and return the index of entry in the parent directory
// -1: file or path not found
int findEntryInDir(DE *parent, char *token);

// First parameter: temporary directory to keep in memory
// Second parameter: DE info of the directory in disk
// load specified directory from disk to a temporary directory in memory
// return -1 if failed or 1 if success
int loadDir(DE *dirInMemory, DE *dirInDisk);

// load root directory from disk to a temporary directory in memory
// return -1 if failed or 1 if success
int loadRootDir(DE *rootDir, int initialDirEntries);

#endif