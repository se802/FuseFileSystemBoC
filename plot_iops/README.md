# FIO Performance Measurement and Filesystem Comparison

This project contains two scripts that work together to generate graphs comparing the performance of different filesystems using the FIO benchmark tool. The first script generates FIO configurations, runs FIO tests on various filesystems, and saves the results as pickle files. The second script reads the generated pickle files, processes the data, and creates comparison charts.


## Introduction

The main purpose of this project is to analyze and compare the performance of different filesystems under varying configurations of I/O operations. The scripts leverage the FIO benchmarking tool to measure IOPS (Input/Output Operations Per Second) and latency for different filesystems and generate comparison charts for easy visualization.

## Installation

1. Install the required dependencies:

    ```bash
    # Install the necessary Python packages
    pip install psutil matplotlib
    ```

2. Ensure that FIO (Flexible I/O Tester) is installed on your system. You can download and install it from [https://fio.github.io/fio/](https://fio.github.io/fio/).

3. Clone this repository:

    ```bash
    git clone git@github.com:se802/FuseFileSystemBoC.git
    cd FuseFileSystemBoc/plotMachineIOPS_LATENCY
    ```

## Usage

1. Configure the settings in the first script `generate_fio_configs.py` to define the combinations of I/O operations, filesystems, and configurations you want to test.

2. Run the first script:

    ```bash
    python generate_fio_configs.py
    ```

    This script will generate FIO configurations, run FIO tests on different filesystems, and save the results as pickle files.

3. Once the first script completes, run the second script `generate_comparison_charts.py` to process the pickle files and create comparison charts:

    ```bash
    python generate_comparison_charts.py
    ```

4. The comparison charts will be saved in the same directory as PNG files.

## Configuration

- The `generate_fio_configs.py` script allows you to specify various configurations for the FIO tests, such as I/O operation type, number of jobs, number of files, file size, etc. Modify the configurations in the script according to your needs.

- The `generate_comparison_charts.py` script performs data processing and chart generation based on the generated pickle files. You can adjust visualization settings and data processing options in this script if required.

## Dependencies

The project relies on the following dependencies:

- Python 3.x
- psutil: [https://pypi.org/project/psutil/](https://pypi.org/project/psutil/)
- matplotlib: [https://pypi.org/project/matplotlib/](https://pypi.org/project/matplotlib/)

Make sure to install these packages before running the scripts.
