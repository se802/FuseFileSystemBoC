import subprocess
import time

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
end_fsync=1
fallocate=native
runtime=120
[jobs1]
filename={file_list}
"""

    file_template = "{file_path}file{file_number}"

    file_list = ":".join([file_template.format(file_number=i, file_path=file_path) for i in range(1, num_files + 1)])

    config_content = config_template.format(
        blocksize=blocksize,
        readwrite=readwrite,
        numjobs=numjobs,
        file_size=file_size,
        file_list=file_list
    )

    return config_content

def config(config_list):
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

    print(f"Configuration saved to '{output_file}'")
    return output_file



def run_fs(filesystem):
    FS = None
    if filesystem == "fuse_boc":
        FS = subprocess.Popen(["/home/sevag/fs/FuseFileSystemBoC"])
    elif filesystem == "fuse_ext4":
        FS = subprocess.Popen(["/home/sevag/fs/StackFsOpt", "-r", "/", "/tmp/ssfs"])
    elif filesystem == "ramdisk":
        password = "1234"
        command = "sudo -S mount -t ramfs -o size=10G ramfs /home/sevag/ramdisk"
        FS = subprocess.run(command, shell=True, input=password, capture_output=True, text=True)
        subprocess.run("sudo -S chown sevag:sevag /home/sevag/ramdisk", shell=True, input=password, capture_output=True, text=True)
        if FS.returncode < 0:
            exit(55)
    elif filesystem == "tmpfs":
        password = "1234"
        command = "sudo -S mount -t tmpfs -o size=10G tmpfs /home/sevag/tmpfs"
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

    completed_process = subprocess.run(command, input=password, shell=True,  capture_output=True, text=True)
    if completed_process.returncode == 0:
        print("Command executed successfully.")
    else:
        print("Error executing the command:", completed_process.stderr)

if __name__ == "__main__":
    # List of configurations, each sublist represents a configuration
    configurations = [
        ["4k", "write", 8, 4, "/home/sevag/ssfs/", "8G"],
        ["4k", "randwrite", 8, 4, "/home/sevag/ssfs/", "8G"],
        ["4k", "read", 8, 4, "/home/sevag/ssfs/", "8G"],
        ["4k", "randread", 8, 4, "/home/sevag/ssfs/", "8G"],
        # Add more configurations here as needed
    ]

    all_iops = {}
    all_lat = {}

    iops_dict = {}
    lat_dict = {}

    for filesystem in ["fuse_boc","fuse_ext4","ramdisk","tmpfs"]:
        for settings in configurations:
            numjobs = settings[2]
            numfiles = settings[3]



            # change the fs mount path according to filesystem
            settings[4] = f"/home/sevag/{filesystem}/"
            if filesystem == "fuse_ext4":
                settings[4] = f"/tmp/ssfs/home/sevag/{filesystem}/"



            #run fs
            FS = run_fs(filesystem)

            #get name of .fio back
            name = config(settings)


            #run fio
            command = f"fio {name} > {name}.out"
            print(command)
            subprocess.run(['/bin/bash', '-c', command])

            #parse .out and add results to dictionaries
            parse_fio_output.parse_and_update_stats(f"{name}.out",filesystem,lat_dict,iops_dict)

            #terminate fs
            kill_fs(FS)

            #kill all fio jobs
            kill_processes_by_command(f"fio {command}")


            #unmount dir if mounted to release ram
            unmount_dir(filesystem)


            if filesystem == "fuse_boc":
                subprocess.run(['/bin/bash', '-c', "fusermount -u /home/sevag/fuse_boc"])
                kill_processes_by_command("/home/sevag/fs/FuseFileSystemBoC")


    #print(f"Done{filesystem}")
    #print(lat_dict, iops_dict)
    all_lat[(numjobs, numfiles)] = lat_dict
    all_iops[(numjobs, numfiles)] = iops_dict
    lat_dict = {}
    iops_dict = {}


    print(all_iops,all_lat)