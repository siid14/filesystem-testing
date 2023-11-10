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


//...take the path, call the parse path, get the parent, the last element, load that last element...
fdDir *fs_opendir(const char *pathname)
    {
    int result = parsePath(pathname, ppi);

    if (result == -2)
        {
        printf("\nError: file or path does not exist..\n");
        return NULL;
        } 
    else if (result == -1) 
        {
        printf("\nError: invalid input pathname\n");
        return NULL;
        }
    else 
        {
        if (ppi->parent[ppi->index].isDir == 0)
            {
            printf("\nError: %s is not a directory\n", ppi->lastElement);
            return NULL;
            }
        else
            {
            //load the last element (current directory) if not already loaded
            if (ppi->index == -1 || strcmp(ppi->parent[ppi->index].fileName, ppi->lastElement) != 0)
                {
                DE *temp;
                int loadResult = loadDir(&temp, &ppi->parent[ppi->index]);
                //if failed to load directory 
                if (loadResult == -1) {
                    return NULL;
                }

                ppi->parent = temp;
                ppi->index = findEntryInDir(ppi->parent, ppi->lastElement);
                //if failed to find directory
                if (ppi->index == -1) 
                    {
                    printf("\nError: Unable to find %s in the directory\n", ppi->lastElement);
                    free(temp);
                    return NULL;
                    }
                }

            fdDir *dirp = malloc(sizeof(fdDir));
            if (dirp == NULL) 
                {
                printf("Error: malloc() failed in fs_opendir\n");
                return NULL;
                }

            dirp->d_reclen = 0;
            dirp->dirEntryPosition = 0;
            dirp->di = NULL;

            return dirp;
            }
        }
    }

//...when calling readdir, it's gonna give us the 1st entry (.) and the 2nd entry...
struct fs_diriteminfo *fs_readdir(fdDir *dirp)
    {
    //check if invalid directory or directory entry position
    if (dirp == NULL || dirp->directory == NULL) 
        {
        return NULL;
        }
    
    //check if the current position is within the valid range of directory entries
    if (dirp->dirEntryPosition < 0 || dirp->dirEntryPosition >= dirp->directory[0].size / sizeof(DE))
        {
        return NULL;
        }

    //check if we have reached the end of the directory
    if (dirp->dirEntryPosition >= dirp->directory[0].size / sizeof(DE))
        {
        return NULL;
        }

    struct fs_diriteminfo *di = (struct fs_diriteminfo *)malloc(sizeof(struct fs_diriteminfo));
    if (di == NULL)
        {
        printf("Error: malloc() failed for fs_diriteminfo\n");
        return NULL;
        }

    //handle special cases for "." and ".."
    if (dirp->dirEntryPosition == 0)
        {
        di->d_reclen = sizeof(struct fs_diriteminfo);
        di->fileType = FT_DIRECTORY; 
        strcpy(di->d_name, ".");

        dirp->dirEntryPosition++;
        return di;
        }
    else if (dirp->dirEntryPosition == 1)
        {
        di->d_reclen = sizeof(struct fs_diriteminfo);
        di->fileType = FT_DIRECTORY; 
        strcpy(di->d_name, "..");

        dirp->dirEntryPosition++;

        return di;
        }

    // Regular case for other directory entries
    int entries = dirp->directory[0].size / sizeof(DE);
    for (int i = dirp->dirEntryPosition; i < entries; i++)
        {
        if (isUsedEntry(&(dirp->directory[i])))
            {
            di->d_reclen = sizeof(struct fs_diriteminfo);
            di->fileType = (dirp->directory[i].isDir) ? FT_DIRECTORY : FT_REGFILE;
            strcpy(di->d_name, dirp->directory[i].fileName);
                
            dirp->dirEntryPosition = i + 1;
            return di;
            }
        }
    free(di);
    return (NULL);
    }

int isUsedEntry(const DE *entry)
    {
    return (entry->isDir || entry->fileName[0] != '\0');
    }
