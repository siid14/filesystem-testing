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

#define DEFAULT_DE_COUNT 50

// DE *newDir;
//  int actualDirEntries;
//  int blocksNeeded;
//  int bytesMalloc;
// char *currentpath = "/";

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
    DE *newDir;
    newDir = malloc(bytesMalloc);
    if (newDir == NULL)
    {
        printf("Error: malloc() failed in fsDir.c\n");
        free(newDir);
        newDir = NULL;
        return -1;
    }

    int startBlock = allocBlocksCont(blocksNeeded);

    // check error
    if (startBlock == -1)
    {
        printf("Error: allocBlocksCont() failed in fsDir.c\n");
        free(newDir);
        newDir = NULL;
        return -1;
    }

    // mark every entry as unused
    for (int i = 0; i < actualDirEntries; i++)
    {
        strcpy(newDir[i].fileName, "\0");
        newDir[i].size = 0;
        newDir[i].location = 0;
        newDir[i].isDir = 0;
        newDir[i].timeCreated = 0;
        newDir[i].timeLastModified = 0;
        newDir[i].timeLastAccessed = 0;
    }

    // set the first directory entry points to itself
    strcpy(newDir[0].fileName, ".");
    newDir[0].size = actualDirEntries * sizeof(DE);
    newDir[0].location = startBlock;
    newDir[0].isDir = 1;

    time_t t = time(NULL);
    newDir[0].timeCreated = t;
    newDir[0].timeLastModified = t;
    newDir[0].timeLastAccessed = t;

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
        p = &newDir[0];
    }

    strcpy(newDir[1].fileName, "..");

    newDir[1].size = p->size;
    newDir[1].location = p->location;
    newDir[1].isDir = p->isDir;

    newDir[1].timeCreated = p->timeCreated;
    newDir[1].timeLastModified = p->timeLastModified;
    newDir[1].timeLastAccessed = p->timeLastAccessed;

    //////////// test parsePath() ////////////
    // strcpy(newDir[2].fileName, "dir1");
    // newDir[2].isDir = 1;
    // strcpy(newDir[3].fileName, "dir2");
    // newDir[3].isDir = 1;
    // strcpy(newDir[4].fileName, "dir3");
    // strcpy(newDir[5].fileName, "dir4");
    // strcpy(newDir[6].fileName, "dir5");
    //////////// test parsePath() ////////////

    int ret = LBAwrite(newDir, blocksNeeded, startBlock);
    // check error
    if (ret != blocksNeeded)
    {
        printf("Error: LBAwrite() returned %d in fsDir.c\n", ret);
        return -1;
    }

    free(newDir);
    newDir = NULL;
    return (startBlock);
}

int fs_setcwd(char *pathname)
{

    // printf("------setcwd function -----");
    // printf("path name is %s", pathname);
    int result = parsePath(pathname, ppi);
    // printf("result from parsePath is %d", result);
    if (result == -2)
    {
        printf("\nError: file or path does not exist..\n");
        return -2;
    }
    else if (result == -1)
    {
        printf("\nError: invalid input path\n");
        return -1;
    }
    else
    {
        // Case:  path is "/"
        if (ppi->lastElement == NULL)
        {
            free(cwd);
            cwd = NULL;
            cwd = loadRootDir(DEFAULT_DE_COUNT);
            strcpy(currentPath, "/");
            return 0;
        }

        if (ppi->parent[ppi->index].isDir == 0)
        {
            printf("\nError: %s is not a directory\n", ppi->lastElement);
            // printf("pathname is : %s", pathname);
            // printf("\nAfter parsePath()\n");
            // printf("Return value of parsePath: %d\n", result);
            // printf("Parent dir[0]: %s\n", ppi->parent[0].fileName);
            // printf("Parent dir[1]: %s\n", ppi->parent[1].fileName);
            // printf("Parent dir[2]: %s\n", ppi->parent[2].fileName);
            // printf("Parent dir[3]: %s\n", ppi->parent[3].fileName);
            // printf("Parent dir[4]: %s\n", ppi->parent[4].fileName);
            // printf("Index: %d\n", ppi->index);
            // printf("Last element: %s\n", ppi->lastElement);
            // printf("parent[index].isDir: %d\n", ppi->parent[ppi->index].isDir);
            return -1;
        }
        else
        {
            free(cwd);
            cwd = NULL;
            cwd = loadDir(&(ppi->parent[ppi->index]));

            if (pathname[0] == '/')
            {
                strcpy(currentPath, pathname);
                currentPath = cleanPath(currentPath);
                // printf("\n----- inside of setcwd function, the currentPath is %s", currentPath);
            }
            else
            {
                strcat(currentPath, "/");
                strcat(currentPath, pathname);
                currentPath = cleanPath(currentPath);
                // printf("\n----- inside of setcwd function, the currentPath is %s", currentPath);
            }
            return 0;
        }
    }
}

char *cleanPath(char *path)
{

    // printf(" ----in cleanPath function ----");

    char *tokens[100];

    // Tokenize the string
    char *token = strtok(path, "/");

    // the index of path tokens arrays
    int i = 0;

    // Loop through the tokens and store them in the array
    while (token != NULL)
    {
        if (strcmp(token, "..") == 0)
        {
            if (i == 0)
            {
                i = 0;
            }
            else
            {
                i--;
            }
        }
        else if (strcmp(token, ".") == 0)
        {
            i = i;
        }
        else
        {
            tokens[i] = token;
            i++;
        }

        token = strtok(NULL, "/");
    }

    // Print the tokens
    // printf("\nTokens:\n");
    // printf("\nnumber of i is %d:\n",i);

    if (i == 0)
    {
        // printf("----inside i == 0 ------");
        char *result = (char *)malloc(2);
        if (result != NULL)
        {

            // Set the content of the memory to "/"
            strcpy(result, "/");
            // printf("----inside i == 0 result is %s------",result);

            // Return the dynamically allocated string
            return result;
        }
    }
    // for (int j = 0; j < i; j++) {
    //     printf("\ntolens[%d]: %s\n", j,tokens[j]);
    // }

    size_t totalLength = 0;
    for (int j = 0; j < i; j++)
    {
        totalLength += strlen(tokens[j]);
    }

    // Add space for the separator '/' and null terminator
    totalLength += i - 1;

    // Create a buffer to store the concatenated string
    char *result = (char *)malloc(totalLength + 1);

    result[0] = '\0';

    // Concatenate the tokens with '/'
    // strcpy(result, "/");
    for (int j = 0; j < i; j++)
    {
        strcat(result, "/");
        // printf(" catenate / ");
        strcat(result, tokens[j]);
        // printf(" catenate %s ",tokens[j] );
    }

    // Print the result
    // printf("Concatenated string: %s\n", result);

    return result;
}

int fs_mkdir(const char *pathname, mode_t mode)
{

    // printf("---- inside fs_mkdir() ----\n");
    int result = parsePath(pathname, ppi);
    // printf("---- the result of parsePath is %d ----\n", result);
    // printf("---- the ppi index is %d ----\n", ppi->index);

    if (result == -2)
    {
        printf("\nfile or directory does not exist\n");
        return -2;
    }
    if (result == -1)
    {
        printf("\ninvalid path\n");
        return -1;
    }

    if (result == 0)
    {

        if (ppi->index == -1)
        {
            int newDirLocation = initDir(DEFAULT_DE_COUNT, ppi->parent, vcb->blockSize);

            DE *newDir = loadDirLocation(newDirLocation);

            int freeIndex = findFreeDE(ppi->parent);
            if (freeIndex == -1)
            {
                printf("error: no more space for new directory\n");
                return (-1);
            }

            copyDE(&ppi->parent[freeIndex], &newDir[0]);
            strcpy(ppi->parent[freeIndex].fileName, ppi->lastElement);
            writeDir(ppi->parent);
            // printf("\nsuccessful make dir\n");

            free(newDir);
            newDir = NULL;

            return 0;
        }
        else
        {
            // lastElement is already in the parent
            printf("\n%s is already exsited............\n", ppi->lastElement);
            return -1;
        }
    }
}

// this function finds the empty entry in the parent directory
int findFreeDE(DE *parent)
{
    int numberofDE = parent[0].size / sizeof(DE);
    for (int i = 0; i < numberofDE; i++)
    {
        if (strcmp(parent[i].fileName, "\0") == 0)
        {
            return i;
        }
    }
    return -1;
}

// this function copis
void copyDE(DE *target, DE *resource)
{

    target->location = resource->location;
    target->size = resource->size;
    target->isDir = resource->isDir;
    target->timeCreated = resource->timeCreated;
    target->timeLastAccessed = resource->timeLastAccessed;
    target->timeLastModified = resource->timeLastModified;
}
// returns 1 if directory, 0 otherwise
int fs_isDir(char *pathname)
{
    int result = parsePath(pathname, ppi);
    if (result == -2)
    {
        printf("\nError: file or path does not exist..\n");
        return -2;
    }
    else if (result == -1)
    {
        printf("\nError: invalid input path\n");
        return -1;
    }
    else
    {
        // Case: path is "/"
        if (ppi->lastElement == NULL)
        {
            return 1;
        }

        if (ppi->parent[ppi->index].isDir == 0)
        {
            // printf("\nThis file is not a directory\n");
            return 0;
        }
        else
        {
            // printf("\nThis file is a directory\n");
            return 1;
        }
    }
}

// return 1 if file, 0 otherwise
int fs_isFile(char *filename)
{
    int result = parsePath(filename, ppi);
    if (result == -2)
    {
        printf("\nError: file or path does not exist..\n");
        return -2;
    }
    else if (result == -1)
    {
        printf("\nError: invalid input path\n");
        return -1;
    }
    else
    {
        // Case: path is "/"
        if (ppi->lastElement == NULL)
        {
            return 0;
        }

        if (ppi->parent[ppi->index].isDir == 0)
        {
            // printf("\nThis file is a file\n");
            return 1;
        }
        else
        {
            // printf("\nThis file is not a file\n");
            return 0;
        }
    }
}

// this function is used to close a directory after it has been read
int fs_closedir(fdDir *dirp)
{
    // printf("\n\n-------- START fs_closedir() --------\n");

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
    // printf("Directory pointer set up to NULL -- Directory closed successfully\n");
    // printf("------ END fs_closedir() -----\n");
    return 0;
}

int fs_stat(const char *path, struct fs_stat *buf)
{
    // printf("\n\n-------- START fs_stat() --------\n");

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
    // including its parent directory (`ppi->parent`) and its position within the parent directory (`ppi->index`)
    // we use this information to access the specific directory entry (DE) that corresponds to the
    // specified file or directory within its parent directory
    DE *entry = ppi->parent + ppi->index;

    // printf("File/Directory Name: %s\n", entry->fileName);
    // printf("Size: %lu bytes\n", entry->size);
    // printf("Block Size: %u bytes\n", vcb->blockSize);
    // printf("Blocks Allocated: %lu\n", entry->size / 512); // number of 512B blocks allocated
    // printf("Last Access Time: %s", ctime(&entry->timeLastAccessed));
    // printf("Last Modification Time: %s", ctime(&entry->timeLastModified));
    // printf("Creation Time: %s", ctime(&entry->timeCreated));

    // fill in the attributes in the `buf` structure
    buf->st_size = entry->size;                   // total size, in bytes
    buf->st_blksize = vcb->blockSize;             // blocksize for file system
    buf->st_blocks = entry->size / 512;           // number of 512B blocks allocated
    buf->st_accesstime = entry->timeLastAccessed; // time of last access
    buf->st_modtime = entry->timeLastModified;    // time of last modification
    buf->st_createtime = entry->timeCreated;      // time of last status change

    // printf("\n\n-------- END fs_stat() --------\n");

    return 0;
}

char *fs_getcwd(char *pathname, size_t size)
{
    // length of currentPath exceeds size
    if (strlen(currentPath) > size)
    {
        return NULL;
    }

    strncpy(pathname, currentPath, size);

    return pathname;
}

/*  functions needed by cmd_rm   */
// return 0: success, 1 fail
int fs_rmdir(const char *pathname)
{
    int validPath = parsePath(pathname, ppi);

    if (strcmp(ppi->lastElement, ".") == 0 || strcmp(ppi->lastElement, "..") == 0)
    {
        printf("Cannot remove . or ..\n");
        return 1;
    }

    if (validPath != 0 || ppi->index == -1 || ppi->parent[ppi->index].isDir != 1)
    {
        printf("Invalid directory to remove\n");
        return 1;
    }

    int checkVal = checkIfDirEmpty(&ppi->parent[ppi->index]);
    if (checkVal != 0)
    {
        printf("Only empty directory can be removed\n");
        return 1;
    }

    freeBlocksDE(&ppi->parent[ppi->index]);
    markUnusedDE(&ppi->parent[ppi->index]);
    writeDir(ppi->parent);
    return 0;
}

// return 0 success;  1 fail
int fs_delete(char *filename)
{
    int validPath = parsePath(filename, ppi);

    if (strcmp(ppi->lastElement, ".") == 0 || strcmp(ppi->lastElement, "..") == 0)
    {
        printf("Cannot remove . or ..\n");
        return 1;
    }

    if (validPath != 0 || ppi->index == -1 || ppi->parent[ppi->index].isDir != 0)
    {
        printf("Invalid file to remove\n");
        return 1;
    }

    freeBlocksDE(&ppi->parent[ppi->index]);
    markUnusedDE(&ppi->parent[ppi->index]);
    writeDir(ppi->parent);
    return 0;
}

/*  Helper functions  */
void freeBlocksDE(DE *IndexInParent)
{
    // If this function is called
    // then parsePath() already checked if the DE is valid

    // calculate blocks used
    int blockCount = (IndexInParent->size + vcb->blockSize - 1) / vcb->blockSize;

    for (int i = IndexInParent->location; i < blockCount; i++)
    {
        setBitFree(i);
    }
}

void markUnusedDE(DE *IndexInParent)
{
    // If this function is called
    // then parsePath() already checked if the DE is valid

    // Mark unused
    // DE name as "\0" and 0 for other values
    strcpy(IndexInParent->fileName, "\0");
    IndexInParent->size = 0;
    IndexInParent->location = 0;
    IndexInParent->isDir = 0;
    IndexInParent->timeCreated = 0;
    IndexInParent->timeLastModified = 0;
    IndexInParent->timeLastAccessed = 0;
}

int writeDir(DE *parent)
{
    // calculate blocks used by parent directory
    int blockCount = (parent->size + vcb->blockSize - 1) / vcb->blockSize;

    // write parent dir to disk
    int checkVal = LBAwrite(parent, blockCount, parent[0].location);

    if (checkVal != blockCount)
    {
        printf("\nError: LBAwrite() failed in writeDir(), fsDir.c\n");
        return 1;
    }

    return 0;
}

int checkIfDirEmpty(DE *IndexInParent)
{

    DE *temp;

    // load directory from disk into temp directory
    int blockCount = (IndexInParent->size + vcb->blockSize - 1) / vcb->blockSize;

    temp = malloc(blockCount * vcb->blockSize);
    if (temp == NULL)
    {
        printf("Error: malloc() failed in checkIfDirEmpty(),  fsDir.c \n");
        return -1;
    }

    int ret = LBAread(temp, blockCount, IndexInParent->location);

    if (ret != blockCount)
    {
        printf("Error: LBAread() returned %d in checkIfDirEmpty(),  fsDir.c \n", ret);
        free(temp);
        temp = NULL;
        return -1;
    }

    int entryCount = temp[0].size / sizeof(DE);
    int isEmpty = 0; // 0: empty;  1: not empty

    // start from 2, since index 0 is . and index 1 is ..
    // check if there is a used DE, and return 1, if not return 0
    for (int i = 2; i < entryCount; i++)
    {
        // Check if a DE is used
        if (strcmp(temp[i].fileName, "\0") != 0)
        {
            isEmpty = 1;
        }
    }

    free(temp);
    temp = NULL;
    return isEmpty;
}

// Load directory from disk using starting block num
DE *loadDirLocation(int startBlock)
{

    // printf("\n--- in loadRootDir() ---\n");

    int bytesNeeded = sizeof(DE) * DEFAULT_DE_COUNT;
    // printf("Size of one entry: %ld\n", sizeof(DE));
    // printf("Bytes needed for %d entris: %d\n", initialDirEntries, bytesNeeded);

    int blocksNeeded = (bytesNeeded + (vcb->blockSize - 1)) / vcb->blockSize;
    // printf("blocksNeeded in loadRootDir(): %d\n", blocksNeeded);

    int bytesMalloc = blocksNeeded * vcb->blockSize;
    // printf("Bytes malloc: %d\n", bytesMalloc);

    DE *tempDir = (DE *)malloc(bytesMalloc);
    if (tempDir == NULL)
    {
        printf("Error: malloc() failed loadDirLocation()\n");
        return NULL;
    }

    int ret = LBAread(tempDir, blocksNeeded, startBlock);

    // printf("block read: %d\n", ret);

    if (ret != blocksNeeded)
    {
        printf("Error: LBAread() returned %d in loadDirLocation()\n", ret);
        return NULL;
    }

    // printf("\n--- out loadRootDir() ---\n");
    return tempDir;
}

fdDir *fs_opendir(const char *pathname)
{

    // printf("\n\n----    In opendir()    ----\n");
    // printf("the path is: %s\n", pathname);
    int checkVal = parsePath(pathname, ppi);

    // Case: / is the path, load root dir
    if (ppi->lastElement == NULL)
    {
        // printf("inside / path\n");
        DE *temp = loadRootDir(DEFAULT_DE_COUNT);

        fdDir *fdd = malloc(sizeof(fdDir));

        fdd->directory = temp;
        fdd->dirEntryPosition = 0;
        fdd->d_reclen = sizeof(fdDir);

        return fdd;
    }

    // printf("return of parsePath(): %d\n", checkVal);

    // printf("\n\nUpdated parse path info\n");
    // printf("Last element: %s\n", ppi->lastElement);
    // printf("Index of last element in parent: %d\n", ppi->index);
    // printf("Parent dir[0]: %s\n", ppi->parent[0].fileName);
    // printf("Parent dir[1]: %s\n", ppi->parent[1].fileName);
    // printf("Parent dir[2]: %s\n", ppi->parent[2].fileName);
    // printf("Parent dir[3]: %s\n", ppi->parent[3].fileName);
    // printf("cwd dir[0]: %s\n", cwd[0].fileName);
    // printf("cwd dir[1]: %s\n", cwd[1].fileName);
    // printf("cwd dir[2]: %s\n", cwd[2].fileName);
    // printf("cwd dir[3]: %s\n", cwd[3].fileName);
    // printf("rootDir[0]: %s\n", rootDir[0].fileName);
    // printf("rootDir[1]: %s\n", rootDir[1].fileName);
    // printf("rootDir[2]: %s\n", rootDir[2].fileName);
    // printf("rootDir[3]: %s\n", rootDir[3].fileName);

    if (checkVal != 0 || ppi->index == -1 || ppi->parent[ppi->index].isDir != 1)
    {
        printf("Invalid directory path\n");
        return NULL;
    }

    DE *temp = loadDir(&ppi->parent[ppi->index]);

    if (temp == NULL)
    {
        return NULL;
    }

    fdDir *fdd = malloc(sizeof(fdDir));
    fdd->directory = temp;
    fdd->dirEntryPosition = 0;
    fdd->d_reclen = sizeof(fdDir);

    return fdd;
}

struct fs_diriteminfo *fs_readdir(fdDir *dirp)
{
    int entryCount = dirp->directory[0].size / sizeof(DE);

    struct fs_diriteminfo *di = (struct fs_diriteminfo *)malloc(sizeof(struct fs_diriteminfo));

    for (int i = dirp->dirEntryPosition; i < entryCount; i++)
    {
        // If the name of dir[i] is not \0, then it is used
        if (strcmp(dirp->directory[i].fileName, "\0") != 0)
        {
            di->d_reclen = sizeof(struct fs_diriteminfo);

            strcpy(di->d_name, dirp->directory[i].fileName);

            // Set type in di to File
            if (dirp->directory[i].isDir == 0)
            {
                di->fileType = FT_REGFILE;
            }
            else // Set type in di to Directory
            {
                di->fileType = FT_DIRECTORY;
            }

            dirp->dirEntryPosition = i + 1;
            return di;
        }
    }
    return NULL;
}

int fs_move(char *src, char *dest)
{
    // when this function is called, the types of src and dest are valid
    // src is a file, and dest is a directory

    parsePath(src, ppi);

    // Load the parent directory of src file
    DE *srcDir = loadDir(&ppi->parent[0]);
    int srcIndex = ppi->index;

    // Copy needed DE info from src
    char srcFilename[MAX_FILENAME_LEN + 1];
    strcpy(srcFilename, ppi->lastElement);
    long srcSize = ppi->parent[ppi->index].size;
    int srcLocation = ppi->parent[ppi->index].location;
    time_t srcTimeCreated = ppi->parent[ppi->index].timeCreated;
    time_t srcTimeAccessed = ppi->parent[ppi->index].timeLastAccessed;
    time_t srcTimeModified = ppi->parent[ppi->index].timeLastModified;

    int srcDirLocation = ppi->parent[0].location;

    parsePath(dest, ppi);
    DE *destDir;

    if (ppi->lastElement == NULL) // Case: path is "/"
    {
        destDir = loadRootDir(DEFAULT_DE_COUNT);
    }
    else
    {
        destDir = loadDir(&ppi->parent[ppi->index]);
    }

    int destDirLocation = ppi->parent[ppi->index].location;
    // printf("srcDirLocation: %d\n", srcDirLocation);
    // printf("destDirLocation: %d\n", destDirLocation);

    if (srcDirLocation == destDirLocation)
    {
        // do nothing, since src and dest are at same directory
        free(srcDir);
        srcDir = NULL;
        free(destDir);
        destDir = NULL;
        return 0;
    }

    int freeIndex = findFreeDE(destDir);

    if (freeIndex == -1)
    {
        printf("No free entries in %s", ppi->lastElement);
        free(srcDir);
        srcDir = NULL;
        free(destDir);
        destDir = NULL;
        return -1;
    }

    // At this point, it is okay to move src file to dest directory

    // Claer DE info at src
    markUnusedDE(&srcDir[srcIndex]);
    writeDir(srcDir);

    // Update DE info at dest
    strcpy(destDir[freeIndex].fileName, srcFilename);
    destDir[freeIndex].isDir = 0;
    destDir[freeIndex].location = srcLocation;
    destDir[freeIndex].size = srcSize;
    destDir[freeIndex].timeCreated = srcTimeCreated;
    destDir[freeIndex].timeLastAccessed = srcTimeAccessed;
    destDir[freeIndex].timeLastModified = srcTimeModified;
    writeDir(destDir);

    free(srcDir);
    srcDir = NULL;
    free(destDir);
    destDir = NULL;
    return 0;
}