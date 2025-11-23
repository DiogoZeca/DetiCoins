# =========================================
# AAD 2025/2026 - DETI Coin Miners Makefile
# Run from: includes/ directory
# =========================================

CC := cc
CFLAGS_BASE := -O3 -std=c11 -D_POSIX_C_SOURCE=199309L -Wall -Wextra \
               -march=native -mtune=native -ffast-math -funroll-loops \
               -finline-functions -fomit-frame-pointer
LDFLAGS :=

# CUDA Configuration
NVCC := nvcc
NVCC_FLAGS := -O3 --ptxas-options=-v
CUDA_ARCH := native
CUDA_LDFLAGS := -lcuda

# OpenCL Configuration
OPENCL_CFLAGS := $(CFLAGS_BASE)
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    OPENCL_LDFLAGS := -framework OpenCL -lm
else
    OPENCL_LDFLAGS := -lOpenCL -lm
endif

# MPI Configuration
MPICC := mpicc
MPI_FLAGS := -O3 -march=native -fopenmp -mavx2

# Directories (relative to includes/)
SIMD_OPENMP_DIR := ./SIMD_OpenMP
AVX_DIR := ./AVX
AVX2_DIR := ./AVX2
AVX512_DIR := ./AVX512
CPU_DIR := ./CPU
CUDA_DIR := ./CUDA
OPENCL_DIR := ./OpenCL
WASM_DIR := ./WebAssembly
WASM_SIMD_DIR := ./WebAssembly_SIMD
MPI_DIR := ./MPI_ClientServer
BIN_DIR := ../bin

# Emscripten Configuration
EMCC := emcc
EMCC_FLAGS := -O3 -flto -I.
EMCC_SIMD_FLAGS := -O3 -msimd128 -I.
EMCC_EXPORTS := -s MODULARIZE=1 -s EXPORT_NAME='createMinerModule' \
	-s EXPORTED_FUNCTIONS="['_miner_init','_miner_start','_miner_stop','_miner_is_running','_miner_mine_batch','_miner_get_hashes_low','_miner_get_hashes_high','_miner_get_coins_found','_miner_get_coin_buffer_count','_miner_get_coin_buffer','_miner_clear_coin_buffer','_malloc','_free']" \
	-s EXPORTED_RUNTIME_METHODS="['HEAPU32']" \
	-s ALLOW_MEMORY_GROWTH=1 -s INITIAL_MEMORY=16777216

# Include paths
INCLUDES := -I. \
            -I$(CPU_DIR) \
            -I$(AVX_DIR) \
            -I$(AVX2_DIR) \
            -I$(AVX512_DIR) \
            -I$(SIMD_OPENMP_DIR) \
            -I$(CUDA_DIR) \
            -I$(OPENCL_DIR) \
            -I$(WASM_DIR)

# Ensure directories exist
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
	@echo "[1T] Single-threaded SIMD miners:"
	@echo "  make cpu          - CPU scalar"
	@echo "  make avx          - AVX 4-way"
	@echo "  make avx2         - AVX2 8-way"
	@echo "  make avx512       - AVX512 16-way"
	@echo ""
	@echo "[MT] Multi-threaded OpenMP miners:"
	@echo "  make cpu-openmp       - CPU + OpenMP"
	@echo "  make avx-openmp       - AVX + OpenMP"
	@echo "  make avx2-openmp      - AVX2 + OpenMP"
	@echo "  make avx512-openmp    - AVX512 + OpenMP"
	@echo ""
	@echo "[GPU] GPU miners:"
	@echo "  make cuda             - CUDA GPU miner (NVIDIA)"
	@echo "  make opencl           - OpenCL GPU miner (AMD/Intel/NVIDIA)"
	@echo ""
	@echo "[MPI] MPI distributed miner:"
	@echo "  make mpi                   - MPI Client/Server miner"
	@echo "  make run-mpi               - Run with 5 processes, unlimited time"
	@echo "  make run-mpi NP=8          - Run with N processes (1 master + N-1 workers)"
	@echo "  make run-mpi TIME=60       - Run for 60 seconds (shows final stats)"
	@echo "  make run-mpi NP=8 TIME=120 - 8 processes for 2 minutes"
	@echo ""
	@echo "[WEB] WebAssembly miners:"
	@echo "  make webAssembly          - WebAssembly miner (browser)"
	@echo "  make webAssembly-simd     - WebAssembly SIMD miner (4-way parallel)"
	@echo "  make run-webAssembly      - Build and serve WebAssembly miner"
	@echo "  make run-webAssembly-simd - Build and serve SIMD miner"
	@echo ""
	@echo "[+] Custom Coin Mining (embed your text):"
	@echo "  make run-cpu CUSTOM=\"TEXT\"           - CPU miner with custom text"
	@echo "  make run-avx2 CUSTOM=\"TEXT\"          - AVX2 miner with custom text"
	@echo "  make run-avx2-openmp CUSTOM=\"TEXT\"   - AVX2 OpenMP with custom text"
	@echo "  make run-cuda CUSTOM=\"TEXT\"          - CUDA miner with custom text"
	@echo ""
	@echo "   Custom text requirements:"
	@echo "     • 1-27 characters max"
	@echo "     • No newline characters"
	@echo "     • Multi-word text must be quoted"
	@echo ""
	@echo "[PKG] Batch builds:"
	@echo "  make all-single      - Build all single-threaded"
	@echo "  make all-openmp      - Build all OpenMP versions"
	@echo "  make all-gpu         - Build all GPU miners"
	@echo "  make all-webAssembly - Build all WebAssembly versions"
	@echo "  make all             - Build everything"
	@echo ""
	@echo "[CLEAN] Utility:"
	@echo "  make clean        - Clean build artifacts"
	@echo ""

# =========================================
# Single-threaded SIMD miners
# =========================================
cpu:
	@echo "[BUILD] Building CPU miner..."
	@$(CC) $(CFLAGS_BASE) $(INCLUDES) \
		-o $(BIN_DIR)/cpu_miner \
		$(CPU_DIR)/aad_sha1_cpu_miner.c
	@echo "[OK] Built: $(BIN_DIR)/cpu_miner"

avx:
	@echo "[BUILD] Building AVX miner..."
	@$(CC) $(CFLAGS_BASE) -mavx $(INCLUDES) \
		-o $(BIN_DIR)/avx_miner \
		$(AVX_DIR)/aad_sha1_cpu_avx_miner.c
	@echo "[OK] Built: $(BIN_DIR)/avx_miner"

avx2:
	@echo "[BUILD] Building AVX2 miner..."
	@$(CC) $(CFLAGS_BASE) -mavx2 $(INCLUDES) \
		-o $(BIN_DIR)/avx2_miner \
		$(AVX2_DIR)/aad_sha1_cpu_avx2_miner.c
	@echo "[OK] Built: $(BIN_DIR)/avx2_miner"

avx512:
	@echo "[BUILD] Building AVX512 miner..."
	@$(CC) $(CFLAGS_BASE) -mavx512f -mavx512bw -mavx512dq -mavx512vl $(INCLUDES) \
		-o $(BIN_DIR)/avx512_miner \
		$(AVX512_DIR)/aad_sha1_cpu_avx512_miner.c
	@echo "[OK] Built: $(BIN_DIR)/avx512_miner"

# =========================================
# OpenMP multi-threaded miners
# =========================================
cpu-openmp:
	@echo "[BUILD] Building CPU OpenMP miner..."
	@$(CC) $(CFLAGS_BASE) -fopenmp $(INCLUDES) \
		-o $(BIN_DIR)/cpu_openmp_miner \
		$(SIMD_OPENMP_DIR)/CPU/aad_sha1_cpu_openMP_miner.c
	@echo "[OK] Built: $(BIN_DIR)/cpu_openmp_miner"

avx-openmp:
	@echo "[BUILD] Building AVX OpenMP miner..."
	@$(CC) $(CFLAGS_BASE) -mavx -fopenmp $(INCLUDES) \
		-o $(BIN_DIR)/avx_openmp_miner \
		$(SIMD_OPENMP_DIR)/AVX/aad_sha1_cpu_avx_openMP_miner.c
	@echo "[OK] Built: $(BIN_DIR)/avx_openmp_miner"

avx2-openmp:
	@echo "[BUILD] Building AVX2 OpenMP miner..."
	@$(CC) $(CFLAGS_BASE) -mavx2 -fopenmp $(INCLUDES) \
		-o $(BIN_DIR)/avx2_openmp_miner \
		$(SIMD_OPENMP_DIR)/AVX2/aad_sha1_cpu_avx2_openMP_miner.c
	@echo "[OK] Built: $(BIN_DIR)/avx2_openmp_miner"

avx512-openmp:
	@echo "[BUILD] Building AVX512 OpenMP miner..."
	@$(CC) $(CFLAGS_BASE) -mavx512f -mavx512bw -mavx512dq -mavx512vl -fopenmp $(INCLUDES) \
		-o $(BIN_DIR)/avx512_openmp_miner \
		$(SIMD_OPENMP_DIR)/AVX512/aad_sha1_cpu_avx512_openMP_miner.c
	@echo "[OK] Built: $(BIN_DIR)/avx512_openmp_miner"

# =========================================
# CUDA GPU miner
# =========================================
cuda:
	@echo "[BUILD] Building CUDA miner..."
	@echo "   Compiling CUDA kernel..."
	@$(NVCC) $(NVCC_FLAGS) -arch=$(CUDA_ARCH) -cubin \
		-I. $(CUDA_DIR)/mine_deti_coins_cuda_kernel.cu \
		-o $(CUDA_DIR)/mine_deti_coins_cuda_kernel.cubin
	@echo "   Compiling host code..."
	@$(CC) $(CFLAGS_BASE) $(INCLUDES) \
		-o $(BIN_DIR)/cuda_miner \
		$(CUDA_DIR)/aad_sha1_cuda_miner.c \
		$(CUDA_LDFLAGS)
	@echo "[OK] Built: $(BIN_DIR)/cuda_miner"

# =========================================
# OpenCL GPU miner
# =========================================
opencl:
	@echo "[BUILD] Building OpenCL miner..."
	@echo "   Platform: $(UNAME_S)"
	@echo "   Linker flags: $(OPENCL_LDFLAGS)"
	@if [ ! -f "$(OPENCL_DIR)/aad_sha1_opencl_kernel.cl" ]; then \
		echo "❌ Error: Kernel file not found: $(OPENCL_DIR)/aad_sha1_opencl_kernel.cl"; \
		exit 1; \
	fi
	@echo "   Compiling OpenCL host code..."
	@$(CC) $(OPENCL_CFLAGS) $(INCLUDES) \
		-o $(BIN_DIR)/opencl_miner \
		$(OPENCL_DIR)/aad_sha1_opencl.c \
		$(OPENCL_LDFLAGS)
	@echo "[OK] Built: $(BIN_DIR)/opencl_miner"
	@echo "   Kernel: $(OPENCL_DIR)/aad_sha1_opencl_kernel.cl"
	@echo ""
	@echo "💡 Usage: $(BIN_DIR)/opencl_miner <platform_id> <device_id>"
	@echo "   Run without args to list available devices"

# =========================================
# MPI Client/Server miner
# =========================================
mpi:
	@echo "[MPI] Building MPI Client/Server miner..."
	@which $(MPICC) > /dev/null 2>&1 || { \
		echo "❌ Error: MPI compiler (mpicc) not found!"; \
		echo "   Install: sudo apt install libopenmpi-dev openmpi-bin"; \
		exit 1; \
	}
	@$(MPICC) $(MPI_FLAGS) -I. \
		-o $(BIN_DIR)/mpi_miner \
		$(MPI_DIR)/aad_sha1_mpi_miner.c
	@echo "[OK] Built: $(BIN_DIR)/mpi_miner"
	@echo ""
	@echo "💡 Usage: mpirun -np N $(BIN_DIR)/mpi_miner [vault_file]"
	@echo "   N >= 2 (1 master + N-1 workers)"

# =========================================
# [WEB] WebAssembly miners
# =========================================
webAssembly:
	@echo "[WEB] Building WebAssembly miner..."
	@which $(EMCC) > /dev/null 2>&1 || { \
		echo "❌ Error: Emscripten (emcc) not found!"; \
		echo "   Install: sudo apt install emscripten"; \
		echo "   Or: https://emscripten.org/docs/getting_started/downloads.html"; \
		exit 1; \
	}
	@$(EMCC) $(EMCC_FLAGS) $(EMCC_EXPORTS) \
		$(WASM_DIR)/aad_sha1_wasm_miner.c \
		-o $(WASM_DIR)/miner.js
	@echo "[OK] Built: $(WASM_DIR)/miner.js, $(WASM_DIR)/miner.wasm"
	@echo ""
	@echo "💡 To run: cd $(WASM_DIR) && python3 -m http.server 8080"
	@echo "   Then open: http://localhost:8080/index.html"

webAssembly-simd:
	@echo "[WEB] Building WebAssembly SIMD miner (4-way parallel)..."
	@which $(EMCC) > /dev/null 2>&1 || { \
		echo "❌ Error: Emscripten (emcc) not found!"; \
		echo "   Install: sudo apt install emscripten"; \
		exit 1; \
	}
	@$(EMCC) $(EMCC_SIMD_FLAGS) $(EMCC_EXPORTS) \
		$(WASM_SIMD_DIR)/aad_sha1_wasm_simd_miner.c \
		-o $(WASM_SIMD_DIR)/miner_simd.js
	@echo "[OK] Built: $(WASM_SIMD_DIR)/miner_simd.js, $(WASM_SIMD_DIR)/miner_simd.wasm"
	@echo ""
	@echo "💡 To run: cd $(WASM_SIMD_DIR) && python3 -m http.server 8080"
	@echo "   Then open: http://localhost:8080/index.html"

# =========================================
# =========================================
# Batch builds
# =========================================
all-single: cpu avx avx2 avx512
	@echo ""
	@echo "[OK] All single-threaded miners built!"

all-openmp: cpu-openmp avx-openmp avx2-openmp avx512-openmp
	@echo ""
	@echo "[OK] All OpenMP miners built!"

all-gpu: cuda opencl
	@echo ""
	@echo "[OK] All GPU miners built!"

all-webAssembly: webAssembly webAssembly-simd
	@echo ""
	@echo "[OK] All WebAssembly miners built!"

all: all-single all-openmp all-gpu mpi all-webAssembly
	@echo ""
	@echo "╔════════════════════════════════════════════════════════════╗"
	@echo "║  [OK] ALL MINERS BUILT SUCCESSFULLY!                        ║"
	@echo "╠════════════════════════════════════════════════════════════╣"
	@echo "║  Single-threaded: cpu, avx, avx2, avx512                  ║"
	@echo "║  OpenMP:          cpu-openmp, avx-openmp                  ║"
	@echo "║                   avx2-openmp, avx512-openmp              ║"
	@echo "║  GPU:             cuda, opencl                            ║"
	@echo "║  [MPI] MPI:          mpi (client/server)                     ║"
	@echo "║  [WEB] WebAssembly:  webAssembly, webAssembly-simd           ║"
	@echo "║  [+] Custom coins: Use CUSTOM=\"TEXT\" with any run target  ║"
	@echo "╚════════════════════════════════════════════════════════════╝"

# =========================================
# Run targets
# =========================================
# CUSTOM variable: set to enable custom coin mining
# Example: make run-cpu CUSTOM=DIOGO

run-cpu: cpu
ifdef CUSTOM
	@echo "[MT] Running CPU miner (CUSTOM: $(CUSTOM))..."
	@$(BIN_DIR)/cpu_miner "$(CUSTOM)"
else
	@echo "[MT] Running CPU miner..."
	@$(BIN_DIR)/cpu_miner
endif

run-avx: avx
ifdef CUSTOM
	@echo "[MT] Running AVX miner (CUSTOM: $(CUSTOM))..."
	@$(BIN_DIR)/avx_miner "$(CUSTOM)"
else
	@echo "[MT] Running AVX miner..."
	@$(BIN_DIR)/avx_miner
endif

run-avx2: avx2
ifdef CUSTOM
	@echo "[MT] Running AVX2 miner (CUSTOM: $(CUSTOM))..."
	@$(BIN_DIR)/avx2_miner "$(CUSTOM)"
else
	@echo "[MT] Running AVX2 miner..."
	@$(BIN_DIR)/avx2_miner
endif

run-avx512: avx512
ifdef CUSTOM
	@echo "[MT] Running AVX512 miner (CUSTOM: $(CUSTOM))..."
	@$(BIN_DIR)/avx512_miner "$(CUSTOM)"
else
	@echo "[MT] Running AVX512 miner..."
	@$(BIN_DIR)/avx512_miner
endif

run-cpu-openmp: cpu-openmp
ifdef CUSTOM
	@echo "[MT] Running CPU OpenMP miner (CUSTOM: $(CUSTOM))..."
	@$(BIN_DIR)/cpu_openmp_miner "$(CUSTOM)"
else
	@echo "[MT] Running CPU OpenMP miner..."
	@$(BIN_DIR)/cpu_openmp_miner
endif

run-avx-openmp: avx-openmp
ifdef CUSTOM
	@echo "[MT] Running AVX OpenMP miner (CUSTOM: $(CUSTOM))..."
	@$(BIN_DIR)/avx_openmp_miner "$(CUSTOM)"
else
	@echo "[MT] Running AVX OpenMP miner..."
	@$(BIN_DIR)/avx_openmp_miner
endif

run-avx2-openmp: avx2-openmp
ifdef CUSTOM
	@echo "[MT] Running AVX2 OpenMP miner (CUSTOM: $(CUSTOM))..."
	@$(BIN_DIR)/avx2_openmp_miner "$(CUSTOM)"
else
	@echo "[MT] Running AVX2 OpenMP miner..."
	@$(BIN_DIR)/avx2_openmp_miner
endif

run-avx512-openmp: avx512-openmp
ifdef CUSTOM
	@echo "[MT] Running AVX512 OpenMP miner (CUSTOM: $(CUSTOM))..."
	@$(BIN_DIR)/avx512_openmp_miner "$(CUSTOM)"
else
	@echo "[MT] Running AVX512 OpenMP miner..."
	@$(BIN_DIR)/avx512_openmp_miner
endif

run-cuda: cuda
ifdef CUSTOM
	@echo "[MT] Running CUDA miner (CUSTOM: $(CUSTOM))..."
	@$(BIN_DIR)/cuda_miner "$(CUSTOM)"
else
	@echo "[MT] Running CUDA miner..."
	@$(BIN_DIR)/cuda_miner
endif

run-opencl: opencl
	@echo "[MT] Running OpenCL miner..."
	@echo ""
	@echo "Available OpenCL devices:"
	@$(BIN_DIR)/opencl_miner || true
	@echo ""
	@echo "To run: $(BIN_DIR)/opencl_miner <platform_id> <device_id>"

# MPI run target (usage: make run-mpi NP=5 TIME=60)
NP ?= 5
TIME ?= 0
run-mpi: mpi
	@echo "[MPI] Running MPI Client/Server miner (1 master + $$(( $(NP) - 1 )) workers)..."
	@if [ $(NP) -lt 2 ]; then \
		echo "❌ Error: Need at least 2 processes (NP >= 2)"; \
		exit 1; \
	fi
	@mpirun --mca mpi_warn_on_fork 0 -np $(NP) $(BIN_DIR)/mpi_miner $(TIME)

# [WEB] WebAssembly run targets
run-webAssembly: webAssembly
	@echo "[WEB] Starting WebAssembly miner server..."
	@echo ""
	@echo "Open http://localhost:8080/index.html in your browser"
	@echo "Press Ctrl+C to stop the server"
	@echo ""
	@cd $(WASM_DIR) && python3 -m http.server 8080

run-webAssembly-simd: webAssembly-simd
	@echo "[WEB] Starting WebAssembly SIMD miner server..."
	@echo ""
	@echo "Open http://localhost:8080/index.html in your browser"
	@echo "Press Ctrl+C to stop the server"
	@echo ""
	@cd $(WASM_SIMD_DIR) && python3 -m http.server 8080

# =========================================
# Clean
# =========================================
clean:
	@echo "[CLEAN] Cleaning build artifacts..."
	@rm -f $(BIN_DIR)/*_miner
	@rm -f $(CUDA_DIR)/*.cubin
	@rm -f $(WASM_DIR)/*.js $(WASM_DIR)/*.wasm
	@rm -f $(WASM_SIMD_DIR)/*.js $(WASM_SIMD_DIR)/*.wasm
	@rm -f *.o
	@echo "[OK] Clean complete"

# =========================================
# PHONY targets
# =========================================
.PHONY: help all all-single all-openmp all-gpu all-webAssembly \
        cpu avx avx2 avx512 \
        cpu-openmp avx-openmp avx2-openmp avx512-openmp \
        cuda opencl mpi \
        webAssembly webAssembly-simd \
        run-cpu run-avx run-avx2 run-avx512 \
        run-cpu-openmp run-avx-openmp run-avx2-openmp run-avx512-openmp \
        run-cuda run-opencl run-mpi \
        run-webAssembly run-webAssembly-simd \
        clean
