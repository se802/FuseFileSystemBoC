import pickle
import subprocess
import time
import json
import psutil as psutil

import parse_fio_output


def generate_filename(io_type, num_jobs, num_files, io_size, file_number=None):
    filename = f"{io_type}-{num_jobs}jobs-{num_files}files-{io_size}"
    if file_number is not None:
        filename += f"-file{file_number}"
    filename += ".fio"
    return filename


def generate_config(blocksize, readwrite, numjobs, num_files, file_path, file_size):
    config_template = """[global]
blocksize={blocksize}
ioengine=libaio
readwrite={readwrite}
size={file_size}
name=test
numjobs={numjobs}
group_reporting=1
iodepth=128
fallocate=native
runtime=120
directory={dir_path}
nrfiles={numfiles}
[jobs1]
filename_format=f$filenum
"""

    file_template = "{file_number}"

    file_list = ":".join([file_template.format(file_number=i, file_path=file_path) for i in range(1, num_files + 1)])

    config_content = config_template.format(
        blocksize=blocksize,
        readwrite=readwrite,
        numjobs=numjobs,
        file_size=file_size,
        file_list=file_list,
        dir_path=file_path,
        numfiles=num_files,
    )

    return config_content


def generate_config_file(config_list):
    blocksize, readwrite, numjobs, num_files, file_path, file_size = config_list
    config_content = generate_config(blocksize, readwrite, numjobs, num_files, file_path, file_size)

    filename = generate_filename(
        readwrite,
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


def kill_fs(FS):
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
    if completed_process.returncode == 0:
        print("Command executed successfully.")
    else:
        print("Error executing the command:", completed_process.stderr)


if __name__ == "__main__":
    # List of configurations, each sublist represents a configuration
    configurations = []

    for num_files in [1, 64, 128, 512, 1024]:
        for io_size in ["4k", "32k", "128k", "512k"]:
            for operation in ["write", "randwrite", "read", "randread"]:
                for num_threads in [1, 16, 32]:
                    for file_size in ["1G", "10G", "30G"]:
                        config = [io_size, operation, num_threads, num_files, "/home/sevag/ssfs/", file_size]
                        configurations.append(config)



    print("Settings:")
    print(configurations)
    print("------------------")

    all_iops = {}
    all_lat = {}

    iops_dict = {}
    lat_dict = {}

    for i in range(0, len(configurations), 4):
        for filesystem in ["fuse_boc", "fuse_ext4", "ramdisk", "tmpfs"]:
            # for filesystem in ["fuse_boc"]:
            group = configurations[i:i + 4]
            io_size = group[0][0]
            num_jobs = group[0][2]
            num_files = group[0][3]
            file_size = group[0][5]
            for settings in group:
                numjobs = settings[2]
                numfiles = settings[3]
                file_size = settings[5]

                # change the fs mount path according to filesystem
                settings[4] = f"/home/sevag/{filesystem}/"
                if filesystem == "fuse_ext4":
                    settings[4] = f"/tmp/ssfs/home/sevag/{filesystem}"

                # run fs
                FS = run_fs(filesystem, num_jobs, num_files, file_size)

                # get name of .fio back
                name = generate_config_file(settings)

                # run fio
                command = f"fio {name} > {name}_{filesystem}_{file_size}.out"
                print(f"Command: {command}, FS: {filesystem}")
                subprocess.run(['/bin/bash', '-c', command])

                # parse .out and add results to dictionaries
                parse_fio_output.parse_and_update_stats(f"{name}_{filesystem}.out", filesystem, lat_dict, iops_dict)

                # terminate fs
                kill_fs(FS)

                # kill all fio jobs
                kill_processes_by_command(f"fio {command}")

                # unmount dir if mounted to release ram
                unmount_dir(filesystem)

                if filesystem == "fuse_boc":
                    subprocess.run(['/bin/bash', '-c', "fusermount -u /home/sevag/fuse_boc"])
                    kill_processes_by_command(
                        "/home/sevag/CLionProjects/FuseFileSystemBoC/cmake-build-release/FuseFileSystemBoC")

                all_lat[(numjobs, numfiles)] = lat_dict
                all_iops[(numjobs, numfiles)] = iops_dict

            # Save the dictionary as JSON in the specified file
        with open(f"{num_files}_files_{num_jobs}_jobs_{io_size}_{file_size}_iops.pkl", 'wb') as pickle_file:
            pickle.dump(all_iops, pickle_file)

        # Save the dictionary as JSON in the specified file
        with open(f"{num_files}_files_{num_jobs}_jobs_{io_size}_{file_size}_latency.pkl", 'wb') as pickle_file:
            pickle.dump(all_lat, pickle_file)

        lat_dict = {}
        iops_dict = {}
        all_lat = {}
        all_iops = {}
