import re
def parse_fio_output(output):
    lat_prefix = "lat"
    iops_prefix = "IOPS="

    lines = output.split('\n')

    avg_lat = None
    avg_iops = None

    for index,line in enumerate(lines):
        if lat_prefix in line and index-3>=0 and iops_prefix in lines[index-3]:
            # Get the next line containing the average latency value
            next_line = line
            avg_lat_start = next_line.find("avg=")
            avg_lat_end = next_line.find(",", avg_lat_start)
            avg_lat = float(next_line[avg_lat_start + 4:avg_lat_end])

        if iops_prefix in line:
            avg_iops_start = line.find(iops_prefix)
            avg_iops_end = line.find("k", avg_iops_start)
            avg_iops = f"{line[avg_iops_start + len(iops_prefix):avg_iops_end]}k"

    return avg_lat, avg_iops


# Rest of the code remains the same...

def parse_and_update_stats(file_path, filesystem_type, latency_dict, iops_dict):
    try:
        with open(file_path, "r") as file:
            fio_output = file.read()
    except FileNotFoundError:
        print(f"File '{file_path}' not found.")
        return

    avg_lat, avg_iops = parse_fio_output(fio_output)

    if avg_lat and avg_iops:
        print("parsed succsefuly")
    else:
        print("Failed to parse the output.")

    # Append the average latency and average IOPS to the corresponding lists for the filesystem type
    if filesystem_type not in latency_dict:
        latency_dict[filesystem_type] = []
    if filesystem_type not in iops_dict:
        iops_dict[filesystem_type] = []

    latency_dict[filesystem_type].append(avg_lat)
    iops_dict[filesystem_type].append(avg_iops)


if __name__ == '__main__':
    # Example usage:
    file_path = "read-8jobs-4files-4k.fio.out"
    latency_dict = {}
    iops_dict = {}

    filesystem_type = "ext4"  # Replace this with the type of filesystem used in the FIO test
    parse_and_update_stats(file_path, filesystem_type, latency_dict, iops_dict)
    print(latency_dict,iops_dict)