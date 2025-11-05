# DetiCoins
Project developed under AAD class with objective to mine DETiCoins

## Features
- **CPU Scalar Miner**: Single-threaded SHA1 implementation
- **AVX Miner**: Parallel mining using AVX instructions (4 hashes simultaneously, ~4x faster)
- **AVX2 Miner**: Parallel mining using AVX2 instructions (8 hashes simultaneously, ~8x faster) 
- **AVX-512 Miner**: Parallel mining using AVX-512 instructions (16 hashes simultaneously, ~16x faster)
- **CUDA Miner**: GPU-accelerated mining using NVIDIA CUDA (speed varies by GPU)
- **OpenMP Support**: Multi-threading support for CPU and AVX miners to utilize multiple CPU cores 


## Requirements
- GCC compiler with C11 support
- AVX support (for AVX miner) - check with `grep avx /proc/cpuinfo`
- AVX2 support (for AVX2 miner) - check with `grep avx2 /proc/cpuinfo`
- AVX-512 support (for AVX-512 miner) - check with `grep avx512 /proc/cpuinfo`
- NVIDIA CUDA toolkit (for CUDA miner) - ensure you have a compatible NVIDIA GPU

## Utilization

### Building Miners

1. Navigate to the includes folder:
   ```bash
   cd aad_assignment_1/includes
   ```

2. Build the miner of your choice:
   ```bash
   make all    # Build all miners
   make all-single  # Build all single-threaded miners
   make all-openmp    # Build all OpenMP miners
   make all-gpu    # Build all GPU miners
   make cpu    # Build CPU scalar miner
   make avx    # Build AVX parallel miner (requires AVX support)
   make avx2   # Build AVX2 parallel miner (requires AVX2 support)
   make avx512 # Build AVX-512 parallel miner (requires AVX-512 support)
   make cpu-openmp    # Build CPU scalar miner with OpenMP support
   make avx-openmp    # Build AVX parallel miner with OpenMP support
   make avx2-openmp   # Build AVX2 parallel miner with OpenMP support
   make avx512-openmp # Build AVX-512 parallel miner with OpenMP support
   make cuda  # Build CUDA miner (requires NVIDIA GPU with CUDA support)
   ```
   

### Running Miners

Run the miner directly:
```bash
make run-cpu    # Build and run CPU scalar miner
make run-avx    # Build and run AVX miner
make run-avx2   # Build and run AVX2 miner
make run-avx512 # Build and run AVX-512 miner
make run-cpu-openmp    # Build and run CPU scalar miner with OpenMP support
make run-avx-openmp    # Build and run AVX miner with OpenMP support
make run-avx2-openmp   # Build and run AVX2 miner with OpenMP support
make run-avx512-openmp # Build and run AVX-512 miner with OpenMP support
make run-cuda  # Build and run CUDA miner

```

Or run the compiled binary directly:
```bash
../bin/cpu_miner    # Run CPU scalar miner
../bin/avx_miner    # Run AVX miner
../bin/avx2_miner   # Run AVX2 miner
../bin/avx512_miner # Run AVX-512 miner
../bin/cpu_openmp_miner    # Run CPU scalar miner with OpenMP support
../bin/avx_openmp_miner    # Run AVX miner with OpenMP support
../bin/avx2_openmp_miner   # Run AVX2 miner with Open
../bin/avx512_openmp_miner # Run AVX-512 miner with OpenMP support
../bin/cuda_miner  # Run CUDA miner
```

### Cleaning Up

Remove all build artifacts:
```bash
make clean
```

## Miner Comparison

| Miner Type | Command | Expected Speed | CPU Requirements |
|------------|---------|----------------|------------------|
| CPU Scalar | `make run-cpu` | Baseline | Any x86-64 CPU |
| AVX | `make run-avx` | ~4x faster | AVX support required |
| AVX2 | `make run-avx2` | ~8x faster | AVX2 support required |
| AVX-512 | `make run-avx512` | ~16x faster | AVX-512 support required |
| CUDA | `make run-cuda` | Varies by GPU | NVIDIA GPU with CUDA support |
| OpenMP Variants | `make run-*-openmp` | Improved multi-core performance | Multi-core CPU |
|------------|---------|----------------|------------------|


## Output

Mined coins are saved to: `aad_assignment_1/deti_coins_v2_vault.txt`

Each line format: `Vxx:DETI coin 2 [random_data]\n`
- `xx` represents the coin's power (number of leading zero bits in hash)



## Troubleshooting

- **AVX miner fails to compile**: Your CPU may not support AVX instructions. Use `make run-cpu` instead.
- **AVX2 miner fails to compile**: Your CPU may not support AVX2 instructions. Use `make run-cpu` instead.
- **AVX-512 miner fails to compile**: Your CPU may not support AVX-512 instructions. Use `make run-cpu` instead.
- **CUDA miner fails to compile**: Ensure you have the NVIDIA CUDA toolkit installed and a compatible GPU.
- **Permission denied**: Ensure the `bin` directory has write permissions.
- **No coins found**: Mining is probabilistic and may take time. The program will automatically exit when a coin is found.
- **High CPU usage**: Mining is CPU-intensive. Monitor your system's performance while running the miner.
- **Incorrect output format**: Ensure you are using the correct version of the miner that matches the expected output format.
- **Makefile issues**: Ensure you are in the correct directory (`aad_assignment_1/includes`) when running make commands.
