# FUSE Filesystem with Verona Concurrency Model

This repository contains the code for a FUSE Filesystem that utilizes the Verona concurrency model. Project Verona is a research project by Microsoft Research and academic collaborators at Imperial College London. It explores language and runtime design for safe, scalable memory management, and compartmentalization. The prototype presented here focuses on the memory management aspects of the research.

## Prerequisites

Before running the application, you need to ensure that you have the necessary dependencies installed:

1. Install libfuse by following the instructions [here](https://github.com/se802/verona-rt/blob/102e3b37d4361bf08449c42b67a14cf14d9cacc7/docs/building.md).

## Configuration

Before building and running the filesystem, make sure to configure the path where you want to mount the filesystem. To do this, modify the source code in the following line:

1. [main.cpp line 22](https://github.com/se802/FuseFileSystemBoC/blob/a62a67be9356947cba7b7ccc803b6818f2a2131b/main.cpp#L22)

## Building and Running

To build and run the project, follow these steps:

1. Clone this project: `git clone git@github.com:se802/FuseFileSystemBoC.git`
2. Change directory to the project folder: `cd FuseFileSystemBoC`
3. Make the run script executable: `chmod +x run.sh`
4. Execute the run script to build and run the filesystem: `./run.sh`

The run script will handle the compilation and execution of the Verona FUSE Filesystem. The filesystem will be mounted at the path you configured earlier.
