# DetiCoins

> Project developed under AAD class with the objective to mine DETICoins

## Features

- **CPU Scalar Miner**: Single-threaded SHA1 implementation
- **AVX Miner**: Parallel mining using AVX instructions (4 hashes simultaneously)
- **AVX2 Miner**: Parallel mining using AVX2 instructions (8 hashes simultaneously)
- **AVX-512 Miner**: Parallel mining using AVX-512 instructions (16 hashes simultaneously)
- **CUDA Miner**: GPU-accelerated mining using NVIDIA CUDA (speed varies by GPU)
- **OpenCL Miner**: Cross-platform GPU mining supporting NVIDIA, AMD, and Intel GPUs
- **OpenMP Support**: Multi-threading support for CPU and AVX miners to utilize multiple CPU cores
- **MPI Distributed Miner**: Client/Server architecture for distributed mining across multiple processes
- **WebAssembly Miner**: Browser-based mining using WebAssembly
- **WebAssembly SIMD Miner**: Browser-based mining with 4-way SIMD parallelization
- **Custom Coin Mining**: Mine personalized coins with embedded text (1-27 characters) using all miners (CPU, SIMD, OpenMP, CUDA)

---

## Requirements

### Basic Requirements

- GCC compiler with C11 support
- Linux operating system (Ubuntu 20.04+ recommended)
- Make build system

### CPU SIMD Requirements

| Feature | Check Command |
|---------|---------------|
| AVX support | `grep avx /proc/cpuinfo` |
| AVX2 support | `grep avx2 /proc/cpuinfo` |
| AVX-512 support | `grep avx512 /proc/cpuinfo` |

### GPU Requirements

| Vendor | Requirement |
|--------|-------------|
| NVIDIA | CUDA toolkit (includes OpenCL) |
| AMD    | Mesa OpenCL (Rusticl) |
| Intel  | Intel OpenCL Runtime |

### MPI Requirements (Distributed Mining)

```bash
sudo apt install openmpi-bin libopenmpi-dev
```

### WebAssembly Requirements (Browser Mining)

Install Emscripten SDK:
```bash
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

---

## Utilization

### Building Miners

1. Navigate to the includes folder:
   ```bash
   cd aad_assignment_1/includes
   ```

2. Build the miner of your choice:

#### Build All Miners

```bash
make all              # Build all miners (CPU, SIMD, OpenMP, GPU)
make all-single       # Build all single-threaded miners
make all-openmp       # Build all OpenMP miners
make all-gpu          # Build all GPU miners (CUDA + OpenCL)
```

#### Single-threaded SIMD

```bash
make cpu              # Build CPU scalar miner
make avx              # Build AVX parallel miner (requires AVX support)
make avx2             # Build AVX2 parallel miner (requires AVX2 support)
make avx512           # Build AVX-512 parallel miner (requires AVX-512 support)
```

#### OpenMP Multi-threaded

```bash
make cpu-openmp       # Build CPU scalar miner with OpenMP support
make avx-openmp       # Build AVX parallel miner with OpenMP support
make avx2-openmp      # Build AVX2 parallel miner with OpenMP support
make avx512-openmp    # Build AVX-512 parallel miner with OpenMP support
```

#### GPU Miners

```bash
make cuda             # Build CUDA miner (requires NVIDIA GPU with CUDA support)
make opencl           # Build OpenCL miner (cross-platform GPU)
```

#### MPI Distributed Miner

```bash
make mpi              # Build MPI Client/Server miner (requires MPI)
```

#### WebAssembly Miners

```bash
make webAssembly      # Build WebAssembly miner (requires Emscripten)
make webAssembly-simd # Build WebAssembly SIMD miner (4-way parallel)
```

---

### Running Miners

#### Standard DETI Coin Mining

**Build and run directly:**

```bash
make run-cpu                # Build and run CPU scalar miner
make run-avx                # Build and run AVX miner
make run-avx2               # Build and run AVX2 miner
make run-avx512             # Build and run AVX-512 miner
make run-cpu-openmp         # Build and run CPU scalar miner with OpenMP support
make run-avx-openmp         # Build and run AVX miner with OpenMP support
make run-avx2-openmp        # Build and run AVX2 miner with OpenMP support
make run-avx512-openmp      # Build and run AVX-512 miner with OpenMP support
make run-cuda               # Build and run CUDA miner
make run-opencl             # Build and run OpenCL miner (shows device list)
```

#### Custom Coin Mining

Mine coins with personalized text embedded (1-27 characters):

```bash
# CPU Miners
make run-cpu CUSTOM="TEXT"                    # CPU scalar with custom text
make run-avx CUSTOM="TEXT"                  # AVX with custom text
make run-avx2 CUSTOM="TEXT"                # AVX2 with custom text
make run-avx512 CUSTOM="TEXT"             # AVX-512 with custom text

# OpenMP Multi-threaded Miners
make run-cpu-openmp CUSTOM="TEXT"            # CPU OpenMP with custom text
make run-avx-openmp CUSTOM="TEXT"       # AVX OpenMP with custom text
make run-avx2-openmp CUSTOM="TEXT"      # AVX2 OpenMP with custom text
make run-avx512-openmp CUSTOM="TEXT"    # AVX-512 OpenMP with custom text

# GPU Miners
make run-cuda CUSTOM="TEXT"               # CUDA with custom text
```

**Requirements:**
- Text must be 1-27 characters long
- No newline characters allowed
- Multi-word text should be quoted: `CUSTOM="HELLO WORLD"`

**Example:**
```bash
# Mine DETI coins with "AAD2025" embedded
make run-avx2-openmp CUSTOM="AAD2025"
```

Output will show:
```
[+] CUSTOM COIN #1 (Thread 2, Lane 5)
[+] CUSTOM COIN #2 (Thread 0, Lane 3)
```

#### MPI Distributed Miner

```bash
make run-mpi                # Run with 5 workers (default)
make run-mpi NP=8           # Run with 8 workers
make run-mpi NP=4 TIME=60   # Run with 4 workers for 60 seconds
```

- `NP`: Number of MPI processes (workers + 1 master, default: 5)
- `TIME`: Duration in seconds (0 = unlimited, requires Ctrl+C to stop)

#### WebAssembly Miners

```bash
make run-webAssembly        # Build and serve WebAssembly miner
make run-webAssembly-simd   # Build and serve WebAssembly SIMD miner
```

Open `http://localhost:8000` in your browser to start mining.

**Note:** The WebAssembly miners allow you to configure the batch size (number of hashes per iteration) in the browser interface. Common values:
- 100,000: Lower CPU usage, more responsive browser
- 500,000: Balanced performance
- 1,000,000: Higher throughput, may affect browser responsiveness


**Or run the compiled binary directly:**

```bash
../bin/cpu_miner            # Run CPU scalar miner
../bin/avx_miner            # Run AVX miner
../bin/avx2_miner           # Run AVX2 miner
../bin/avx512_miner         # Run AVX-512 miner
../bin/cpu_openmp_miner     # Run CPU scalar miner with OpenMP support
../bin/avx_openmp_miner     # Run AVX miner with OpenMP support
../bin/avx2_openmp_miner    # Run AVX2 miner with OpenMP support
../bin/avx512_openmp_miner  # Run AVX-512 miner with OpenMP support
../bin/cuda_miner           # Run CUDA miner
```

#### OpenCL GPU Miner

The OpenCL miner requires platform and device selection:

1. **List available devices:**
   ```bash
   ../bin/opencl_miner
   ```
   Or use:
   ```bash
   clinfo --list
   ```

2. **Run on specific GPU:**
   ```bash
   ../bin/opencl_miner <platform_id> <device_id>
   ```

   **Examples:**
   ```bash
   ../bin/opencl_miner 0 0  # Platform 0, Device 0 (usually NVIDIA)
   ../bin/opencl_miner 1 0  # Platform 1, Device 0 (usually AMD)
   ```

3. **Example output from device listing:**
   ```
   Available OpenCL platforms and devices:
   Platform 0: NVIDIA CUDA
     Device 0: NVIDIA GeForce RTX 3080 Laptop GPU (7.79 GB, 48 CUs)
   Platform 1: rusticl
     Device 0: AMD Radeon 680M (23.19 GB, 12 CUs)
   ```

---

### Cleaning Up

Remove all build artifacts:

```bash
make clean
```

---

## OpenCL Setup Guide

### NVIDIA GPU (Automatic)

If you have NVIDIA CUDA toolkit installed, OpenCL support is already included. **No additional setup needed.**

**Verify NVIDIA OpenCL is working:**
```bash
clinfo | grep NVIDIA
```

---

### AMD GPU Setup (Ubuntu/Debian)

AMD GPUs require Mesa OpenCL drivers. Follow these steps:

#### 1. Install OpenCL Runtime

**Update package list:**
```bash
sudo apt update
```

**Install Mesa OpenCL and dependencies:**
```bash
sudo apt install mesa-opencl-icd clinfo libclc-dev opencl-headers
```

**Verify installation:**
```bash
clinfo --list
```

#### 2. Enable Rusticl for AMD Integrated Graphics

AMD integrated graphics (APUs) require the newer `rusticl` OpenCL implementation:

**Create environment configuration file:**
```bash
sudo nano /etc/profile.d/rusticl.sh
```

**Add this line:**
```bash
export RUSTICL_ENABLE=radeonsi
```

Save and exit (`Ctrl+X`, `Y`, `Enter`).

#### 3. Disable Old Clover Driver

**Rename old Mesa ICD to disable Clover:**
```bash
sudo dpkg-divert --divert /etc/OpenCL/vendors/mesa.icd.disabled --rename /etc/OpenCL/vendors/mesa.icd
```

#### 4. Apply Changes

**Reload environment variables:**
```bash
source /etc/profile.d/rusticl.sh
```
Or logout and login again.

#### 5. Verify AMD GPU is Detected

**List OpenCL devices:**
```bash
clinfo --list
```

**Check for AMD device:**
```bash
clinfo | grep "Device Name"
```

You should see output like:
```
Device Name: AMD Radeon 680M (radeonsi, rembrandt, LLVM 20.1.2, DRM 3.61)
```

#### 6. Run OpenCL Miner on AMD GPU

**Set environment variable (if not permanent):**
```bash
export RUSTICL_ENABLE=radeonsi
```

**Run miner on AMD GPU (Platform 1, Device 0):**
```bash
../bin/opencl_miner 1 0
```

---

## Intel GPU Setup

Works for **UHD, Iris Xe, Intel Arc iGPU**.

### Step 1 — Check Intel GPU

```bash
lscpu | grep "Model name"
lspci | grep VGA
```

### Step 2 — Install Intel OpenCL

Modern Intel CPUs (11th gen+, ~2020→):

```bash
sudo apt update
sudo apt install intel-opencl-icd clinfo
```

Older CPUs (6th–10th gen, 2015–2019):

```bash
sudo apt update
sudo apt install intel-opencl-icd ocl-icd-libopencl1 clinfo
```

Very old CPUs (4th–5th gen, 2013–2014):

```bash
sudo apt update
sudo apt install beignet-opencl-icd clinfo
```

### Step 3 — Verify

```bash
clinfo --list
```

Expected:

```
Platform #0: Intel(R) OpenCL Graphics
  -- Device #0: Intel(R) UHD Graphics [your model]
```

### Step 4 — Build Miner

```bash
cd aad_assignment_1/includes
make opencl
```

### Step 5 — Run on Intel GPU

```bash
../bin/opencl_miner 0 0
```

---


## Troubleshooting OpenCL

### Intel GPU not detected

```bash
sudo apt install --reinstall intel-opencl-icd
clinfo | grep Intel
```

### AMD GPU Not Detected

**Problem:** `clinfo` shows no AMD devices or "No GPU devices"

**Solution:**

1. Ensure rusticl is enabled:
   ```bash
   export RUSTICL_ENABLE=radeonsi
   ```

2. Check if Mesa OpenCL is installed:
   ```bash
   apt list --installed | grep opencl
   ```

3. Reinstall if needed:
   ```bash
   sudo apt install --reinstall mesa-opencl-icd
   ```

4. Verify with verbose output:
   ```bash
   RUSTICL_ENABLE=radeonsi clinfo
   ```

---

### Missing CLC Headers Error

**Problem:** Build error mentioning `'clc/clcfunc.h' file not found`

**Solution:**

1. Install missing headers:
   ```bash
   sudo apt install libclc-dev libclc-17-dev opencl-c-headers
   ```

2. If still failing, install LLVM components:
   ```bash
   sudo apt install clang-17 libclang-17-dev
   ```

---

### Multiple OpenCL Platforms

If you have multiple GPUs, list all platforms:

```bash
clinfo --list
```

**Example output:**
```
Platform #0: NVIDIA CUDA
  -- Device #0: NVIDIA GeForce RTX 3080
Platform #1: rusticl
  -- Device #0: AMD Radeon 680M
```

**Choose the platform/device combination when running:**
```bash
../bin/opencl_miner 0 0  # Use NVIDIA
../bin/opencl_miner 1 0  # Use AMD
```


## Output

Mined coins are saved to: `aad_assignment_1/includes/deti_coins_v2_vault.txt`

**Each line format:** `Vxx:DETI coin 2 [random_data]\n`

- `xx` represents the coin's power (number of leading zero bits in hash)
- Higher power = more valuable coin

**Example output during mining:**
```
[*] COIN #1 (OpenCL)
[*] COIN #2 (OpenCL)
[5s] 10569 M @ 2113.93 M/s | Coins: 3 | GPU: 1
```

---

## CUDA Performance Analysis (Histograms)

The CUDA miner automatically collects performance data for generating histograms required for the assignment.

### Generate Histograms

**Step 1: Run CUDA Miner to Collect Data**

The CUDA miner logs kernel execution times and coins found to `includes/cuda_histogram_data.txt`.

```bash
cd aad_assignment_1/includes
make run-cuda
# Let it run for 1-2 minutes, then press Ctrl+C
```

**Step 2: Generate Histogram Visualizations**

The scripts automatically find the data file from any directory.

**Option A - Graphical Histograms (requires matplotlib):**

```bash
cd ..  # Go to aad_assignment_1 directory
python3 histogram_related/generate_cuda_histograms.py
```

This creates:
- `histogram_related/cuda_kernel_time_histogram.png`
- `histogram_related/cuda_coins_found_histogram.png`

**Option B - Text-Based Histograms (no dependencies):**

```bash
python3 histogram_related/generate_cuda_histograms_text.py
```

This creates:
- `histogram_related/cuda_histogram_report.txt`
- Console output with ASCII bar charts

**Install Matplotlib (Optional):**

```bash
pip3 install matplotlib numpy
# or
sudo apt install python3-matplotlib python3-numpy
```

### Histogram Output

**1. Kernel Execution Time Histogram**
- Shows distribution of CUDA kernel execution times
- Mean, median, standard deviation, and outliers
- Indicates GPU performance consistency

**2. Coins Found per Kernel Histogram**
- Shows how many kernels found 0, 1, 2+ coins
- Validates probabilistic behavior (most kernels find 0 coins)
- Compares against theoretical probability (1 in 2³²)

**For detailed instructions, see:** `histogram_related/HISTOGRAM_README.md`

---

## Project Structure

```
aad_assignment_1/
├── bin/                    # Compiled binaries
│   ├── cpu_miner
│   ├── avx_miner
│   ├── cuda_miner
│   ├── opencl_miner
│   └── ...
├── Docs/                   # Assignment documentation and reports
│   ├── AAD_A1.pdf         # Assignment specification
│   └── AAD_proj1_Report.pdf # Project report
├── histogram_related/      # CUDA performance analysis tools
│   ├── generate_cuda_histograms.py      # Graphical histogram generator
│   ├── generate_cuda_histograms_text.py # Text-based histogram generator
│   ├── HISTOGRAM_README.md              # Detailed histogram instructions
│   ├── cuda_histogram_data.txt          # Raw data (generated by CUDA miner)
│   ├── cuda_kernel_time_histogram.png   # Generated histogram (graphical)
│   ├── cuda_coins_found_histogram.png   # Generated histogram (graphical)
│   └── cuda_histogram_report.txt        # Generated report (text-based)
├── includes/               # Source code directory
│   ├── CPU/                # CPU miner implementation
│   ├── AVX/                # AVX SIMD implementation
│   ├── AVX2/               # AVX2 SIMD implementation
│   ├── AVX512/             # AVX-512 SIMD implementation
│   ├── CUDA/               # CUDA GPU implementation
│   │   ├── mine_deti_coins_cuda_kernel.cu  # CUDA kernel
│   │   ├── aad_sha1_cuda_miner.c           # CUDA host code
│   │   └── aad_cuda_miner.h                # CUDA miner header
│   ├── OpenCL/             # OpenCL GPU implementation
│   │   ├── aad_sha1_opencl_kernel.cl       # OpenCL kernel
│   │   ├── aad_sha1_opencl.c               # OpenCL host code
│   │   └── aad_sha1_opencl.h               # OpenCL miner header
│   ├── SIMD_OpenMP/        # OpenMP implementations
│   │   ├── CPU/            # CPU + OpenMP
│   │   ├── AVX/            # AVX + OpenMP
│   │   ├── AVX2/           # AVX2 + OpenMP
│   │   └── AVX512/         # AVX-512 + OpenMP
│   ├── MPI_ClientServer/   # MPI distributed miner
│   │   ├── aad_mpi_common.h            # Common MPI definitions
│   │   ├── aad_mpi_master.h            # Master (server) logic
│   │   ├── aad_mpi_worker.h            # Worker (client) logic
│   │   └── aad_sha1_mpi_miner.c        # MPI miner entry point
│   ├── WebAssembly/        # WebAssembly browser miner
│   ├── WebAssembly_SIMD/   # WebAssembly SIMD miner (4-way parallel)
│   ├── aad_sha1_cpu.h      # CPU SHA1 implementation
│   ├── aad_coin_types.h    # Coin type system (DETI/CUSTOM)
│   ├── aad_vault.h         # Coin storage and validation
│   ├── aad_utilities.h     # Timing and utilities
│   ├── aad_data_types.h    # Type definitions
│   ├── aad_cuda_utilities.h # CUDA utilities
│   ├── deti_coins_v2_vault.txt # Output file (mined coins)
│   └── makefile            # Build system
└── README.md               # This file (in parent DetiCoins/ directory)
```

---

## Troubleshooting

### CPU/SIMD Miners

| Issue | Solution |
|-------|----------|
| AVX miner fails to compile | Your CPU may not support AVX instructions. Use `make run-cpu` instead. |
| AVX2 miner fails to compile | Your CPU may not support AVX2 instructions. Try `make run-avx` or `make run-cpu`. |
| AVX-512 miner fails to compile | Your CPU may not support AVX-512 instructions. Use `make run-avx2` instead. |

---

### CUDA Miner

| Issue | Solution |
|-------|----------|
| CUDA miner fails to compile | Ensure you have the NVIDIA CUDA toolkit installed and a compatible GPU. |
| CUDA runtime errors | Verify GPU is available with `nvidia-smi`. |

---

### OpenCL Miner

| Issue | Solution |
|-------|----------|
| No OpenCL devices found | Install OpenCL runtime for your GPU vendor (see [OpenCL Setup Guide](#opencl-setup-guide)). |
| AMD GPU not detected | Ensure `RUSTICL_ENABLE=radeonsi` is set and rusticl is installed. |
| Kernel compilation fails | Update Mesa drivers and install `libclc-dev`. |
| Segmentation fault | Check platform/device IDs are correct with `clinfo --list`. |
| Low performance on AMD iGPU | Integrated graphics are slower than discrete GPUs - this is expected. |

---

### General Issues

| Issue | Solution |
|-------|----------|
| Permission denied | Ensure the `bin` directory has write permissions. |
| No coins found | Mining is probabilistic and may take time. Wait a few seconds/minutes depending on miner speed. |
| High CPU/GPU usage | Mining is compute-intensive. This is normal behavior. |
| Makefile issues | Ensure you are in the correct directory (`aad_assignment_1/includes`) when running `make` commands. |

---


