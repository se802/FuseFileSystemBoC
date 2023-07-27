import matplotlib.pyplot as plt

# ... (Your generate_filename, generate_config, and config function implementations)

def plot_iops_comparison(results):
    # Extract IOPS values and convert them to integers or floats
    configurations = list(results.keys())
    filesystems = list(results[configurations[0]].keys())
    metrics = ['write', 'randwrite', 'read', 'randread']

    iops_values = {
        (jobs, files, fs, metric): float(result[:-1]) if '.' in result else int(result[:-1])
        for jobs, files in configurations
        for fs in filesystems
        for metric, result in zip(metrics, results[(jobs, files)][fs])
    }

    # Plot the graph (correctly separate x and y for each metric)

    plt.figure(figsize=(10, 6))

    for i, metric in enumerate(metrics):
        x = range(len(filesystems) * len(configurations))
        y = [iops_values[(jobs, files, fs, metric)] for jobs, files in configurations for fs in filesystems]
        plt.plot(x, y, label=metric)

    # Set x-axis labels
    x_labels = [f"{fs} (Jobs: {jobs}, Files: {files})" for jobs, files in configurations for fs in filesystems]
    plt.xticks(x, x_labels, rotation=45, ha='right')

    plt.xlabel('Filesystem (Jobs: X, Files: Y)')
    plt.ylabel('IOPS')
    plt.title('Comparison of IOPS for Different Filesystems')
    plt.legend()
    plt.grid(True)
    plt.tight_layout()

    # Save the plot to a file or display it (unchanged from the previous version)

    plt.savefig('iops_comparison.png')
    plt.show()

def main():
    results = {
        (8, 4): {
            'fuse_boc': ['2092k', '76.0k', '3548k', '1659k'],
            'fuse_ext4': ['1811k', '179k', '2927k', '868k'],
            'ramdisk': ['2973k', '2655k', '4765k', '3805k'],
            'tmpfs': ['3416k', '2875k', '4684k', '3797k']
        },
        # Add more configurations and results as needed
    }

    plot_iops_comparison(results)

if __name__ == "__main__":
    main()
