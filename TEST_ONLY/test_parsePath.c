#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define MAX_FILENAME_LEN 255 // maximum filename length

// TEST ONLY struct
typedef struct DE
{
    char fileName[MAX_FILENAME_LEN + 1]; // file name cstring, +1 for the NULL
    unsigned long size;                  // size of the directory/file in bytes
    unsigned int location;               // starting block number of the directory/file
    unsigned int isDir;                  // flag indicating if this entry is a directory (1) or a file (0)

} DE;

typedef struct ppInfo
{
    DE *parent;        // parent directory
    int index;         // index of lastElement, -1 if it does not exist
    char *lastElement; // name of last element
} ppInfo;

int parsePath(char *path, ppInfo *ppi);

// Helper function
int findEntryInDir(DE *parent, char *token); // -2: file or path not found,
void loadDir(DE *temp, DE *parent); // Not tested in this file


// Keep these in memory
DE *rootDir;
DE *cwd;

// may need change VCB to global in FS project to get blockSize
int blockSize = 512;

/*
rootDir     dir1
            dir2    foo
            dir3

*/

int main()
{
    ppInfo *ppi;
    ppi = malloc(sizeof(ppInfo));

    // Initialize TEST root directory
    rootDir = malloc(4 * sizeof(DE));
    strcpy(rootDir[0].fileName, "rootDir");
    rootDir[0].size = 4 * sizeof(DE);
    rootDir[0].isDir = 1;

    strcpy(rootDir[1].fileName, "dir1");
    rootDir[1].isDir = 1;

    strcpy(rootDir[2].fileName, "dir2");
    rootDir[2].isDir = 1;

    strcpy(rootDir[3].fileName, "dir3");
    rootDir[3].isDir = 1;

    // Initialize TEST current working directory
    cwd = malloc(2 * sizeof(DE));

    strcpy(cwd[0].fileName, "dir2");
    cwd[0].size = 2 * sizeof(DE);
    cwd[0].isDir = 1;
    printf("cwd[0]: %s\n", cwd->fileName);

    strcpy(cwd[1].fileName, "foo");
    cwd[1].isDir = 0;

    // test parse path, change path to test

    /*

    working path: 
    /dir1  
    /dir2   
    /dir3

    dir2/foo

    */
    
    char path[] = "/dir3";
    parsePath(path, ppi);

    printf("\n\n----   OUTSIDE parsePath() -----\n");

    // Add printf() to check value
    printf("\nAfter parsePath()\n");
    printf("Parent dir: %s\n", ppi->parent->fileName);
    printf("Index: %d\n", ppi->index);
    printf("Last element: %s\n", ppi->lastElement);

    free(rootDir);
    rootDir = NULL;
    free(cwd);
    cwd = NULL;
    free(ppi);
    ppi = NULL;
}

// pseudo code from 10-31
// need to modify
int parsePath(char *path, ppInfo *ppi)
{

    printf("\n\n----   INSIDE parsePath() -----\n");
    printf("\npath: %s\n", path);

    DE *startDir;
    DE *parent;
    char *token1;
    char *token2;
    char *savePtr;
    int index;

    if (path == NULL)
    {
        return (-1);
    }

    if (ppi == NULL)
    {
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
    token1 = strtok_r(path, "/", &savePtr);

    printf("parent[0]: %s\n", parent[0].fileName);
    printf("token1: %s\n", token1);

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
        token2 = strtok_r(NULL, "/", &savePtr);

        printf("\n\ntoken1: %s\n", token1);
        printf("index: %d\n", index);
        printf("token2: %s\n", token2);

        printf("\n\nIndex: %d\n", index);
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
            return (-2);
        }

        if (parent[index].isDir != 1) // not a dir
        {
            return (-2);
        }

        // for TEST only
        // use LBAread  in FS project

        DE *temp = &parent[index];

        // to be implemented in FS project
        // loadDir(temp, &parent[index]); 

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

    // get the total number of DE in the directory
    int numberOfDE = parent[0].size / sizeof(parent);

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

// Not for TEST
// to be implemented FS project
// use LBAread() in FS project
void loadDir(DE *temp, DE *parent)
{

    int blockCount = (parent->size + blockSize - 1) / blockSize; // may need to change VCB to global to get blockSize
    temp = malloc(blockCount * blockSize);
    LBAread(temp, blockCount, parent->location);
}
