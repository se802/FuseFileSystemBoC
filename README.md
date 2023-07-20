# FUSE Filesystem with Verona Concurrency Model

This repository contains the code for a FUSE Filesystem that utilizes the Verona concurrency model. 
## Configuration

Before running the application, make sure to configure the path to mount the filesystem to a valid path on your machine. To do this, modify the source code in the following lines:

1. [main.cpp line 22](https://github.com/se802/FuseFileSystemBoC/blob/a62a67be9356947cba7b7ccc803b6818f2a2131b/main.cpp#L22)


## Building

To build the project, follow these steps:

1. Install libfuse by following the instructions [here](https://github.com/se802/verona-rt/blob/102e3b37d4361bf08449c42b67a14cf14d9cacc7/docs/building.md).
2. Clone this project and build it by following the instructions [here](https://github.com/microsoft/verona-rt/blob/main/docs/building.md).
