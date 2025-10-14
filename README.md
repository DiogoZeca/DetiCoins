# DetiCoins
Project developed under AAD class with objective to mine DETiCoins

## Features
- **CPU Scalar Miner**: Single-threaded SHA1 implementation
- **AVX Miner**: Parallel mining using AVX instructions (4 hashes simultaneously, ~4x faster)

## Requirements
- GCC compiler with C11 support
- AVX support (for AVX miner) - check with `grep avx /proc/cpuinfo`

## Utilization

### Building Miners

1. Navigate to the includes folder:
   ```bash
   cd aad_assignment_1/includes
   ```

2. Build the miner of your choice:
   ```bash
   make cpu    # Build CPU scalar miner
   make avx    # Build AVX parallel miner (requires AVX support)
   ```

### Running Miners

Run the miner directly:
```bash
make run-cpu    # Build and run CPU scalar miner
make run-avx    # Build and run AVX miner
```

Or run the compiled binary directly:
```bash
../bin/cpu_miner    # Run CPU scalar miner
../bin/avx_miner    # Run AVX miner
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

## Output

Mined coins are saved to: `aad_assignment_1/deti_coins_v2_vault.txt`

Each line format: `Vxx:DETI coin 2 [random_data]\n`
- `xx` represents the coin's power (number of leading zero bits in hash)

## Example Usage

```bash
# Quick start with AVX miner (fastest)
cd aad_assignment_1/includes
make run-avx

# Or use CPU scalar miner (compatible with all systems)
make run-cpu
```

## Troubleshooting

- **AVX miner fails to compile**: Your CPU may not support AVX instructions. Use `make run-cpu` instead.
- **Permission denied**: Ensure the `bin` directory has write permissions.
- **No coins found**: Mining is probabilistic and may take time. The program will automatically exit when a coin is found.