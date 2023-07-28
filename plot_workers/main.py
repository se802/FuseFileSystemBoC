import json
import os
import subprocess
import time

import matplotlib.pyplot as plt
import numpy as np
from matplotlib import cm


def run_fs():
    FS = subprocess.Popen(["/home/sevag/CLionProjects/FuseFileSystemBoC/cmake-build-release/FuseFileSystemBoC"])
    return FS


def kill_fs(FS):
    FS.kill()


def plot_data(data):
    # Extract the list of worker counts from the 'data' dictionary dynamically
    num_workers_list = list(data.keys())

    io_types = ["write", "randwrite", "read", "randread"]
    num_bars = len(num_workers_list)

    # Create a dictionary to store IOPS values for each I/O type
    iops_by_io_type = {io_type: [data[num_workers][i] for num_workers in num_workers_list] for i, io_type in
                       enumerate(io_types)}

    # Generate a range of colors based on the number of workers using the 'tab10' colormap
    # Define a list of custom colors for 5 workers
    colors = ['red', 'green', 'blue', 'orange', 'purple']

    # Define the width for each bar in the grouped bar chart
    bar_width = 0.15

    # Create an array of indices for the x-axis tick positions
    index = np.arange(len(io_types))

    plt.figure(figsize=(12, 8))
    for i, num_workers in enumerate(num_workers_list):
        iops_values = [iops_by_io_type[io_type][i] for io_type in io_types]
        plt.bar(index + i * bar_width, iops_values, bar_width, label=f"{num_workers} Workers", color=colors[i])

    plt.xlabel('I/O Type')
    plt.ylabel('IOPS (Operations Per Second)')
    plt.title('IOPS vs. I/O Type for Different Numbers of Workers')
    plt.xticks(index + bar_width * (num_bars - 1) / 2, io_types)
    plt.legend()
    plt.grid(True)
    plt.savefig("fig.png")
    plt.show()


def run_cpp_program(num_workers, lat_dict, iops_dict):
    # Set the environmental variable
    os.environ['NUM_WORKERS'] = str(num_workers)
    for io_type in ["write", "randwrite", "read", "randread"]:
        FS = run_fs()

        # run fio
        # --output test --output-format=json
        command = f"fio workload_{io_type}.fio --output workload_{io_type}.json --output-format=json"
        # print(f"Command: {command}, FS: {filesystem}")
        subprocess.run(['/bin/bash', '-c', command])

        with open(f"workload_{io_type}.json", "r") as file:
            json_data = file.read()
        time.sleep(2)
        # Parse the JSON data
        data = json.loads(json_data)
        x = "write" if io_type == "write" or io_type == "randwrite" else "read"
        print(x)
        if x == "read":
            print("aa")
        iops = data["jobs"][0][x]["iops"]
        clat_ns_mean = data["jobs"][0][x]["clat_ns"]["mean"]

        if num_workers not in lat_dict:
            lat_dict[num_workers] = []

        if num_workers not in iops_dict:
            iops_dict[num_workers] = []

        iops_dict[num_workers].append(iops)
        lat_dict[num_workers].append(clat_ns_mean)

        kill_fs(FS)


def main():
    # Define the number of workers to test
    num_workers_list = [4, 8, 12, 16, 32]

    lat_dict = {}
    iops_dict = {}

    for num_workers in num_workers_list:
        run_cpp_program(num_workers, lat_dict, iops_dict)

    plot_data(iops_dict)
    plot_data(lat_dict)


if __name__ == "__main__":
    main()
