# DetiCoins

> Project developed under AAD class with the objective to mine DETICoins

## Features

- **CPU Scalar Miner**: Single-threaded SHA1 implementation
- **AVX Miner**: Parallel mining using AVX instructions (4 hashes simultaneously, ~4x faster)
- **AVX2 Miner**: Parallel mining using AVX2 instructions (8 hashes simultaneously, ~8x faster)
- **AVX-512 Miner**: Parallel mining using AVX-512 instructions (16 hashes simultaneously, ~16x faster)
- **CUDA Miner**: GPU-accelerated mining using NVIDIA CUDA (speed varies by GPU)
- **OpenCL Miner**: Cross-platform GPU mining supporting NVIDIA, AMD, and Intel GPUs
- **OpenMP Support**: Multi-threading support for CPU and AVX miners to utilize multiple CPU cores

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
| Intel  | Intel OpenCL Runtime || Vendor | Requirement |
|--------|-------------|
| NVIDIA | CUDA toolkit (includes OpenCL) |
| AMD    | Mesa OpenCL (Rusticl) |
| Intel  | Intel OpenCL Runtime |

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

---

### Running Miners

#### CPU and SIMD Miners

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

---

## Miner Comparison

| Miner Type | Command | Expected Speed | Requirements |
|------------|---------|----------------|--------------|
| CPU Scalar | `make run-cpu` | Baseline (1x) | Any x86-64 CPU |
| AVX | `make run-avx` | ~4x faster | AVX support |
| AVX2 | `make run-avx2` | ~8x faster | AVX2 support |
| AVX-512 | `make run-avx512` | ~16x faster | AVX-512 support |
| CPU OpenMP | `make run-cpu-openmp` | 4-8x faster | Multi-core CPU |
| AVX2 OpenMP | `make run-avx2-openmp` | 20-40x faster | AVX2 + multi-core |
| CUDA | `make run-cuda` | 100-500x faster | NVIDIA GPU with CUDA |
| OpenCL | `../bin/opencl_miner 0 0` | 100-500x faster | Any OpenCL GPU (NVIDIA/AMD/Intel) |


## Output

Mined coins are saved to: `aad_assignment_1/deti_coins_v2_vault.txt`

**Each line format:** `Vxx:DETI coin 2 [random_data]\n`

- `xx` represents the coin's power (number of leading zero bits in hash)
- Higher power = more valuable coin

**Example output during mining:**
```
💰 COIN #1 (OpenCL)
💰 COIN #2 (OpenCL)
[5s] 10569 M @ 2113.93 M/s | Coins: 3 | GPU: 1
```

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
├── includes/               # Source code directory
│   ├── CPU/                # CPU miner implementation
│   ├── AVX/                # AVX SIMD implementation
│   ├── AVX2/               # AVX2 SIMD implementation
│   ├── AVX512/             # AVX-512 SIMD implementation
│   ├── CUDA/               # CUDA GPU implementation
│   │   ├── mine_deti_coins_cuda_kernel.cu
│   │   ├── aad_sha1_cuda_miner.c
│   │   └── aad_cuda_miner.h
│   ├── OpenCL/             # OpenCL GPU implementation
│   │   ├── aad_sha1_opencl_kernel.cl
│   │   ├── aad_sha1_opencl.c
│   │   └── aad_sha1_opencl.h
│   ├── SIMD_OpenMP/        # OpenMP implementations
│   ├── aad_sha1_cpu.h      # CPU SHA1 implementation
│   ├── aad_vault.h         # Coin storage and validation
│   ├── aad_utilities.h     # Timing and utilities
│   ├── aad_data_types.h    # Type definitions
│   └── makefile            # Build system
├── deti_coins_v2_vault.txt # Output file (mined coins)
└── README.md               # This file
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
| CUDA architecture mismatch | Edit `makefile` and change `CUDA_ARCH := sm_86` to match your GPU architecture. |
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

## Performance Tips

1. **Use GPU miners for best performance**: OpenCL or CUDA are 100-500x faster than CPU

2. **For AMD integrated graphics**: Set realistic expectations - iGPUs are much slower than discrete GPUs

3. **Multi-GPU mining**: Run multiple instances with different platform/device IDs:

   **Terminal 1: NVIDIA GPU**
   ```bash
   ../bin/opencl_miner 0 0
   ```

   **Terminal 2: AMD GPU**
   ```bash
   export RUSTICL_ENABLE=radeonsi
   ../bin/opencl_miner 1 0
   ```

4. **CPU mining**: Use OpenMP versions (example `make run-avx2-openmp`) for best multi-core performance

---

## License

Academic project for AAD course 2025/2026.

---
