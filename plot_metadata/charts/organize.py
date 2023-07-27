import os
import re
import shutil

def organize_files_by_attributes():
    source_directory = "."  # Current directory where the files are located
    target_directory = "Performance_Data"  # Root directory where the files will be organized

    if not os.path.exists(target_directory):
        os.mkdir(target_directory)

    pattern = r'(IOPS|Latency)\s\(ns\)_comparison_jobs-(\d+)_files-(\d+)_size-(\w+)_operation-(\w+)\.png'
    file_list = [file for file in os.listdir(source_directory) if os.path.isfile(file)]

    for file in file_list:
        match = re.match(pattern, file)
        if match:
            file_type, num_jobs, num_files, file_size, operation = match.groups()

            file_size_dir = os.path.join(target_directory, file_size)
            num_files_dir = os.path.join(file_size_dir, num_files)
            num_jobs_dir = os.path.join(num_files_dir, num_jobs)

            if not os.path.exists(file_size_dir):
                os.mkdir(file_size_dir)
            if not os.path.exists(num_files_dir):
                os.mkdir(num_files_dir)
            if not os.path.exists(num_jobs_dir):
                os.mkdir(num_jobs_dir)

            source_file_path = os.path.join(source_directory, file)
            target_file_path = os.path.join(num_jobs_dir, file)

            shutil.move(source_file_path, target_file_path)

if __name__ == "__main__":
    organize_files_by_attributes()
    print("Files organized successfully.")
