# FUSE Filesystem with Verona Concurrency Model

![Project Logo](path/to/logo.png)

This repository contains the code for a FUSE Filesystem that utilizes the Verona concurrency model. Project Verona is a research project by Microsoft Research and academic collaborators at Imperial College London. It explores language and runtime design for safe, scalable memory management, and compartmentalization. The prototype presented here focuses on the memory management aspects of the research.

## :rocket: Prerequisites

Before running the application, you need to ensure that you have the necessary dependencies installed:

1. Install libfuse by following the instructions [here](https://github.com/se802/verona-rt/blob/102e3b37d4361bf08449c42b67a14cf14d9cacc7/docs/building.md).

## :wrench: Configuration

Before building and running the filesystem, make sure to configure the path where you want to mount the filesystem. To do this, modify the source code in the following line:

1. [main.cpp line 22](https://github.com/se802/FuseFileSystemBoC/blob/a62a67be9356947cba7b7ccc803b6818f2a2131b/main.cpp#L22)

## :hammer_and_wrench: Building and Running

To build and run the project, follow these steps:

1. Clone this project: `git clone git@github.com:se802/FuseFileSystemBoC.git`
2. Change directory to the project folder: `cd FuseFileSystemBoC`
3. Make the run script executable: `chmod +x run.sh`
4. Execute the run script to build and run the filesystem: `./run.sh`

The run script will handle the compilation and execution of the Verona FUSE Filesystem. The filesystem will be mounted at the path you configured earlier.

## :muscle: Contributing

We welcome contributions to this project! If you want to contribute, please read our [Contribution Guidelines](CONTRIBUTING.md) for more details.

## :page_facing_up: License

This project is licensed under the [MIT License](LICENSE).

## :bulb: Acknowledgments

The Verona FUSE Filesystem is built on the Verona concurrency model and inspired by ideas from various languages such as Rust, Cyclone, and Pony. We would like to thank the Project Verona team at Microsoft Research and the academic collaborators at Imperial College London for their valuable research and contributions.

Please note that this project is still in its research phase and not intended for production use. Use it at your own risk.

## :movie_camera: Screenshots/GIFs

![Screenshot 1](path/to/screenshot1.png)
![Screenshot 2](path/to/screenshot2.gif)

## :memo: Examples and Use Cases

Provide some examples of how to use your FUSE Filesystem in different scenarios or showcase specific features with code snippets:

```c++
// Example code goes here
