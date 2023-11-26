/**************************************************************
 * Class:  CSC-415-01  Fall 2023
 * Names: Sidney Thomas, Hoang-Anh Tran, Ruxue Jin, Yee-Tsing Yang
 * Student IDs: 918656419, 922617784, 923092817, 922359864
 * GitHub Name: siid14, htran31, RuxueJ, Y-Y1Q
 * Group Name: Alibaba
 * Project: Basic File System
 *
 * File: fsDir.h
 *
 * Description:  
 * This header defines functions and structures for directory operations
 * within the basic file system. It includes functionalities for initializing 
 * directories, managing directory entries, path manipulation, checking 
 * directory emptiness, and handling directory-related file system operations
 * such as freeing blocks and writing directories to disk.
 *
 **************************************************************/
#ifndef _FS_DIR_H
#define _FS_DIR_H
#include "fsFree.h"

extern char *currentPath; // the current working path updated by setcwd

// this function init directory
// it returns the number of first block of the directory in the disk
// the first parameter is the number of directory entry in the directory
// the second parameter is the pointer to its parent directory entry.
// the third parameter is the bytes per block
// for root directory, the parent should be null.
int initDir(int DEcount, DE *parent, int blockSize);

/*      Helper function       */

// Load directory from disk using starting block num
// return pointer to the loaded directory
DE *loadDirLocation(int startBlock);

// this function finds the empty entry in the parent directory
// return -1 if all in use, otherwise return the index of free DE
int findFreeDE(DE *parent);

// Copy DEs from resource to target
void copyDE(DE *target, DE *resource);

// Clean the path string for cwd
char *cleanPath(char *path);

// can be used by b_open() with TRUNC flag
// free blocks of the specified DE in the bitmap
void freeBlocksDE(DE *IndexInParent);

// mark the DE as unused in its parent directory
void markUnusedDE(DE *IndexInParent);

// write updated DEs of parent to disk
// return 0: success    1: fail
int writeDir(DE *parent);

// If DE is a directory, check if the the dir is empty
// the dir should only have . and ..  DEs
// return 0:  empty    1: not empty     -1: other error
int checkIfDirEmpty(DE *IndexInParent);

#endif