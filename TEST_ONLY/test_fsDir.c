#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define blockSize 512 // For TEST ONLY, can take as parameter or use the value from VCB

#define MAX_FILENAME_LEN 255 // maximum filename length

// Directory Entry structure
typedef struct
{
    char fileName[MAX_FILENAME_LEN + 1]; // file name cstring, +1 for the NULL
    unsigned long size;                  // size of the directory/file in bytes
    unsigned int location;               // starting block number of the directory/file
    unsigned int isDir;                  // flag indicating if this entry is a directory (1) or a file (0)

    time_t timeCreated;      // time when the file created
    time_t timeLastModified; // time when the file last modified
    time_t timeLastAccessed; // time when the file last accessed
} DE;

int initDir(int initialDirEntries, DE *parent);

// int main()
// {

//     DE *parent;

//     int retVal = initDir(5, parent);
//     printf("\n--------- OUTSIDE THE initDir function ---------\n");
//     printf("return value from initDir: %d\n", retVal);

//     return 0;
// }

int initDir(int initialDirEntries, DE *parent)
{

    printf("\n--------- INSIDE THE initDir function ---------\n");

    // allocate memory

    int bytesNeeded = sizeof(DE) * initialDirEntries;

    int blocksNeeded = (bytesNeeded + (blockSize - 1)) / blockSize;

    bytesNeeded = blocksNeeded * blockSize;

    int actualDirEntries = blocksNeeded / sizeof(DE);

    DE *directory = malloc(bytesNeeded);
    int startBlock = 2; // For TEST ONLY
    // check error
    if (startBlock == -1)
    {
        printf("Error: allocBlock() failed...\n");
        free(directory);
        directory = NULL;
        return -1;
    }

    // mark every entry as unused
    for (int i = 0; i < actualDirEntries; i++)
    {
        strcpy(directory[0].fileName, "\0");
        directory[i].size = 0;
        directory[i].location = 0;
        directory[i].isDir = 0;
        directory[i].timeCreated = 0;
        directory[i].timeLastModified = 0;
        directory[i].timeLastModified = 0;
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

    printf("\n");
    printf("Entry at directory[0]\n");
    printf("Filename: %s\n", directory[0].fileName);
    printf("Time created: %s\n", ctime(&directory[0].timeCreated));
    printf("Time Last Modified: %s\n", ctime(&directory[0].timeLastModified));
    printf("Time Last Accessed: %s\n", ctime(&directory[0].timeLastAccessed));
    printf("\n");

    // check if the directory is root directory
    // if it is root directory, set the second directory entry to itself
    // otherwise, set the second directory entru to its parent

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
    directory[1].timeLastModified = p->timeLastModified;

    printf("\n");
    printf("Entry at directory[1]\n");
    printf("Filename: %s\n", directory[1].fileName);
    printf("\n");

    //// LBAwrite will be tested via Hexdump

    // int ret = LBAwrite(directory, blocksNeeded, startBlock);
    // // check error
    // if (ret != numOfBlockNeeded)
    // {
    //     printf("Error: LBAwrite() returned %d\n", ret);
    //     return -1;
    // }

    free(directory);
    directory = NULL;

    return (startBlock);
}
