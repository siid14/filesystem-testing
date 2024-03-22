## File System Project

This project involves the development of a file system in C programming language. The file system is designed to meet specific requirements including volume formatting, directory management, file operations, and interfacing with low-level disk I/O functions.

Project Overview
The goal of this project is to implement a functional file system, which includes formatting the volume, managing free space, maintaining directory structure, and supporting file operations such as creation, reading, writing, and deletion. The project is divided into three phases:

Formatting the Volume: Involves initializing the volume structure, defining directory entry structure, and managing free space.

Directory Based Functions: Implementation of directory management functions like creating and deleting directories, listing directory contents, and navigating through directories.

File Operations: Implementing file-related functionalities including reading, writing, and deleting files, along with support for copying and moving files between the file system and the host system.

Project Components
Low-Level Disk I/O Functions
The project utilizes low-level LBA-based read and write functions (LBAread and LBAwrite) to interact with the underlying disk. These functions handle reading and writing data blocks on the disk, facilitating file system operations.

Directory Functions
Directory functions are implemented to manage directory structure and operations. These include creating directories (fs_mkdir), deleting directories (fs_rmdir), listing directory contents (fs_readdir), and navigating directories (fs_opendir).

File Operations
File-related functions are implemented to support file creation, reading, writing, and deletion. Additionally, functionalities for checking file types (fs_isFile and fs_isDir), obtaining file stats (fs_stat), and changing the current working directory (fs_setcwd) are provided.

Shell Program (fsshell.c)
A shell program (fsshell.c) is provided to demonstrate the functionality of the file system. It includes built-in commands for common file system operations such as listing files (ls), copying files (cp), moving files (mv), creating directories (md), and removing files or directories (rm).

Submission Requirements
The submission for this project includes:

All source files (.c and .h)
Modified Driver program (fsshell.c)
Makefile to build the entire program
PDF writeup on the project including GitHub link, file system description, encountered issues, details of the driver program, screenshots of implemented commands, and volume file (limited to 10MB).
Conclusion
This project provides a hands-on experience in designing and implementing a file system in C, covering various aspects including disk I/O, directory management, and file operations. The provided shell program serves as a testing environment for demonstrating the functionality of the implemented file system.

For further clarification or assistance, feel free to reach out.


User
delete submission requirements
ChatGPT
File System Project
This project involves the development of a file system in C programming language. The file system is designed to meet specific requirements including volume formatting, directory management, file operations, and interfacing with low-level disk I/O functions.

Project Overview
The goal of this project is to implement a functional file system, which includes formatting the volume, managing free space, maintaining directory structure, and supporting file operations such as creation, reading, writing, and deletion. The project is divided into three phases:

Formatting the Volume: Involves initializing the volume structure, defining directory entry structure, and managing free space.

Directory Based Functions: Implementation of directory management functions like creating and deleting directories, listing directory contents, and navigating through directories.

File Operations: Implementing file-related functionalities including reading, writing, and deleting files, along with support for copying and moving files between the file system and the host system.

Project Components
Low-Level Disk I/O Functions
The project utilizes low-level LBA-based read and write functions (LBAread and LBAwrite) to interact with the underlying disk. These functions handle reading and writing data blocks on the disk, facilitating file system operations.

Directory Functions
Directory functions are implemented to manage directory structure and operations. These include creating directories (fs_mkdir), deleting directories (fs_rmdir), listing directory contents (fs_readdir), and navigating directories (fs_opendir).

File Operations
File-related functions are implemented to support file creation, reading, writing, and deletion. Additionally, functionalities for checking file types (fs_isFile and fs_isDir), obtaining file stats (fs_stat), and changing the current working directory (fs_setcwd) are provided.

Shell Program (fsshell.c)
A shell program (fsshell.c) is provided to demonstrate the functionality of the file system. It includes built-in commands for common file system operations such as listing files (ls), copying files (cp), moving files (mv), creating directories (md), and removing files or directories (rm).

Conclusion
This project provides a hands-on experience in designing and implementing a file system in C, covering various aspects including disk I/O, directory management, and file operations. The provided shell program serves as a testing environment for demonstrating the functionality of the implemented file system.





