/**************************************************************
* Class:  CSC-415-01 Fall 2023
* Names: Sidney Thomas, Hoang-Anh Tran, Ruxue Jin, Yee-Tsing Yang
* Student IDs: 918656419, 922617784, 923092817, 922359864
* GitHub Name: siid14, htran31, RuxueJ, Y-Y1Q
* Group Name:Alibaba
* Project: Basic File System
*
* File: DE.h
*
* Description: struct of directory entry
*
* 
*
**************************************************************/

#include <time.h>
#define MAX_FILENAME_LEN 255 // maximum filename length

// Directory Entry structure
typedef struct
  {
  char fileName[MAX_FILENAME_LEN + 1]; // file name cstring, +1 for the NULL
  unsigned long size;    // size of the directory/file in bytes
  unsigned int location; // starting block number of the directory/file
  unsigned int isDir;    // flag indicating if this entry is a directory (1) or a file (0)

  time_t timeCreated;      // time when the file created
  time_t timeLastModified; // time when the file last modified
  time_t timeLastAccessed; // time when the file last accessed
  } DE;
