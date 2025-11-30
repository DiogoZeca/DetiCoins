# CUDA Histogram Generation - Instructions

## Overview
This generates the **required histograms** for the AAD assignment:
1. **Kernel Execution Time Histogram** - Wall time distribution of CUDA kernel runs
2. **Coins Found Histogram** - Distribution of coins found per kernel run

## Requirements

**Option A - Graphical Histograms (Recommended):**
Install Python dependencies:
```bash
pip3 install matplotlib numpy
```

Or:
```bash
sudo apt install python3-matplotlib python3-numpy
```

**Option B - Text-Based Histograms:**
No installation needed! Use `generate_cuda_histograms_text.py` for ASCII-based output.

## Quick Usage

### Step 1: Run CUDA Miner to Collect Data

The CUDA miner automatically logs histogram data to `includes/cuda_histogram_data.txt`.

```bash
cd includes
make run-cuda
# Let it run for 1-2 minutes, then press Ctrl+C
```

Or with custom coins:
```bash
make run-cuda CUSTOM="YOUR_NAME"
```

Data file format created in `includes/cuda_histogram_data.txt`:
```
# CUDA Kernel Histogram Data
# kernel_time_ms coins_found
1.250 0
1.248 1
1.252 0
...
```

### Step 2: Generate Histograms

The scripts automatically find the data file regardless of where you run them from.

**Graphical Histograms (requires matplotlib):**
```bash
# From project root
python3 histogram_related/generate_cuda_histograms.py

# Or from histogram_related/ directory
cd histogram_related
python3 generate_cuda_histograms.py
```

**Text-Based Histograms (no dependencies):**
```bash
# From project root
python3 histogram_related/generate_cuda_histograms_text.py

# Or from histogram_related/ directory
cd histogram_related
python3 generate_cuda_histograms_text.py
```

## Output Files

### Graphical Output (`generate_cuda_histograms.py`)
Creates two PNG files:
- `cuda_kernel_time_histogram.png` - Kernel execution time distribution
- `cuda_coins_found_histogram.png` - Coins found per kernel distribution

### Text Output (`generate_cuda_histograms_text.py`)
Creates:
- `cuda_histogram_report.txt` - Statistical summary report
- Console output with ASCII bar charts

## Output Files

### 1. Kernel Time Histogram
Shows:
- Distribution of kernel execution times
- Mean and median time (vertical lines)
- Statistics box with min/max/stddev
- Total number of kernel runs

### 2. Coins Found Histogram
Shows:
- How many kernels found 0, 1, 2, ... coins
- Total coins found
- Percentage of successful runs
- Average coins per kernel

## What to Include in Your Report

Add both PNG images to your report under a "CUDA Performance Analysis" section. Discuss:

1. **Kernel Time Histogram:**
   - Is the execution time consistent?
   - Are there outliers? Why?
   - What affects kernel execution time?

2. **Coins Found Histogram:**
   - Most kernels find 0 coins (why? probability!)
   - Rare to find multiple coins in one run
   - Matches the expected probability (1 in 2^32 for value 0)

## Example Analysis Text

```
The kernel execution time histogram shows a consistent distribution with
a mean of 12.23 ms and standard deviation of 0.45 ms. This indicates
stable GPU performance with minimal variance.

The coins found histogram demonstrates that the vast majority of kernel
runs (99.97%) find zero coins, which aligns with the theoretical
probability of 1/2^32 for finding a DETI coin. Out of 1000 kernel runs,
we found 3 coins total, distributed as: 997 runs with 0 coins, 2 runs
with 1 coin, and 1 run with 2 coins.
```

## Troubleshooting

**Problem:** `FileNotFoundError: cuda_histogram_data.txt`
**Solution:** Run the CUDA miner first to generate data

**Problem:** Empty histograms
**Solution:** Let the CUDA miner run longer (at least 50-100 kernel executions)

**Problem:** `ModuleNotFoundError: No module named 'matplotlib'`
**Solution:** Install matplotlib: `pip3 install matplotlib numpy`

## Advanced: Custom Analysis

You can modify `generate_cuda_histograms.py` to:
- Change bin sizes
- Add percentile lines
- Export statistics to CSV
- Compare multiple runs

## Files Created

- `cuda_histogram_data.txt` - Raw data (gitignored)
- `cuda_kernel_time_histogram.png` - Time distribution graph
- `cuda_coins_found_histogram.png` - Coins distribution graph
- `generate_cuda_histograms.py` - Generator script
