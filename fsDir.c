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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h> // for struct stat

#include "fsDir.h"
#include "fsParse.h"
#include "fsLow.h"
#include "mfs.h"

// this function init directory
// it returns the number of first block of the directory in the disk
// the first parameter is the number of directory entry in the directory
// the second parameter is the pointer to its parent directory entry.
// the third parameter is the bytes per block
// for root directory, the parent should be null.

int initDir(int initialDirEntries, DE *parent, int blockSize)
{

    // printf("\n------ INSIDE THE initDir function ------ \n");

    // allocate memory

    int bytesNeeded = sizeof(DE) * initialDirEntries;
    // printf("Size of one entry: %ld\n", sizeof(DE));
    // printf("Bytes needed for %d entris: %d\n", initialDirEntries, bytesNeeded);

    int blocksNeeded = (bytesNeeded + (blockSize - 1)) / blockSize;
    // printf("blocksNeeded in initDir(): %d\n", blocksNeeded);

    int bytesMalloc = blocksNeeded * blockSize;
    // printf("Bytes malloc: %d\n", bytesMalloc);

    int actualDirEntries = bytesMalloc / sizeof(DE);
    // printf("actualDirEntries: %d\n",actualDirEntries);

    DE *directory = malloc(bytesMalloc);
    if (directory == NULL)
    {
        printf("Error: malloc() failed in fsDir.c\n");
        free(directory);
        directory = NULL;
        return -1;
    }

    int startBlock = allocBlocksCont(blocksNeeded);

    // check error
    if (startBlock == -1)
    {
        printf("Error: allocBlocksCont() failed in fsDir.c\n");
        free(directory);
        directory = NULL;
        return -1;
    }

    // mark every entry as unused
    for (int i = 0; i < actualDirEntries; i++)
    {
        strcpy(directory[i].fileName, "\0");
        directory[i].size = 0;
        directory[i].location = 0;
        directory[i].isDir = 0;
        directory[i].timeCreated = 0;
        directory[i].timeLastModified = 0;
        directory[i].timeLastAccessed = 0;
    }

    // set the first directory entry points to itself
    strcpy(directory[0].fileName, ".");
    directory[0].size = actualDirEntries * sizeof(DE);
    directory[0].location = startBlock;
    directory[0].isDir = 1;

    time_t t = time(NULL);
    directory[0].timeCreated = t;
    directory[0].timeLastModified = t;
    directory[0].timeLastAccessed = t;

    // check if the directory is root directory
    // if it is root directory, set the first directory entry to itself
    // otherwise, set the second directory entry to its parent

    DE *p;
    if (parent != NULL)
    {
        p = parent;
    }
    else
    {
        p = &directory[0];
    }

    strcpy(directory[1].fileName, "..");

    directory[1].size = p->size;
    directory[1].location = p->location;
    directory[1].isDir = p->isDir;

    directory[1].timeCreated = p->timeCreated;
    directory[1].timeLastModified = p->timeLastModified;
    directory[1].timeLastAccessed = p->timeLastAccessed;

    //////////// test parsePath() ////////////
    strcpy(directory[2].fileName, "dir1");
    strcpy(directory[3].fileName, "dir2");
    strcpy(directory[4].fileName, "dir3");
    strcpy(directory[5].fileName, "dir4");
    strcpy(directory[6].fileName, "dir5");
    //////////// test parsePath() ////////////

    int ret = LBAwrite(directory, blocksNeeded, startBlock);
    // check error
    if (ret != blocksNeeded)
    {
        printf("Error: LBAwrite() returned %d in fsDir.c\n", ret);
        return -1;
    }
    free(directory);
    directory = NULL;
    return (startBlock);
}

// this function is used to close a directory after it has been read
int fs_closedir(fdDir *dirp)
{
    printf("\n\n-------- START fs_closedir() --------\n");

    if (dirp == NULL)
    {
        // directory pointer is not pointing to a valid memory location
        fprintf(stderr, "Directory pointer is already NULL: Cannot close a non-existent directory.\n");
        return -1;
    }

    // free the memory allocated for the directory
    free(dirp);
    // set the directory pointer to NULL to avoid pointer issues
    dirp = NULL;
    printf("Directory pointer set up to NULL -- Directory closed successfully\n");
    printf("------ END fs_closedir() -----\n");
    return 0;
}

int fs_stat(const char *path, ppInfo *ppi, struct fs_stat *buf)
{
    printf("\n\n-------- START fs_stat() --------\n");

    if (path == NULL || ppi == NULL || buf == NULL)
    {
        printf("Error: Invalid input parameters in fs_stat()\n");
        return -1;
    }

    //  extract information about the file or directory
    if (parsePath(path, ppi) == -1)
    {
        printf("Error: Unable to parse path in fs_stat()\n");
        return -1;
    }

    // `ppi` contains information about the specified file or directory
    DE *entry = ppi->parent + ppi->index;

    printf("File/Directory Name: %s\n", entry->fileName);
    printf("Size: %lu bytes\n", entry->size);
    printf("Block Size: %u bytes\n", vcb->blockSize);
    printf("Blocks Allocated: %lu\n", entry->size / 512); // number of 512B blocks allocated
    printf("Last Access Time: %s", ctime(&entry->timeLastAccessed));
    printf("Last Modification Time: %s", ctime(&entry->timeLastModified));
    printf("Creation Time: %s", ctime(&entry->timeCreated));

    // fill in the attributes in the `buf` structure
    buf->st_size = entry->size;                   // total size, in bytes
    buf->st_blksize = vcb->blockSize;             // blocksize for file system
    buf->st_blocks = entry->size / 512;           // number of 512B blocks allocated
    buf->st_accesstime = entry->timeLastAccessed; // time of last access
    buf->st_modtime = entry->timeLastModified;    // time of last modification
    buf->st_createtime = entry->timeCreated;      // time of last status change

    printf("\n\n-------- END fs_stat() --------\n");

    return 0;
}
