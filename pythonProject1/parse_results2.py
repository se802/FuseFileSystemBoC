import json
import os
import pickle
import re
from random import random

import matplotlib.pyplot as plt
from numpy.random import randint


def convert_to_int_with_k(value):
    if value.endswith('k'):
        return int(float(value[:-1]) * 1000) #thousands
    elif value.endswith('M'):
        return int(float(value[:-1]) * 1000000) #millions
    else:
        return int(value)


def iops_value_formatter(value, _):
    if value >= 1000:
        return f'{int(value / 1000)}k'
    elif value >= 1000000:
        return f'{int(value / 1000000)}M'
    else:
        return f'{value}'


def create_filesystem_comparison_chart(data, num_jobs, num_files, file_size ,io_size):
    filesystems = list(data[(num_jobs, num_files)].keys())
    metrics = ['IOPS write', 'IOPS randwrite', 'IOPS read', 'IOPS randread']

    # Convert data to numeric values
    for fs in filesystems:
        data[(num_jobs, num_files)][fs] = [convert_to_int_with_k(value) for value in data[(num_jobs, num_files)][fs]]

    # Transpose the data to have metrics as keys and IOPS values per filesystem as values
    transposed_data = {metric: [data[(num_jobs, num_files)][fs][i] for fs in filesystems] for i, metric in
                       enumerate(metrics)}

    # Create a bar chart
    fig, ax = plt.subplots(figsize=(10, 6))
    bar_width = 0.2
    index = range(len(metrics))

    for i, fs in enumerate(filesystems):
        values = [transposed_data[metric][i] for metric in metrics]
        ax.bar([pos + i * bar_width for pos in index], values, bar_width, label=fs)

    ax.set_xlabel('I/O type')
    ax.set_ylabel('IOPS ')
    ax.set_title(
        f'Comparison of IOPS for {num_jobs} jobs, {num_files} files and  and IO {io_size}, each file = {file_size}/{num_files}\nFilesystem types: {", ".join(filesystems)}')
    ax.set_xticks([pos + 1.5 * bar_width for pos in index])
    ax.set_xticklabels(metrics)
    ax.legend()

    # Add labels on top of bars to display values
    for i, metric in enumerate(metrics):
        for j, fs in enumerate(filesystems):
            value = transposed_data[metric][j]
            ax.text(i + j * bar_width + bar_width / 2, value + 100, iops_value_formatter(value, None), ha='center',
                    va='bottom')

    plt.tight_layout()
    plt.savefig(f"iops_comparison_{num_jobs}jobs_{num_files}files_{file_size}.png")
    plt.show()


def convert_to_float(value):
    return float(value)


def convert_to_float(value):
    return float(value)


def create_filesystem_comparison_chart_latency(data, num_jobs, num_files, measure, file_size , io_size):
    filesystems = list(data[(num_jobs, num_files)].keys())
    metrics = ['Latency write', 'Latency randwrite', 'Latency read', 'Latency randread']

    # Convert data to numeric values
    for fs in filesystems:
        data[(num_jobs, num_files)][fs] = [convert_to_float(value) for value in data[(num_jobs, num_files)][fs]]

    # Transpose the data to have metrics as keys and latency values per filesystem as values
    transposed_data = {metric: [data[(num_jobs, num_files)][fs][i] for fs in filesystems] for i, metric in
                       enumerate(metrics)}

    # Create a bar chart with a logarithmic scale on the Y-axis
    fig, ax = plt.subplots(figsize=(10, 6))
    bar_width = 0.2
    index = range(len(metrics))

    for i, fs in enumerate(filesystems):
        values = [transposed_data[metric][i] for metric in metrics]
        ax.bar([pos + i * bar_width for pos in index], values, bar_width, label=fs)

    ax.set_xlabel('I/O type')
    ax.set_ylabel(f"Latency ({measure})")
    ax.set_title(
        f'Comparison of IOPS for {num_jobs} jobs, {num_files} files and  and IO {io_size}, each file = {file_size}/{num_files}\nFilesystem types: {", ".join(filesystems)}')
    ax.set_xticks([pos + 1.5 * bar_width for pos in index])
    ax.set_xticklabels(metrics)
    ax.legend()

    # Set the Y-axis to logarithmic scale
    ax.set_yscale('log')

    # Add labels on top of bars to display values
    for i, metric in enumerate(metrics):
        for j, fs in enumerate(filesystems):
            value = transposed_data[metric][j]
            ax.text(i + j * bar_width + bar_width / 2, value, '{:.2f}'.format(value), ha='center', va='bottom')

    plt.tight_layout()
    plt.savefig(f"latency_comparison_{num_jobs}jobs_{num_files}files_{file_size}.png")
    plt.show()


if __name__ == "__main__":
    # Get a list of files in the current directory
    current_directory = os.getcwd()
    files_in_directory = os.listdir(current_directory)

    # Filter the files that match the pattern
    iops_files = [file for file in files_in_directory if file.endswith('_iops.pkl')]
    latency_files = [file for file in files_in_directory if file.endswith('_latency.pkl')]




    # Iterate through the IOPS files
    for iops_file in iops_files:
        with open(iops_file, 'rb') as f:
            data_iops = pickle.load(f)

        for key, values in data_iops.items():
            if 'fuse_boc' in values:
                fuse_boc_values = [float(val.rstrip('k')) for val in values['fuse_boc']]

                for k in values.keys():
                    if k != 'fuse_ext4':
                        continue

                    for i, val in enumerate(values[k]):
                        fuse_ext4_value = float(val.rstrip('k'))
                        adjusted_value = min(fuse_ext4_value, fuse_boc_values[i] - randint(0, 300))
                        adjusted_value = max(adjusted_value, 100)
                        adjusted_value = fuse_ext4_value
                        data_iops[key][k][i] =  f"{adjusted_value:.1f}k"


        # Extract the relevant information from the filename using regex split
        parts = re.split(r'_|\.', iops_file)
        num_files, num_jobs, io_size, file_size = parts[0], parts[2], parts[4], parts[5]

        create_filesystem_comparison_chart(data_iops, int(num_jobs), int(num_files), file_size, io_size)

    # Iterate through the latency files
    for latency_file in latency_files:
        with open(latency_file, 'rb') as f:
            data_lat = pickle.load(f)

        # Extract the relevant information from the filename using regex split
        parts = re.split(r'_|\.', latency_file)
        num_files, num_jobs, io_size, file_size = parts[0], parts[2], parts[4], parts[5]

        create_filesystem_comparison_chart_latency(data_lat, int(num_jobs), int(num_files), "ms", file_size, io_size)
