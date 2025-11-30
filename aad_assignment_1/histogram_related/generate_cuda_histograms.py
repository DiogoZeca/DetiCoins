#!/usr/bin/env python3
"""
CUDA Histogram Generator for AAD Assignment
Generates two histograms:
1. Wall time distribution of CUDA kernel executions
2. Distribution of coins found per CUDA kernel run
"""

import matplotlib.pyplot as plt
import numpy as np
import sys

def load_histogram_data(filename='cuda_histogram_data.txt'):
    """Load kernel timing and coin count data from file."""
    kernel_times = []
    coins_found = []

    try:
        with open(filename, 'r') as f:
            for line in f:
                line = line.strip()
                # Skip comments and empty lines
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

        print(f"Loaded {len(kernel_times)} kernel execution records")
        return kernel_times, coins_found

    except FileNotFoundError:
        print(f"Error: File '{filename}' not found!")
        print("Please run the CUDA miner first to generate histogram data.")
        sys.exit(1)

def plot_kernel_time_histogram(kernel_times):
    """Generate histogram of CUDA kernel execution times."""
    if not kernel_times:
        print("No kernel timing data available")
        return

    plt.figure(figsize=(12, 6))

    # Create histogram
    n, bins, patches = plt.hist(kernel_times, bins=50, edgecolor='black', alpha=0.7, color='steelblue')

    # Calculate statistics
    mean_time = np.mean(kernel_times)
    median_time = np.median(kernel_times)
    std_time = np.std(kernel_times)
    min_time = np.min(kernel_times)
    max_time = np.max(kernel_times)

    # Add vertical lines for mean and median
    plt.axvline(mean_time, color='red', linestyle='dashed', linewidth=2, label=f'Mean: {mean_time:.2f} ms')
    plt.axvline(median_time, color='green', linestyle='dashed', linewidth=2, label=f'Median: {median_time:.2f} ms')

    # Labels and title
    plt.xlabel('Kernel Execution Time (ms)', fontsize=12)
    plt.ylabel('Frequency (Number of Kernel Runs)', fontsize=12)
    plt.title('CUDA Kernel Execution Time Distribution', fontsize=14, fontweight='bold')
    plt.legend(fontsize=10)
    plt.grid(True, alpha=0.3)

    # Add statistics text box
    stats_text = f'Statistics:\n'
    stats_text += f'Mean: {mean_time:.3f} ms\n'
    stats_text += f'Median: {median_time:.3f} ms\n'
    stats_text += f'Std Dev: {std_time:.3f} ms\n'
    stats_text += f'Min: {min_time:.3f} ms\n'
    stats_text += f'Max: {max_time:.3f} ms\n'
    stats_text += f'Total Runs: {len(kernel_times)}'

    plt.text(0.98, 0.97, stats_text,
             transform=plt.gca().transAxes,
             fontsize=9,
             verticalalignment='top',
             horizontalalignment='right',
             bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5))

    plt.tight_layout()
    plt.savefig('cuda_kernel_time_histogram.png', dpi=300, bbox_inches='tight')
    print(f"Saved: cuda_kernel_time_histogram.png")
    print(f"  Mean time: {mean_time:.3f} ms")
    print(f"  Median time: {median_time:.3f} ms")
    print(f"  Std deviation: {std_time:.3f} ms")

def plot_coins_found_histogram(coins_found):
    """Generate histogram of coins found per kernel run."""
    if not coins_found:
        print("No coin count data available")
        return

    plt.figure(figsize=(12, 6))

    # Get unique values and their counts
    unique_coins = sorted(set(coins_found))
    max_coins = max(unique_coins) if unique_coins else 0

    # Create bins for discrete data (0, 1, 2, 3, ...)
    bins = np.arange(-0.5, max_coins + 1.5, 1)

    # Create histogram
    n, bins, patches = plt.hist(coins_found, bins=bins, edgecolor='black', alpha=0.7, color='orange')

    # Calculate statistics
    total_kernels = len(coins_found)
    total_coins = sum(coins_found)
    kernels_with_coins = sum(1 for c in coins_found if c > 0)
    kernels_no_coins = total_kernels - kernels_with_coins
    avg_coins_per_kernel = np.mean(coins_found)

    # Labels and title
    plt.xlabel('Number of Coins Found', fontsize=12)
    plt.ylabel('Frequency (Number of Kernel Runs)', fontsize=12)
    plt.title('Distribution of DETI Coins Found per CUDA Kernel Run', fontsize=14, fontweight='bold')
    plt.grid(True, alpha=0.3, axis='y')

    # Set x-axis to show integer values
    plt.xticks(range(max_coins + 1))

    # Add statistics text box
    stats_text = f'Statistics:\n'
    stats_text += f'Total Kernel Runs: {total_kernels}\n'
    stats_text += f'Total Coins Found: {total_coins}\n'
    stats_text += f'Runs with 0 coins: {kernels_no_coins}\n'
    stats_text += f'Runs with ≥1 coin: {kernels_with_coins}\n'
    stats_text += f'Avg coins/run: {avg_coins_per_kernel:.4f}\n'

    if kernels_with_coins > 0:
        avg_when_found = sum(coins_found) / kernels_with_coins
        stats_text += f'Avg (when >0): {avg_when_found:.2f}'

    plt.text(0.98, 0.97, stats_text,
             transform=plt.gca().transAxes,
             fontsize=9,
             verticalalignment='top',
             horizontalalignment='right',
             bbox=dict(boxstyle='round', facecolor='lightgreen', alpha=0.5))

    plt.tight_layout()
    plt.savefig('cuda_coins_found_histogram.png', dpi=300, bbox_inches='tight')
    print(f"\nSaved: cuda_coins_found_histogram.png")
    print(f"  Total coins found: {total_coins}")
    print(f"  Kernel runs: {total_kernels}")
    print(f"  Runs with coins: {kernels_with_coins} ({100*kernels_with_coins/total_kernels:.2f}%)")
    print(f"  Average coins per run: {avg_coins_per_kernel:.4f}")

def main():
    """Main function to generate both histograms."""
    print("=" * 60)
    print("CUDA Histogram Generator - AAD Assignment")
    print("=" * 60)

    # Load data
    kernel_times, coins_found = load_histogram_data()

    if not kernel_times:
        print("No data to process!")
        return

    print()

    # Generate histograms
    print("Generating histograms...")
    plot_kernel_time_histogram(kernel_times)
    plot_coins_found_histogram(coins_found)

    print()
    print("=" * 60)
    print("Histogram generation complete!")
    print("=" * 60)
    print("Generated files:")
    print("  1. cuda_kernel_time_histogram.png")
    print("  2. cuda_coins_found_histogram.png")
    print()
    print("You can include these histograms in your report.")

if __name__ == '__main__':
    main()
