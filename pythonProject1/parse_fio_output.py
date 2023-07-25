import re
def parse_fio_output(output):
    lat_prefix = "lat"
    iops_prefix = "IOPS="

    lines = output.split('\n')

    avg_lat = None
    avg_iops = None

    # Dictionary defining conversion factors to usec
    conversion_factors = {
        "nsec": 1e-3,  # nanoseconds to microseconds
        "usec": 1,  # microseconds (no conversion needed)
        "msec": 1e3,  # milliseconds to microseconds
        "sec": 1e6,  # seconds to microseconds
    }

    for index,line in enumerate(lines):
        if lat_prefix in line and index-3>=0 and iops_prefix in lines[index-3]:
            # Get the next line containing the average latency value
            next_line = line
            avg_lat_start = next_line.find("avg=")
            avg_lat_end = next_line.find(",", avg_lat_start)
            unit_measurement = next_line.split(':')[0].strip().split(' ')[1].split('(')[-1].split(')')[0].strip()
            avg_lat = float(next_line[avg_lat_start + 4:avg_lat_end]) * conversion_factors[unit_measurement]

        if iops_prefix in line:
            avg_iops_start = line.find(iops_prefix)
            comma_index = line.find(",", avg_iops_start)
            avg_iops_value = line[avg_iops_start + len(iops_prefix):comma_index]
            iops = float(line[avg_iops_start + len(iops_prefix):comma_index-1])
            # Check if the value has a suffix and perform the appropriate conversion
            if avg_iops_value.endswith("M"):
                avg_iops = f"{iops*1000}k"
            elif avg_iops_value.endswith("k"):
                avg_iops = f"{iops}k"
            elif avg_iops_value.endswith("G"):
                avg_iops = f"{iops*1000**2}k"
            elif avg_iops_value.endswith("T"):
                avg_iops = f"{iops*1000**3}k"
            elif avg_iops_value.endswith("P"):
                avg_iops = f"{iops*1000**4}k"
            elif avg_iops_value[-1].isdigit():
                avg_iops_value = line[avg_iops_start + len(iops_prefix):comma_index]
                iops = float(line[avg_iops_start + len(iops_prefix):comma_index ])
                # If the last character is a digit, divide by 1000
                avg_iops = f"{iops / 1000}k"

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
        print("parsed successfully")
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
    file_path = "read-32jobs-4files-4k.fio.out"
    latency_dict = {}
    iops_dict = {}

    filesystem_type = "ext4"  # Replace this with the type of filesystem used in the FIO test
    parse_and_update_stats(file_path, filesystem_type, latency_dict, iops_dict)
    print(latency_dict,iops_dict)