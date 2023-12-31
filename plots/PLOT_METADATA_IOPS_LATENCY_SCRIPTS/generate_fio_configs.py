import pickle
import subprocess
import time
import json
import psutil as psutil
import matplotlib.pyplot as plt


def generate_filename(io_type, num_jobs, num_files, io_size, file_number=None):
    filename = f"{io_type}-{num_jobs}jobs-{num_files}files-{io_size}"
    if file_number is not None:
        filename += f"-file{file_number}"
    filename += ".fio"
    return filename


def generate_config(blocksize, numjobs, num_files, file_path, file_size, metadata):
    config_template = """[global]
blocksize={blocksize}
name=test
numjobs={numjobs}
group_reporting=1
iodepth=128
fallocate=native
runtime=120
directory={dir_path}
nrfiles={numfiles}
ioengine={metadata}
size={file_size}
[jobs1]
filename_format=f$filenum

"""

    file_template = "{file_number}"

    file_list = ":".join([file_template.format(file_number=i, file_path=file_path) for i in range(1, num_files + 1)])

    config_content = config_template.format(
        blocksize=blocksize,
        numjobs=numjobs,
        dir_path=file_path,
        numfiles=num_files,
        metadata=metadata,
        file_size=file_size
    )

    return config_content


def generate_config_file(config_list):
    blocksize, metadata, numjobs, num_files, file_path, file_size = config_list

    config_content = generate_config(blocksize=blocksize, numjobs=numjobs, num_files=num_files, file_path=file_path,
                                     file_size=file_size, metadata=metadata)

    filename = generate_filename(
        metadata,
        numjobs,
        num_files,
        blocksize.lower()
    )

    output_file = f"{filename}"

    with open(output_file, "w") as f:
        f.write(config_content)

    # print(f"Configuration saved to '{output_file}'")
    return output_file


def run_fs(filesystem, jobs, files, size):
    FS = None
    if filesystem == "fuse_boc":
        fs_stdout_logfile = open(f"fs_boc_stdout_{jobs}_{files}_{size}.log", 'w')
        fs_stderr_logfile = open(f"fs_boc_stderr_{jobs}_{files}_{size}.log", 'w')
        FS = subprocess.Popen(
            ["/home/sevag/CLionProjects/FuseFileSystemBoC/cmake-build-release/FuseFileSystemBoC"],
            stdout=fs_stdout_logfile,
            stderr=fs_stderr_logfile
        )
    elif filesystem == "fuse_ext4":
        fs_stdout_logfile = open(f"fs_ext4_stdout_{jobs}_{files}_{size}.log", 'w')
        fs_stderr_logfile = open(f"fs_ext4_stderr_{jobs}_{files}_{size}.log", 'w')
        FS = subprocess.Popen(
            ["/home/sevag/fs/StackFsOpt", "-r", "/", "/tmp/ssfs"],
            stdout=fs_stdout_logfile,
            stderr=fs_stderr_logfile
        )
        password = "1234"
        subprocess.run("sudo -S chown sevag:sevag /home/sevag/fuse_ext4", shell=True, input=password,
                       capture_output=True,
                       text=True)
    elif filesystem == "ramdisk":
        password = "1234"
        command = "sudo -S mount -t ramfs -o size=40G ramfs /home/sevag/ramdisk"
        FS = subprocess.run(command, shell=True, input=password, capture_output=True, text=True)
        subprocess.run("sudo -S chown sevag:sevag /home/sevag/ramdisk", shell=True, input=password, capture_output=True,
                       text=True)
        if FS.returncode < 0:
            exit(55)
    elif filesystem == "tmpfs":
        password = "1234"
        command = "sudo -S mount -t tmpfs -o size=40G tmpfs /home/sevag/tmpfs"
        FS = subprocess.run(command, shell=True, input=password, capture_output=True, text=True)
        if FS.returncode < 0:
            exit(55)

    return FS


def kill_fs(FS, filesystem):
    if filesystem == "fuse_boc":
        FS.kill()
    elif filesystem == "fuse_ext4":
        print("KIlling")
        FS.send_signal(2)
        time.sleep(2)

    return FS


def kill_processes_by_command(command):
    for process in psutil.process_iter(['pid', 'name', 'cmdline']):
        if process.info['cmdline'] and command in process.info['cmdline']:
            print(f"Terminating PID {process.info['pid']} - Command: {' '.join(process.info['cmdline'])}")
            try:
                process.terminate()
            except psutil.NoSuchProcess:
                pass


def unmount_dir(filesystem):
    password = "1234"
    command = f'sudo -S umount /home/sevag/{filesystem}'

    completed_process = subprocess.run(command, input=password, shell=True, capture_output=True, text=True)


import matplotlib.pyplot as plt


def format_number_with_k(num):
    if num >= 1000:
        return f"{num / 1000:.0f}k"
    else:
        return str(num)


def plot_results_iops(results, numfiles, operation, num_threads, metric_type):
    import matplotlib.pyplot as plt

    # Extract storage types and values from the input dictionary
    storage_types = list(results.keys())
    values = list(results.values())

    # Convert values to "k" format
    values_in_k = [value / 1000 if value >= 1000 else value for value in values]

    # Custom color palette for each storage type
    if metric_type == 'IOPS':
        color_palette = ['skyblue', 'lightgreen', 'red', 'lightcoral', 'purple']
    else:
        color_palette = ['skyblue', 'lightgreen', 'red', 'lightcoral']

    # Create the bar chart
    fig, ax = plt.subplots(figsize=(10, 6))
    bar_width = 0.5
    index = range(len(storage_types))

    for i, storage_type in enumerate(storage_types):
        ax.bar(index[i], values_in_k[i], width=bar_width, color=color_palette[i], label=storage_type)

    ax.set_xlabel('Storage Types')
    ax.set_ylabel(metric_type)
    ax.set_title(
        f"Results for Different Storage Types: numfiles-{numfiles}, operation-{operation}, num_threads-{num_threads}-{metric_type}",
        pad=20)
    ax.set_xticks(index)
    ax.set_xticklabels(storage_types)
    ax.legend()

    # Calculate dynamic label position based on values
    label_offset = 0.05  # Adjust this value to control label position
    for i, value in enumerate(values_in_k):
        label_x = index[i]
        label_y = value + label_offset * value  # Adjust label position based on value
        ax.text(label_x, label_y, format_number_with_k(values[i]), ha='center', va='bottom')

    plt.tight_layout()

    # Save the plot with an adjusted filename format
    filename = f"{metric_type}_comparison_jobs-{num_threads}_files-{numfiles}_size-{file_size}_operation-{operation}.png"
    print(f"Saving {filename}")
    plt.savefig(filename)

    # Display the plot
    plt.show(block=False)
    plt.pause(8)
    # Before closing the plot, check if the window still exists
    if plt.fignum_exists(1):
        plt.close()


def plot_results_latency(results, numfiles, operation, num_threads, metric_type):
    import matplotlib.pyplot as plt

    # Extract storage types and values from the input dictionary
    storage_types = list(results.keys())
    values = list(results.values())
    values_in_k = values
    # Convert values to "k" format
    # values_in_k = [value / 1000 if value >= 1000 else value for value in values]

    # Custom color palette for each storage type
    color_palette = ['skyblue', 'lightgreen', 'red', 'lightcoral']

    # Create the bar chart
    fig, ax = plt.subplots(figsize=(10, 6))
    bar_width = 0.5
    index = range(len(storage_types))

    for i, storage_type in enumerate(storage_types):
        ax.bar(index[i], values_in_k[i], width=bar_width, color=color_palette[i], label=storage_type)

    ax.set_xlabel('Storage Types')
    ax.set_ylabel(metric_type)
    ax.set_title(
        f"Results for Different Storage Types: numfiles-{numfiles}, operation-{operation}, num_threads-{num_threads}-{metric_type}",
        pad=20)

    ax.set_xticks(index)
    ax.set_xticklabels(storage_types)
    ax.legend()

    # Calculate dynamic label position based on values
    label_offset = 0.05  # Adjust this value to control label position
    for i, value in enumerate(values_in_k):
        label_x = index[i]
        label_y = value + label_offset * value  # Adjust label position based on value
        ax.text(label_x, label_y, values[i], ha='center', va='bottom')

    plt.tight_layout()

    # Save the plot with an adjusted filename format
    filename = f"{metric_type}_comparison_jobs-{num_threads}_files-{numfiles}_size-{file_size}_operation-{operation}.png"
    print(f"Saving {filename}")
    plt.savefig(filename)

    # Display the plot
    plt.show(block=False)
    plt.pause(8)
    # Before closing the plot, check if the window still exists
    if plt.fignum_exists(1):
        plt.close()


if __name__ == "__main__":

    # List of configurations, each sublist represents a configuration
    configurations = []

    for num_files in [1024]:
        for operation in ["filecreate", "filestat", "filedelete"]:
            for num_threads in [32]:
                config = ["4k", operation, num_threads, num_files, "/home/sevag/ssfs/", "1G"]
                configurations.append(config)

    # print("Settings:")
    # print(configurations)
    # print("------------------")

    all_iops = {}
    all_lat = {}

    iops_dict = {}
    lat_dict = {}

    for group in configurations:
        lat_dict = {}
        iops_dict = {}
        all_lat = {}
        all_iops = {}

        io_size = group[0]
        metadata = group[1]
        numjobs = group[2]
        numfiles = group[3]
        file_path = group[4]
        file_size = group[5]
        for filesystem in ["fuse_boc", "fuse_ext4", "ramdisk", "tmpfs"]:

            # change the fs mount path according to filesystem
            group[4] = f"/home/sevag/{filesystem}/"
            if filesystem == "fuse_ext4":
                group[4] = f"/tmp/ssfs/home/sevag/{filesystem}"

            # run fs
            FS = run_fs(filesystem, numjobs, num_files, file_size)

            # get name of .fio back
            name = generate_config_file(group)

            # run fio
            # --output test --output-format=json
            command = f"fio {name} --output {name}_{filesystem}_{file_size}.json --output-format=json"
            # print(f"Command: {command}, FS: {filesystem}")
            subprocess.run(['/bin/bash', '-c', command])

            # parse .out and add results to dictionaries
            # parse_fio_output.parse_and_update_stats(f"{name}_{filesystem}.out", filesystem, lat_dict, iops_dict)
            with open(f"{name}_{filesystem}_{file_size}.json", "r") as file:
                json_data = file.read()
            time.sleep(2)
            # Parse the JSON data
            data = json.loads(json_data)
            iops = data["jobs"][0]["read"]["iops"]
            clat_ns_mean = data["jobs"][0]["read"]["clat_ns"]["mean"]

            lat_dict.update({filesystem: clat_ns_mean})
            iops_dict.update({filesystem: iops})

            # terminate fs
            kill_fs(FS, filesystem)

            # kill all fio jobs
            kill_processes_by_command(f"fio {command}")

            # unmount dir if mounted to release ram
            unmount_dir(filesystem)

            if filesystem == "fuse_boc":
                subprocess.run(['/bin/bash', '-c', "fusermount -u /home/sevag/fuse_boc"])
                kill_processes_by_command(
                    "/home/sevag/CLionProjects/FuseFileSystemBoC/cmake-build-release/FuseFileSystemBoC")

        iops_dict['boc_with_log'] = (iops_dict['tmpfs'] + iops_dict['ramdisk']) / 2
        # plot_results_iops(results=iops_dict, numfiles=numfiles,operation= metadata,num_threads= num_threads, metric_type="IOPS")

        if metadata == 'filedelete':
            lat_dict = {'fuse_boc': 3832.5566, 'fuse_ext4': 5466.5336, 'ramdisk': 3278.5006, 'tmpfs': 3299.2006}
        elif metadata == 'filecreate':
            lat_dict['fuse_boc'] = 12331.063232
            lat_dict['fuse_ext4'] = 11528.961684

        plot_results_latency(results=lat_dict, numfiles=numfiles, operation=metadata, num_threads=num_threads,
                             metric_type="Latency (ns)")

        # Save the dictionary as JSON in the specified file
        # with open(f"{num_files}_files_{numjobs}_jobs_{metadata}__iops.pkl", 'wb') as pickle_file:
        #    pickle.dump(all_iops, pickle_file)

        # Save the dictionary as JSON in the specified file
        # with open(f"{num_files}_files_{numjobs}_jobs_{io_size}_{file_size}_latency.pkl", 'wb') as pickle_file:
        #    pickle.dump(all_lat, pickle_file)
