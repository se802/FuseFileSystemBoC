import matplotlib.pyplot as plt

def convert_to_int_with_k(value):
    if value.endswith('k'):
        return int(float(value[:-1]) * 1000)
    else:
        return int(value)

def create_filesystem_comparison_chart(data, num_jobs, num_files):
    filesystems = list(data[(num_jobs, num_files)].keys())
    metrics = ['IOPS write', 'IOPS randwrite', 'IOPS read', 'IOPS randread']

    # Convert data to numeric values
    for fs in filesystems:
        data[(num_jobs, num_files)][fs] = [convert_to_int_with_k(value) for value in data[(num_jobs, num_files)][fs]]

    # Transpose the data to have metrics as keys and IOPS values per filesystem as values
    transposed_data = {metric: [data[(num_jobs, num_files)][fs][i] for fs in filesystems] for i, metric in enumerate(metrics)}

    # Create a bar chart
    fig, ax = plt.subplots(figsize=(10, 6))
    bar_width = 0.2
    index = range(len(metrics))

    for i, fs in enumerate(filesystems):
        values = [transposed_data[metric][i] for metric in metrics]
        ax.bar([pos + i * bar_width for pos in index], values, bar_width, label=fs)

    ax.set_xlabel('I/O type')
    ax.set_ylabel('IOPS in thousands')
    ax.set_title(f'Comparison of IOPS for different filesystems (Jobs: {num_jobs}, Files: {num_files})')
    ax.set_xticks([pos + 1.5 * bar_width for pos in index])
    ax.set_xticklabels(metrics)
    ax.legend()

    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    # Replace this 'data' variable with your actual data
    data = {
        (8, 4): {
            'fuse_boc': ['2092k', '76.0k', '3548k', '1659k'],
            'fuse_ext4': ['1811k', '179k', '2927k', '868k'],
            'ramdisk': ['2973k', '2655k', '4765k', '3805k'],
            'tmpfs': ['3416k', '2875k', '4684k', '3797k']
        }
    }

    num_jobs = 8
    num_files = 4

    create_filesystem_comparison_chart(data, num_jobs, num_files)
