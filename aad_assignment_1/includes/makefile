# =========================================
# AAD 2025/2026 - DETI Coin Miners Makefile
# =========================================

# Compiler and flags
CC := cc
# ULTIMATE-OPTIMIZED FLAGS - Added -finline-functions for aggressive inlining
# CFLAGS_BASE := -O3 -std=c11 -Wall -Wextra -I. -march=native -mtune=native -ffast-math -funroll-loops -finline-functions -fomit-frame-pointer
CFLAGS_BASE := -O3 -std=c11 -D_POSIX_C_SOURCE=199309L -Wall -Wextra -I. -march=native -mtune=native -ffast-math -funroll-loops -finline-functions -fomit-frame-pointer
LDFLAGS :=

# Directories
CPU_DIR := ./CPU
AVX_DIR := ./AVX
AVX2_DIR := ./AVX2
AVX512F_DIR := ./AVX512
SIMD_OPENMP_DIR := ./SIMD_OpenMP
CUDA_DIR := ./CUDA
BIN_DIR := ../bin

# Ensure bin directory exists
$(shell mkdir -p $(BIN_DIR))

# =========================================
# Default target
# =========================================
.DEFAULT_GOAL := help

help:
	@echo "============================================"
	@echo "  AAD 2025/2026 - DETI Coin Miners"
	@echo "============================================"
	@echo ""
	@echo "Build targets:"
	@echo "  make cpu          - Build CPU scalar miner (single-threaded)"
	@echo "  make avx          - Build AVX miner (4-way SIMD)"
	@echo "  make avx2         - Build AVX2 miner (8-way SIMD)"
	@echo "  make avx512       - Build AVX512 miner (16-way SIMD)"
	@echo "  make simd_openmp  - Build SIMD+OpenMP hybrid (multi-core + SIMD)"
	@echo "  make all_miners   - Build all CPU miners"
	@echo ""
	@echo "Run targets:"
	@echo "  make run-cpu      - Build and run CPU scalar miner"
	@echo "  make run-avx      - Build and run AVX miner"
	@echo "  make run-avx2     - Build and run AVX2 miner ⚡"
	@echo "  make run-avx512   - Build and run AVX512 miner"
	@echo "  make run-simd     - Build and run SIMD+OpenMP miner"
	@echo ""
	@echo "Utility:"
	@echo "  make clean        - Clean build artifacts"
	@echo "  make help         - Show this help message"
	@echo ""

# =========================================
# CPU scalar miner (single-threaded)
# =========================================
cpu: $(CPU_DIR)/aad_sha1_cpu_miner.c $(CPU_DIR)/aad_cpu_miner.h
	@echo "Building CPU scalar miner..."
	$(CC) $(CFLAGS_BASE) -o $(BIN_DIR)/cpu_miner $(CPU_DIR)/aad_sha1_cpu_miner.c
	@echo "✓ CPU scalar miner built: $(BIN_DIR)/cpu_miner"

# =========================================
# AVX miner (4-way SIMD, single-threaded)
# =========================================
avx: $(AVX_DIR)/aad_sha1_cpu_avx_miner.c $(AVX_DIR)/aad_cpu_avx_miner.h
	@echo "Building AVX miner..."
	$(CC) $(CFLAGS_BASE) -mavx -o $(BIN_DIR)/avx_miner $(AVX_DIR)/aad_sha1_cpu_avx_miner.c
	@echo "✓ AVX miner built: $(BIN_DIR)/avx_miner"

# =========================================
# AVX2 miner (8-way SIMD, single-threaded) ⚡ BEST SINGLE-CORE
# =========================================
avx2: $(AVX2_DIR)/aad_sha1_cpu_avx2_miner.c $(AVX2_DIR)/aad_cpu_avx2_miner.h
	@echo "Building AVX2 miner..."
	$(CC) $(CFLAGS_BASE) -mavx2 -o $(BIN_DIR)/avx2_miner $(AVX2_DIR)/aad_sha1_cpu_avx2_miner.c
	@echo "✓ AVX2 miner built: $(BIN_DIR)/avx2_miner"

# =========================================
# AVX512 miner (16-way SIMD, single-threaded)
# =========================================
avx512: $(AVX512F_DIR)/aad_sha1_cpu_avx512_miner.c $(AVX512F_DIR)/aad_cpu_avx512_miner.h
	@echo "Building AVX512 miner..."
	$(CC) $(CFLAGS_BASE) -mavx512f -o $(BIN_DIR)/avx512_miner $(AVX512F_DIR)/aad_sha1_cpu_avx512_miner.c
	@echo "✓ AVX512 miner built: $(BIN_DIR)/avx512_miner"

# =========================================
# SIMD + OpenMP hybrid (BEST MULTI-CORE PERFORMANCE)
# =========================================
simd_openmp: $(SIMD_OPENMP_DIR)/aad_cpu_simd_miner.c $(SIMD_OPENMP_DIR)/aad_cpu_simd_miner.h
	@echo "Building SIMD+OpenMP miner..."
	$(CC) $(CFLAGS_BASE) -mavx -fopenmp -o $(BIN_DIR)/simd_openmp_miner $(SIMD_OPENMP_DIR)/aad_cpu_simd_miner.c
	@echo "✓ SIMD+OpenMP miner built: $(BIN_DIR)/simd_openmp_miner"

# =========================================
# Build all miners
# =========================================
all_miners: cpu avx avx2 simd_openmp
	@echo ""
	@echo "============================================"
	@echo "✓ All miners built successfully!"
	@echo "============================================"
	@echo ""
	@echo "Recommended for single-core: ./$(BIN_DIR)/avx2_miner"
	@echo "Recommended for multi-core:  ./$(BIN_DIR)/simd_openmp_miner"

# =========================================
# Run targets
# =========================================
run-cpu: cpu
	@echo ""
	@echo "=========================================="
	@echo "Running CPU scalar miner..."
	@echo "=========================================="
	@$(BIN_DIR)/cpu_miner

run-avx: avx
	@echo ""
	@echo "=========================================="
	@echo "Running AVX miner (4-way SIMD)..."
	@echo "=========================================="
	@$(BIN_DIR)/avx_miner

run-avx2: avx2
	@echo ""
	@echo "=========================================="
	@echo "Running AVX2 miner (8-way SIMD)..."
	@echo "=========================================="
	@$(BIN_DIR)/avx2_miner

run-avx512: avx512
	@echo ""
	@echo "=========================================="
	@echo "Running AVX512 miner (16-way SIMD)..."
	@echo "=========================================="
	@$(BIN_DIR)/avx512_miner

run-simd: simd_openmp
	@echo ""
	@echo "=========================================="
	@echo "Running SIMD+OpenMP miner (best performance)..."
	@echo "=========================================="
	@$(BIN_DIR)/simd_openmp_miner

# =========================================
# Clean
# =========================================
clean:
	@echo "Cleaning build artifacts..."
	@rm -f $(BIN_DIR)/*_miner
	@rm -f *.o
	@rm -f $(CPU_DIR)/*.o $(AVX_DIR)/*.o $(AVX2_DIR)/*.o $(AVX512F_DIR)/*.o $(SIMD_OPENMP_DIR)/*.o
	@echo "✓ Clean done"

# =========================================
# PHONY targets
# =========================================
.PHONY: help all_miners cpu avx avx2 avx512 simd_openmp run-cpu run-avx run-avx2 run-avx512 run-simd clean
