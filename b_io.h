/**************************************************************
 * Class:  CSC-415-01  Fall 2023
 * Names: Sidney Thomas, Hoang-Anh Tran, Ruxue Jin, Yee-Tsing Yang
 * Student IDs: 918656419, 922617784, 923092817, 922359864
 * GitHub Name: siid14, htran31, RuxueJ, Y-Y1Q
 * Group Name: Alibaba
 * Project: Basic File System
 *
 * File: b_io.h
 *
 * Description: 
 * This header file defines functions for basic file system operations,
 * including file opening, reading, writing, seeking, and closing.
 * It introduces a file descriptor type 'b_io_fd' and functions
 * such as 'b_open', 'b_read', 'b_write', 'b_seek', and 'b_close'
 * for handling file I/O with specified file descriptors and operations.
 **************************************************************/

#ifndef _B_IO_H
#define _B_IO_H
#include <fcntl.h>

typedef int b_io_fd;

b_io_fd b_open(char *filename, int flags);
int b_read(b_io_fd fd, char *buffer, int count);
int b_write(b_io_fd fd, char *buffer, int count);
int b_seek(b_io_fd fd, off_t offset, int whence);
int b_close(b_io_fd fd);

#endif
