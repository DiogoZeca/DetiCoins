#!/usr/bin/env python3
"""
CUDA Histogram Generator (Text-based version)
For systems without matplotlib installed
Generates text-based histogram output that can be included in reports
"""

import sys
import os
from collections import Counter

def find_histogram_data_file():
    """Find histogram data file in multiple possible locations."""
    # Get the script's directory
    script_dir = os.path.dirname(os.path.abspath(__file__))

    possible_paths = [
        'cuda_histogram_data.txt',                                    # Current working directory
        os.path.join(script_dir, 'cuda_histogram_data.txt'),         # Same dir as script
        os.path.join(script_dir, '../includes/cuda_histogram_data.txt'),  # includes/ from script location
        'includes/cuda_histogram_data.txt',                          # From root directory
    ]

    for path in possible_paths:
        abs_path = os.path.abspath(path)
        if os.path.exists(abs_path):
            print(f"Found data file at: {abs_path}\n")
            return abs_path

    return None

def load_histogram_data(filename=None):
    """Load kernel timing and coin count data from file."""
    kernel_times = []
    coins_found = []

    if filename is None:
        filename = find_histogram_data_file()

    if filename is None:
        print("Error: Could not find 'cuda_histogram_data.txt' in any expected location!")
        print("Searched in:")
        print("  - Current directory")
        print("  - ../includes/")
        print("  - includes/")
        print("\nPlease run the CUDA miner first to generate histogram data:")
        print("  cd includes && make run-cuda")
        sys.exit(1)

    try:
        with open(filename, 'r') as f:
            for line in f:
                line = line.strip()
                if line.startswith('#') or not line:
                    continue
                parts = line.split()
                if len(parts) >= 2:
                    try:
                        time_ms = float(parts[0])
                        coins = int(parts[1])
                        kernel_times.append(time_ms)
                        coins_found.append(coins)
                    except ValueError:
                        continue
        return kernel_times, coins_found
    except FileNotFoundError:
        print(f"Error: File '{filename}' not found!")
        sys.exit(1)

def print_kernel_time_histogram(kernel_times):
    """Print text-based histogram of kernel times."""
    if not kernel_times:
        return

    mean_time = sum(kernel_times) / len(kernel_times)
    sorted_times = sorted(kernel_times)
    median_time = sorted_times[len(sorted_times) // 2]
    min_time = min(kernel_times)
    max_time = max(kernel_times)

    # Calculate standard deviation
    variance = sum((x - mean_time) ** 2 for x in kernel_times) / len(kernel_times)
    std_dev = variance ** 0.5

    print("=" * 70)
    print("HISTOGRAM 1: CUDA Kernel Execution Time Distribution")
    print("=" * 70)
    print(f"\nStatistics:")
    print(f"  Total kernel runs: {len(kernel_times)}")
    print(f"  Mean time:         {mean_time:.3f} ms")
    print(f"  Median time:       {median_time:.3f} ms")
    print(f"  Std deviation:     {std_dev:.3f} ms")
    print(f"  Min time:          {min_time:.3f} ms")
    print(f"  Max time:          {max_time:.3f} ms")

    # Create bins
    num_bins = min(20, len(kernel_times) // 2)
    if num_bins < 5:
        num_bins = 5

    bin_width = (max_time - min_time) / num_bins
    bins = [0] * num_bins

    for time in kernel_times:
        bin_idx = min(int((time - min_time) / bin_width), num_bins - 1)
        bins[bin_idx] += 1

    max_count = max(bins) if bins else 1
    bar_width = 50  # characters

    print(f"\nDistribution (bin width: {bin_width:.3f} ms):")
    print()

    for i, count in enumerate(bins):
        bin_start = min_time + i * bin_width
        bin_end = bin_start + bin_width
        bar_len = int((count / max_count) * bar_width) if max_count > 0 else 0
        bar = '█' * bar_len
        print(f"{bin_start:7.2f}-{bin_end:7.2f} ms | {bar} {count}")

    print()

def print_coins_found_histogram(coins_found):
    """Print text-based histogram of coins found."""
    if not coins_found:
        return

    coin_counts = Counter(coins_found)
    max_coins = max(coins_found)
    total_kernels = len(coins_found)
    total_coins = sum(coins_found)
    kernels_with_coins = sum(1 for c in coins_found if c > 0)

    print("=" * 70)
    print("HISTOGRAM 2: DETI Coins Found per Kernel Run")
    print("=" * 70)
    print(f"\nStatistics:")
    print(f"  Total kernel runs:   {total_kernels}")
    print(f"  Total coins found:   {total_coins}")
    print(f"  Runs with 0 coins:   {coin_counts[0]} ({100*coin_counts[0]/total_kernels:.2f}%)")
    print(f"  Runs with ≥1 coin:   {kernels_with_coins} ({100*kernels_with_coins/total_kernels:.2f}%)")
    print(f"  Avg coins per run:   {total_coins/total_kernels:.6f}")

    if kernels_with_coins > 0:
        avg_when_found = total_coins / kernels_with_coins
        print(f"  Avg (when >0):       {avg_when_found:.3f}")

    max_count = max(coin_counts.values())
    bar_width = 50

    print(f"\nDistribution:")
    print()

    for coins in range(max_coins + 1):
        count = coin_counts[coins]
        bar_len = int((count / max_count) * bar_width) if max_count > 0 else 0
        bar = '█' * bar_len
        percentage = (count / total_kernels) * 100
        print(f"{coins:2d} coins | {bar} {count:6d} ({percentage:6.2f}%)")

    print()

def save_report(kernel_times, coins_found):
    """Save a formatted text report."""
    # Save to script directory
    script_dir = os.path.dirname(os.path.abspath(__file__))
    output_path = os.path.join(script_dir, 'cuda_histogram_report.txt')

    with open(output_path, 'w') as f:
        f.write("CUDA PERFORMANCE HISTOGRAM ANALYSIS\n")
        f.write("=" * 70 + "\n\n")

        # Kernel time stats
        mean_time = sum(kernel_times) / len(kernel_times)
        median = sorted(kernel_times)[len(kernel_times) // 2]

        f.write("1. KERNEL EXECUTION TIME ANALYSIS\n")
        f.write("-" * 70 + "\n")
        f.write(f"Mean execution time:    {mean_time:.3f} ms\n")
        f.write(f"Median execution time:  {median:.3f} ms\n")
        f.write(f"Min execution time:     {min(kernel_times):.3f} ms\n")
        f.write(f"Max execution time:     {max(kernel_times):.3f} ms\n")
        f.write(f"Total kernel runs:      {len(kernel_times)}\n\n")

        # Coins stats
        total_coins = sum(coins_found)
        coin_counts = Counter(coins_found)

        f.write("2. COINS FOUND ANALYSIS\n")
        f.write("-" * 70 + "\n")
        f.write(f"Total coins found:      {total_coins}\n")
        f.write(f"Kernels with 0 coins:   {coin_counts[0]}\n")
        f.write(f"Kernels with ≥1 coin:   {sum(1 for c in coins_found if c > 0)}\n")
        f.write(f"Success rate:           {100*sum(1 for c in coins_found if c > 0)/len(coins_found):.4f}%\n")
        f.write(f"\nTheoretical probability: ~0.000000023% (1 in 2^32)\n")

    print(f"Report saved to: {output_path}")

def main():
    print("=" * 70)
    print("CUDA HISTOGRAM GENERATOR (Text-based)")
    print("=" * 70)
    print()

    kernel_times, coins_found = load_histogram_data()

    if not kernel_times:
        print("No data to process!")
        return

    print(f"Loaded {len(kernel_times)} kernel execution records\n")

    print_kernel_time_histogram(kernel_times)
    print_coins_found_histogram(coins_found)

    save_report(kernel_times, coins_found)

    print("=" * 70)
    print("COMPLETE!")
    print("=" * 70)
    print("\nFor graphical histograms, install matplotlib and run:")
    print("  pip3 install matplotlib numpy")
    print("  python3 generate_cuda_histograms.py")

if __name__ == '__main__':
    main()
