# FUSE Filesystem with Verona Concurrency Model

This repository contains the code for a FUSE Filesystem that utilizes the Verona concurrency model. Project Verona is a research project by Microsoft Research and academic collaborators at Imperial College London. It explores language and runtime design for safe, scalable memory management, and compartmentalization. The prototype presented here focuses on the memory management aspects of the research.

## :rocket: Prerequisites

Before running the application, you need to ensure that you have the necessary dependencies installed:

1. Install libfuse by following the instructions [here](https://github.com/se802/verona-rt/blob/102e3b37d4361bf08449c42b67a14cf14d9cacc7/docs/building.md).

## :wrench: Configuration

Before building and running the filesystem, make sure to configure the path where you want to mount the filesystem. To do this, modify the source code in the following line:

1. [main.cpp line 22](https://github.com/se802/FuseFileSystemBoC/blob/a62a67be9356947cba7b7ccc803b6818f2a2131b/main.cpp#L22)

Install the latest kernel to enjoy the latest FUSE optimizations: 

1. wget https://raw.githubusercontent.com/pimlie/ubuntu-mainline-kernel.sh/master/ubuntu-mainline-kernel.sh
2. sudo install ubuntu-mainline-kernel.sh /usr/local/bin/
3. sudo ubuntu-mainline-kernel.sh -c
4. sudo ubuntu-mainline-kernel.sh -i
5. sudo reboot   

## :hammer_and_wrench: Building and Running

To build and run the project, follow these steps:

1. Clone this project: `git clone git@github.com:se802/FuseFileSystemBoC.git`
2. Change directory to the project folder: `cd FuseFileSystemBoC`
3. Make the run script executable: `chmod +x run.sh`
4. Execute the run script to build and run the filesystem: `./run.sh`

The run script will handle the compilation and execution of the Verona FUSE Filesystem. The filesystem will be mounted at the path you configured earlier.

## :open_file_folder: File Descriptions

The project consists of the following files and directories:

- `main.cpp`: Performs initializations related to Verona-RT and starts the FUSE filesystem.

- `FileSystem.cpp` and `FileSystem.h`: Implement FUSE callbacks that handle filesystem operations such as reading, writing, creating files, and more.

- `Block.cpp` and `Block.h`: Define data structures and functions related to blocks in the filesystem. Blocks are used to efficiently store data.

- `DirInode.cpp` and `DirInode.h`: Contain data structures and functions related to directory inodes. Directory inodes represent directories in the filesystem and store information about their contents.

- `RegInode.cpp` and `RegInode.h`: Implement data structures and functions for regular file inodes. Regular file inodes represent individual files in the filesystem and store data blocks.

- `Inode.cpp` and `Inode.h`: Define the base class for representing inodes in the filesystem. Inodes store metadata about files and directories.

- `CMakeLists.txt`: Used by CMake to configure the build process and generate the Makefile.

- `my_fuse_loop.h`: This file contains custom definitions or helper functions related to FUSE.

- `README.md`: This markdown file contains documentation or information about the project.

- `run.sh`: Shell script used to run the Verona-RT filesystem.


## :muscle: Contributing

We welcome contributions to this project!

## :bulb: Acknowledgments

The Verona FUSE Filesystem is built on the Verona concurrency model and inspired by ideas from various languages such as Rust, Cyclone, and Pony. We would like to thank the Project Verona team at Microsoft Research and the academic collaborators at Imperial College London for their valuable research and contributions.

Please note that this project is still in its research phase and not intended for production use. Use it at your own risk.

## :blue_book: References

- [Project Verona Official Website](https://www.microsoft.com/research/project/project-verona/)
- [Project Verona GitHub Repository](https://github.com/microsoft/verona)
- [libfuse GitHub Repository](https://github.com/libfuse/libfuse)
