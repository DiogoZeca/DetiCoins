# =========================================
# AAD 2025/2026 - DETI Coin Miners Makefile
# Run from: includes/ directory
# =========================================

CC := cc
CFLAGS_BASE := -O3 -std=c11 -D_POSIX_C_SOURCE=199309L -Wall -Wextra \
               -march=native -mtune=native -ffast-math -funroll-loops \
               -finline-functions -fomit-frame-pointer
LDFLAGS :=

# Directories (relative to includes/)
SIMD_OPENMP_DIR := ./SIMD_OpenMP
AVX_DIR := ./AVX
AVX2_DIR := ./AVX2
AVX512_DIR := ./AVX512
CPU_DIR := ./CPU
BIN_DIR := ../bin

# Include paths
INCLUDES := -I. -I$(CPU_DIR) -I$(AVX_DIR) -I$(AVX2_DIR) -I$(AVX512_DIR) \
            -I$(SIMD_OPENMP_DIR)

# Ensure bin directory exists
$(shell mkdir -p $(BIN_DIR))

.DEFAULT_GOAL := help

# =========================================
# Help
# =========================================
help:
	@echo "============================================"
	@echo "  AAD 2025/2026 - DETI Coin Miners"
	@echo "============================================"
	@echo ""
	@echo "🔥 Single-threaded SIMD miners:"
	@echo "  make cpu          - CPU scalar"
	@echo "  make avx          - AVX 4-way"
	@echo "  make avx2         - AVX2 8-way"
	@echo "  make avx512       - AVX512 16-way"
	@echo ""
	@echo "🚀 Multi-threaded OpenMP miners:"
	@echo "  make cpu-openmp       - CPU + OpenMP"
	@echo "  make avx-openmp       - AVX + OpenMP"
	@echo "  make avx2-openmp      - AVX2 + OpenMP"
	@echo "  make avx512-openmp    - AVX512 + OpenMP"
	@echo ""
	@echo "📦 Batch builds:"
	@echo "  make all-single   - Build all single-threaded"
	@echo "  make all-openmp   - Build all OpenMP versions"
	@echo "  make all          - Build everything"
	@echo ""
	@echo "🧹 Utility:"
	@echo "  make clean        - Clean build artifacts"
	@echo ""

# =========================================
# Single-threaded SIMD miners
# =========================================
cpu:
	@echo "🔨 Building CPU miner..."
	@$(CC) $(CFLAGS_BASE) $(INCLUDES) \
		-o $(BIN_DIR)/cpu_miner \
		$(CPU_DIR)/aad_sha1_cpu_miner.c
	@echo "✅ Built: $(BIN_DIR)/cpu_miner"

avx:
	@echo "🔨 Building AVX miner..."
	@$(CC) $(CFLAGS_BASE) -mavx $(INCLUDES) \
		-o $(BIN_DIR)/avx_miner \
		$(AVX_DIR)/aad_sha1_cpu_avx_miner.c
	@echo "✅ Built: $(BIN_DIR)/avx_miner"

avx2:
	@echo "🔨 Building AVX2 miner..."
	@$(CC) $(CFLAGS_BASE) -mavx2 $(INCLUDES) \
		-o $(BIN_DIR)/avx2_miner \
		$(AVX2_DIR)/aad_sha1_cpu_avx2_miner.c
	@echo "✅ Built: $(BIN_DIR)/avx2_miner"

avx512:
	@echo "🔨 Building AVX512 miner..."
	@$(CC) $(CFLAGS_BASE) -mavx512f -mavx512bw -mavx512dq -mavx512vl $(INCLUDES) \
		-o $(BIN_DIR)/avx512_miner \
		$(AVX512_DIR)/aad_sha1_cpu_avx512_miner.c
	@echo "✅ Built: $(BIN_DIR)/avx512_miner"

# =========================================
# OpenMP multi-threaded miners
# =========================================
cpu-openmp:
	@echo "🔨 Building CPU OpenMP miner..."
	@$(CC) $(CFLAGS_BASE) -fopenmp $(INCLUDES) \
		-o $(BIN_DIR)/cpu_openmp_miner \
		$(SIMD_OPENMP_DIR)/CPU/aad_sha1_cpu_openMP_miner.c
	@echo "✅ Built: $(BIN_DIR)/cpu_openmp_miner"

avx-openmp:
	@echo "🔨 Building AVX OpenMP miner..."
	@$(CC) $(CFLAGS_BASE) -mavx -fopenmp $(INCLUDES) \
		-o $(BIN_DIR)/avx_openmp_miner \
		$(SIMD_OPENMP_DIR)/AVX/aad_sha1_cpu_avx_openMP_miner.c
	@echo "✅ Built: $(BIN_DIR)/avx_openmp_miner"

avx2-openmp:
	@echo "🔨 Building AVX2 OpenMP miner..."
	@$(CC) $(CFLAGS_BASE) -mavx2 -fopenmp $(INCLUDES) \
		-o $(BIN_DIR)/avx2_openmp_miner \
		$(SIMD_OPENMP_DIR)/AVX2/aad_sha1_cpu_avx2_openMP_miner.c
	@echo "✅ Built: $(BIN_DIR)/avx2_openmp_miner"

avx512-openmp:
	@echo "🔨 Building AVX512 OpenMP miner..."
	@$(CC) $(CFLAGS_BASE) -mavx512f -mavx512bw -mavx512dq -mavx512vl -fopenmp $(INCLUDES) \
		-o $(BIN_DIR)/avx512_openmp_miner \
		$(SIMD_OPENMP_DIR)/AVX512/aad_sha1_cpu_avx512_openMP_miner.c
	@echo "✅ Built: $(BIN_DIR)/avx512_openmp_miner"

# =========================================
# Batch builds
# =========================================
all-single: cpu avx avx2 avx512
	@echo ""
	@echo "✅ All single-threaded miners built!"

all-openmp: cpu-openmp avx-openmp avx2-openmp avx512-openmp
	@echo ""
	@echo "✅ All OpenMP miners built!"

all: all-single all-openmp
	@echo ""
	@echo "╔════════════════════════════════════════════════╗"
	@echo "║  ✅ ALL MINERS BUILT SUCCESSFULLY!            ║"
	@echo "╠════════════════════════════════════════════════╣"
	@echo "║  Single-threaded: cpu, avx, avx2, avx512      ║"
	@echo "║  OpenMP:          cpu-openmp, avx-openmp      ║"
	@echo "║                   avx2-openmp, avx512-openmp  ║"
	@echo "╚════════════════════════════════════════════════╝"

# =========================================
# Run targets
# =========================================
run-cpu: cpu
	@echo "🚀 Running CPU miner..."
	@$(BIN_DIR)/cpu_miner

run-avx: avx
	@echo "🚀 Running AVX miner..."
	@$(BIN_DIR)/avx_miner

run-avx2: avx2
	@echo "🚀 Running AVX2 miner..."
	@$(BIN_DIR)/avx2_miner

run-avx512: avx512
	@echo "🚀 Running AVX512 miner..."
	@$(BIN_DIR)/avx512_miner

run-cpu-openmp: cpu-openmp
	@echo "🚀 Running CPU OpenMP miner..."
	@$(BIN_DIR)/cpu_openmp_miner

run-avx-openmp: avx-openmp
	@echo "🚀 Running AVX OpenMP miner..."
	@$(BIN_DIR)/avx_openmp_miner

run-avx2-openmp: avx2-openmp
	@echo "🚀 Running AVX2 OpenMP miner..."
	@$(BIN_DIR)/avx2_openmp_miner

run-avx512-openmp: avx512-openmp
	@echo "🚀 Running AVX512 OpenMP miner..."
	@$(BIN_DIR)/avx512_openmp_miner

# =========================================
# Clean
# =========================================
clean:
	@echo "🧹 Cleaning build artifacts..."
	@rm -f $(BIN_DIR)/*_miner
	@rm -f *.o
	@echo "✅ Clean complete"

# =========================================
# PHONY targets
# =========================================
.PHONY: help all all-single all-openmp \
        cpu avx avx2 avx512 \
        cpu-openmp avx-openmp avx2-openmp avx512-openmp \
        run-cpu run-avx run-avx2 run-avx512 \
        run-cpu-openmp run-avx-openmp run-avx2-openmp run-avx512-openmp \
        clean