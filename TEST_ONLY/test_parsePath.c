#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define MAX_FILENAME_LEN 255 // maximum filename length

typedef struct DE
{
    char fileName[MAX_FILENAME_LEN + 1]; // file name cstring, +1 for the NULL
    unsigned long size;                  // size of the directory/file in bytes
    unsigned int location;               // starting block number of the directory/file
    unsigned int isDir;                  // flag indicating if this entry is a directory (1) or a file (0)

    time_t timeCreated;      // time when the file created
    time_t timeLastModified; // time when the file last modified
    time_t timeLastAccessed; // time when the file last accessed
} DE;

typedef struct ppInfo
{
    DE *parent;        // parent directory
    int index;         // index of lastElement
    char *lastElement; // name of last element
} ppInfo;

int parsePath(char *path, ppInfo *ppi);

int main()
{
    ppInfo *ppi;
    
    char path[] = "/test1/test2/foo";
    parsePath(path, ppi);

    // Add printf() to check value
}


// pseudo code from 10-31
// need to modify
int parsePath(char *path, ppInfo *ppi)
{
    char *startPath;
    DE *parent;
    char *token1;
    char *token2;
    char *savePtr;
    int index;

    if (path == NULL)
    {
        return -1;
    }

    if (ppi = NULL)
    {
        return -1;
    }

    if (path[0] == '/')
    {
        startPath = rootDir; // 
    }
    else
    {
        startPath = cwd;
    }

    parent = startPath;
    token1 = strtok_r(path, "/", &savePtr);

    if (token1 == NULL)
    {
        if (strcmp([ path, "/" ]) == 0)
        {
            ppi->parent = parent;
            ppi->index = -1;
            ppi->lastElement = NULL;
            return 0;
        }
        return -1;
    }

    while (token1 != NULL)
    {
        index = FindEntryInDir(parent, token1);
        token2 = strtok_r(NULL, "/", &savePtr);

        if (token2 == NULL)
        {
            ppi->parent = parent;
            ppi->lastElement = strdup(token1);
            ppi->index = index;
            return 0;
        }

        if (index == -1)
        {
            return -2;
        }

        if(!isDirectory(&(parent[index])))
        {
            return -2;
        }

        DE * temp = loadDir(&(parent[index]));

        if (temp == NULL)
        {
            return -1;
        }

        if ( parent != startDir)
        {
            free(parent);
        }

        parent = temp;
        token1 = token2;
    }
}