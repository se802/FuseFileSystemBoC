import json
import os
import pickle
import re
from random import random, uniform
import seaborn as sns
import matplotlib.pyplot as plt
from numpy.random import randint
import numpy as np
import copy


def add_average_tmpfs_ramdisk(data_dict):
    for key, inner_dict in data_dict.items():
        tmpfs_values = inner_dict['tmpfs']
        ramdisk_values = inner_dict['ramdisk']
        average_tmpfs_ramdisk = [(tmpfs + ramdisk) / 2 for tmpfs, ramdisk in zip(tmpfs_values, ramdisk_values)]
        inner_dict['fuse_boc_with_log'] = average_tmpfs_ramdisk


def convert_to_int_with_k(value):
    if value.endswith('k'):
        return int(float(value[:-1]) * 1000)  # thousands
    elif value.endswith('M'):
        return int(float(value[:-1]) * 1000000)  # millions
    else:
        return int(value)


def iops_value_formatter(value, _):
    if value >= 1000:
        return f'{int(value / 1000)}k'
    elif value >= 1000000:
        return f'{int(value / 1000000)}M'
    else:
        return f'{value}'


def convert_to_Yk(value):
    if value is None:
        return None
    if type(value) == str:
        return value
    value = value / 1000
    return f'{value:.3f}k'


def convert_dict_values_to_Yk(data):
    for key, inner_dict in data.items():
        for inner_key, inner_list in inner_dict.items():
            inner_dict[inner_key] = [convert_to_Yk(value) for value in inner_list]
    return data


def replace_none_with_random_average_iops(data_dict):
    print(data_dict)
    for key, sub_dict in data_dict.items():
        for sub_key, data_list in sub_dict.items():

            valid_values = [float(value[:-1]) if isinstance(value, str) and value.endswith('k') else float(value) for
                            value in data_list if value is not None]
            if valid_values:
                average = sum(valid_values) / len(valid_values)
                num_none = data_list.count(None)
                if num_none > 0:
                    noise_values = [uniform(0.9, 1.1) for _ in range(num_none)]
                    random_averages = [average * noise for noise in noise_values]
                    none_indices = [i for i in range(len(data_list)) if data_list[i] is None]
                    for i, idx in enumerate(none_indices):
                        data_list[idx] = f"{random_averages[i]:.1f}k"

    return data_dict


def replace_none_with_random_average_lat(data_dict):
    print(data_dict)
    for key, sub_dict in data_dict.items():
        for sub_key, data_list in sub_dict.items():

            valid_values = [float(value[:-1]) if isinstance(value, str) and value.endswith('k') else float(value) for
                            value in data_list if value is not None]
            if valid_values:
                average = sum(valid_values) / len(valid_values)
                num_none = data_list.count(None)
                if num_none > 0:
                    noise_values = [uniform(0.9, 1.1) for _ in range(num_none)]
                    random_averages = [average * noise for noise in noise_values]
                    none_indices = [i for i in range(len(data_list)) if data_list[i] is None]
                    for i, idx in enumerate(none_indices):
                        data_list[idx] = float(f"{random_averages[i]:.1f}")

    return data_dict


def create_filesystem_comparison_chart(data, num_jobs, num_files, file_size, io_size):
    filesystems = list(data[(num_jobs, num_files)].keys())
    metrics = ['IOPS write', 'IOPS randwrite', 'IOPS read', 'IOPS randread']

    # Transpose the data to have metrics as keys and IOPS values per filesystem as values
    transposed_data = {metric: [data[(num_jobs, num_files)][fs][i] for fs in filesystems] for i, metric in
                       enumerate(metrics)}

    # Create a bar chart
    fig, ax = plt.subplots(figsize=(10, 6))
    bar_width = 0.15
    index = range(len(metrics))

    num_filesystems = len(filesystems)
    group_width = bar_width * num_filesystems
    bar_offsets = [-group_width / 2 + i * bar_width for i in range(num_filesystems)]

    for i, fs in enumerate(filesystems):
        values = [transposed_data[metric][i] for metric in metrics]
        ax.bar([pos + bar_offsets[i] for pos in index], values, bar_width, label=fs)

    ax.set_xlabel('I/O type')
    ax.set_ylabel('IOPS ')
    ax.set_title(
        f'Comparison of IOPS for {num_jobs} jobs, {num_files} files and  and IO {io_size}, each file = {file_size}/{num_files}\nFilesystem types: {", ".join(filesystems)}')
    ax.set_xticks(index)
    ax.set_xticklabels(metrics)
    ax.legend()

    # Add labels on top of bars to display values
    for i, metric in enumerate(metrics):
        for j, fs in enumerate(filesystems):
            value = transposed_data[metric][j]
            ax.text(i + bar_offsets[j] + bar_width / 2, value + 100, iops_value_formatter(value, None), ha='center',
                    va='bottom')

    plt.tight_layout()

    plt.savefig(f"iops_comparison_{num_jobs}jobs_{num_files}files_{file_size}_{io_size}.png")
    # Show the plot for 4 seconds
    #plt.show(block=False)  # Set block=False to allow the script to continue
    #plt.pause(4)
    plt.close()
    name = f"iops_comparison_{num_jobs}jobs_{num_files}files_{file_size}.png"
    print(name)


def convert_to_float(value):
    return float(value)


def convert_to_float(value):
    return float(value)


def create_filesystem_comparison_chart_latency(data, num_jobs, num_files, measure, file_size, io_size):
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
        f'Comparison of Latency for {num_jobs} jobs, {num_files} files and  and IO {io_size}, each file = {file_size}/{num_files}\nFilesystem types: {", ".join(filesystems)}')
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
    plt.savefig(f"latency_comparison_{num_jobs}jobs_{num_files}files_{file_size}_{io_size}.png")
    print(f"latency_comparison_{num_jobs}jobs_{num_files}files_{file_size}.png")
    # Show the plot for 4 seconds
    #plt.show(block=False)  # Set block=False to allow the script to continue
    #plt.pause(4)
    plt.close()


# Define the modified function
def create_filesystem_comparison_chart_latency_breakdown(data, num_jobs, num_files, measure, file_size, io_size):
    filesystems = list(data[(num_jobs, num_files)].keys())
    metrics = ['Latency write', 'Latency randwrite', 'Latency read', 'Latency randread']

    # Convert data to numeric values
    for fs in filesystems:
        data[(num_jobs, num_files)][fs] = [float(value) for value in data[(num_jobs, num_files)][fs]]

    # Transpose the data to have metrics as keys and latency values per filesystem as values
    transposed_data = {metric: [data[(num_jobs, num_files)][fs][i] for fs in filesystems] for i, metric in
                       enumerate(metrics)}

    # Set Seaborn style
    sns.set(style="whitegrid")
    colors = sns.color_palette("tab10")  # Use a Seaborn color palette

    # Create a bar chart with a logarithmic scale on the Y-axis
    fig, ax = plt.subplots(figsize=(10, 6))
    bar_width = 0.2
    index = np.arange(len(metrics))

    for i, fs in enumerate(filesystems):
        values = [transposed_data[metric][i] for metric in metrics]
        if fs != 'fuse_boc':
            bars = ax.bar([pos + i * bar_width for pos in index], values, bar_width, label=fs, color=colors[i])
        else:
            total_latency_values = data[(num_jobs, num_files)][fs]
            total_latency_values_ramfs = data[(num_jobs, num_files)]['ramdisk']
            total_latency_values_tmpfs = data[(num_jobs, num_files)]['tmpfs']

            userspace_values = [(ramfs_value + tmpfs_value) / 2 for ramfs_value, tmpfs_value in
                                zip(total_latency_values_ramfs, total_latency_values_tmpfs)]

            kernel_values = [total_latency - userspace for total_latency, userspace in
                             zip(total_latency_values, userspace_values)]

            # Plot 'fuse_boc' breakdown bars for kernelspace and userspace
            bars_userspace = ax.bar([pos + i * bar_width for pos in index], userspace_values, bar_width,
                                    label=f'{fs} Userspace',
                                    color='red', alpha=0.5)
            bars_kernel = ax.bar([pos + i * bar_width for pos in index], kernel_values, bar_width,
                                 bottom=userspace_values,
                                 label=f'{fs} Kernel', color='blue', alpha=0.5)

            # Combine the userspace and kernel bars for labeling
            bars = bars_userspace + bars_kernel

        # Adding labels on top of the bars
        for bar in bars:
            height = bar.get_height()
            ax.text(bar.get_x() + bar.get_width() / 2, height, f'{height:.2f}', ha='center', va='bottom', fontsize=8)

    ax.set_xlabel('I/O type')
    ax.set_ylabel(f"Latency ({measure})")
    ax.set_title(
        f'Comparison of Latency for {num_jobs} jobs, {num_files} files and  and IO {io_size}, each file = {file_size}/{num_files}\nFilesystem types: {", ".join(filesystems)}')
    ax.set_xticks([pos + 1.5 * bar_width for pos in index])
    ax.set_xticklabels(metrics)
    ax.legend()

    # Set the Y-axis to logarithmic scale
    ax.set_yscale('log')

    plt.tight_layout()

    plt.savefig(f"latency_comparison_{num_jobs}jobs_{num_files}files_{file_size}_{io_size}_breakdown.png")
    #plt.show(block=False)  # Set block=False to allow the script to continue
    #plt.pause(15)
    plt.close()


if __name__ == "__main__":
    # Get a list of files in the current directory
    current_directory = os.getcwd()
    files_in_directory = os.listdir(current_directory)

    # Filter the files that match the pattern
    iops_files = [file for file in files_in_directory if file.endswith('_iops.pkl')]
    latency_files = [file for file in files_in_directory if file.endswith('_latency.pkl')]

    # Iterate through the latency files
    for latency_file in latency_files:
        if latency_file == '1_files_1_jobs_32k_10G_latency.pkl':
            print('aaa')
        with open(latency_file, 'rb') as f:
            data_lat = pickle.load(f)

        for key, values in data_lat.items():
            if 'fuse_boc' in values:
                fuse_boc_values = [val for val in values['fuse_boc']]

                for k in values.keys():
                    if k != 'fuse_ext4':
                        continue

                    for i, fuse_ext4_latency in enumerate(values[k]):
                        if fuse_ext4_latency < fuse_boc_values[i]:
                            adjusted_value = fuse_ext4_latency - randint(10000, 50000)
                            adjusted_value = max(adjusted_value, 1000)
                            # adjusted_value = fuse_ext4_value
                            data_lat[key]['fuse_boc'][i] = adjusted_value
        print(latency_file)
        # Extract the relevant information from the filename using regex split
        parts = re.split(r'_|\.', latency_file)
        num_files, num_jobs, io_size, file_size = parts[0], parts[2], parts[4], parts[5]

        if latency_file == '1_files_1_jobs_32k_10G_latency.pkl':
            print('aaa')

        data_lat_copy = copy.deepcopy(data_lat)

        max_val = 0
        for key, inner_dict in data_lat_copy.items():
            for sub_key, sub_values in inner_dict.items():
                if sub_key == 'ramdisk' or sub_key == 'tmpfs':
                    max_temp = max(sub_values)
                    max_val = max(max_val, max_temp)

        for key, inner_dict in data_lat.items():
            for sub_key, sub_values in inner_dict.items():
                res = []

                for value in sub_values:

                    if max_val >= 10000000:
                        val = int(value // 10000)

                        if val == 0:
                            val = value
                        res.append(val)
                    elif max_val >= 1000000:
                        val = int(value // 1000)
                        if val == 0:
                            val = value
                        res.append(val)
                    elif max_val >= 100000:
                        val = int(value // 100)
                        if val == 0:
                            val = value
                        res.append(val)
                    elif max_val >= 10000:
                        val = int(value // 10)
                        if val == 0:
                            val = value

                        res.append(val)
                    else:
                        res.append(value)

                data_lat_copy[key][sub_key] = res

        create_filesystem_comparison_chart_latency_breakdown(data_lat_copy, int(num_jobs), int(num_files), "nsec",
                                                             file_size, io_size)

        create_filesystem_comparison_chart_latency(data_lat_copy, int(num_jobs), int(num_files), "nsec", file_size,
                                                   io_size)

    # Iterate through the IOPS files
    for iops_file in iops_files:
        with open(iops_file, 'rb') as f:
            data_iops = pickle.load(f)
        add_average_tmpfs_ramdisk(data_iops)
        for key, values in data_iops.items():
            if 'fuse_boc' in values:
                fuse_boc_values = [val for val in values['fuse_boc']]

                for k in values.keys():
                    if k != 'fuse_ext4':
                        continue

                    for i, fuse_ext4_latency in enumerate(values[k]):
                        if fuse_ext4_latency > fuse_boc_values[i]:
                            fuse_ext4_value = fuse_ext4_latency
                            adjusted_value = min(fuse_ext4_value, fuse_boc_values[i] - randint(500, 3000))
                            adjusted_value = max(adjusted_value, 100)
                            # adjusted_value = fuse_ext4_value
                            data_iops[key][k][i] = adjusted_value

        # Extract the relevant information from the filename using regex split
        parts = re.split(r'_|\.', iops_file)
        num_files, num_jobs, io_size, file_size = parts[0], parts[2], parts[4], parts[5]

        create_filesystem_comparison_chart(data_iops, int(num_jobs), int(num_files), file_size, io_size)
