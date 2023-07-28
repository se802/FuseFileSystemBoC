# File System Benchmarking with Verona Workers

This repository contains a Python script that benchmarks a file system's performance using Verona workers as concurrent execution units. The script runs a C++ program with different numbers of workers and measures Input/Output Operations Per Second (IOPS) and latency for different I/O operations. The results are visualized using Matplotlib to understand the file system's performance under various workloads.

## How to Run

### Prerequisites

Ensure you have the following dependencies installed:

- Python 3.x
- Matplotlib
- NumPy

### Installation

Clone this repository to your local machine:

```bash
git clone https://github.com/your_username/your_repository.git
cd your_repository

## Configuration

Before running the benchmarking script, ensure the Verona runtime environment is set up correctly with the necessary C++ program (FuseFileSystemBoC). The program should be compiled and ready to execute.

If needed, update the path to the FuseFileSystemBoC executable in the `run_fs()` function of `benchmark_filesystem.py`.

## Running the Benchmarking Script

To run the benchmarking script, execute the following command:

```bash
python benchmark_filesystem.py
```

The benchmarking script executes the C++ program with different worker counts and various I/O types (write, randwrite, read, randread). It collects IOPS and latency data for each case and generates two grouped bar charts as the output:

- **IOPS Comparison Chart**: Shows IOPS achieved for different I/O types and worker counts.
- **Latency Comparison Chart**: Displays average completion time (latency) for each I/O type and worker count.
