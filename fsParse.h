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
 * Description: The 'fsParse.h' header file contains essential structures and
 *              function prototypes related to path parsing, directory management,
 *              and file system initialization. It defines the 'ppInfo' structure,
 *              which encapsulates details about parsed path elements,
 *              including the parent directory, element index, and name.
 *              This header declares functions like 'parsePath' to parse a path string
 *              and update path parsing information, 'findEntryInDir' to locate entries within a directory,
 *              'loadDir' to load a specified directory from disk into memory,
 *              and 'loadRootDir' to load the root directory during system initialization.
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

// 0: valid path, -1: invalid path or ppi, -2: Dir not found, or parent is not a dir
// Take a path cstring and parse the info
int parsePath(const char *path, ppInfo *ppi);

// Helper function //
// find and return the index of entry in the parent directory
// -1: file or path not found
int findEntryInDir(DE *parent, char *token);

// First parameter: temporary directory to keep in memory
// Second parameter: DE info of the directory in disk
// load specified directory from disk to a temporary directory in memory
// return -1 if failed or 1 if success
DE *loadDir(DE *parent);

// load root directory from disk to a temporary directory in memory
// return -1 if failed or 1 if success
DE *loadRootDir(int initialDirEntries);

#endif