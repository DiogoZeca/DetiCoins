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
BANANA_DIR := ./Banana_Miner
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

# Include paths - Updated to include all Banana subdirs
INCLUDES := -I. \
            -I$(CPU_DIR) \
            -I$(AVX_DIR) \
            -I$(AVX2_DIR) \
            -I$(AVX512_DIR) \
            -I$(SIMD_OPENMP_DIR) \
            -I$(CUDA_DIR) \
            -I$(OPENCL_DIR) \
            -I$(BANANA_DIR)/CPU \
            -I$(BANANA_DIR)/AVX \
            -I$(BANANA_DIR)/AVX2 \
            -I$(BANANA_DIR)/AVX512 \
            -I$(BANANA_DIR)/CUDA \
            -I$(BANANA_DIR)/SIMD_OpenMP \
            -I$(BANANA_DIR)/OpenCL \
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
	@echo "⚡ GPU miners:"
	@echo "  make cuda             - CUDA GPU miner (NVIDIA)"
	@echo "  make opencl           - OpenCL GPU miner (AMD/Intel/NVIDIA)"
	@echo ""
	@echo "🔗 MPI distributed miner:"
	@echo "  make mpi              - MPI Client/Server miner"
	@echo "  make run-mpi          - Run with 5 processes, unlimited time"
	@echo "  make run-mpi NP=8     - Run with N processes (1 master + N-1 workers)"
	@echo "  make run-mpi TIME=60  - Run for 60 seconds (shows final stats)"
	@echo "  make run-mpi NP=8 TIME=120 - 8 processes for 2 minutes"
	@echo ""
	@echo "🌐 WebAssembly miners:"
	@echo "  make webAssembly          - WebAssembly miner (browser)"
	@echo "  make webAssembly-simd     - WebAssembly SIMD miner (4-way parallel)"
	@echo "  make run-webAssembly      - Build and serve WebAssembly miner"
	@echo "  make run-webAssembly-simd - Build and serve SIMD miner"
	@echo ""
	@echo "🍌 BANANA Special Form Miners:"
	@echo "  make banana-cpu           - CPU Banana miner"
	@echo "  make banana-avx           - AVX Banana miner"
	@echo "  make banana-avx2          - AVX2 Banana miner"
	@echo "  make banana-avx512        - AVX512 Banana miner"
	@echo "  make banana-cuda          - CUDA Banana miner"
	@echo "  make banana-opencl        - OpenCL Banana miner"
	@echo "  make banana-cpu-openmp    - CPU+OpenMP Banana miner"
	@echo "  make banana-avx-openmp    - AVX+OpenMP Banana miner"
	@echo "  make banana-avx2-openmp   - AVX2+OpenMP Banana miner"
	@echo "  make banana-avx512-openmp - AVX512+OpenMP Banana miner"
	@echo "  make all-banana           - Build all banana miners"
	@echo ""
	@echo "📦 Batch builds:"
	@echo "  make all-single   - Build all single-threaded"
	@echo "  make all-openmp   - Build all OpenMP versions"
	@echo "  make all-gpu      - Build all GPU miners"
	@echo "  make all-webAssembly - Build all WebAssembly versions"
	@echo "  make all          - Build everything (including banana)"
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
# CUDA GPU miner
# =========================================
cuda:
	@echo "🔨 Building CUDA miner..."
	@echo "   Compiling CUDA kernel..."
	@$(NVCC) $(NVCC_FLAGS) -arch=$(CUDA_ARCH) -cubin \
		-I. $(CUDA_DIR)/mine_deti_coins_cuda_kernel.cu \
		-o $(CUDA_DIR)/mine_deti_coins_cuda_kernel.cubin
	@echo "   Compiling host code..."
	@$(CC) $(CFLAGS_BASE) $(INCLUDES) \
		-o $(BIN_DIR)/cuda_miner \
		$(CUDA_DIR)/aad_sha1_cuda_miner.c \
		$(CUDA_LDFLAGS)
	@echo "✅ Built: $(BIN_DIR)/cuda_miner"

# =========================================
# OpenCL GPU miner
# =========================================
opencl:
	@echo "🔨 Building OpenCL miner..."
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
	@echo "✅ Built: $(BIN_DIR)/opencl_miner"
	@echo "   Kernel: $(OPENCL_DIR)/aad_sha1_opencl_kernel.cl"
	@echo ""
	@echo "💡 Usage: $(BIN_DIR)/opencl_miner <platform_id> <device_id>"
	@echo "   Run without args to list available devices"

# =========================================
# MPI Client/Server miner
# =========================================
mpi:
	@echo "🔗 Building MPI Client/Server miner..."
	@which $(MPICC) > /dev/null 2>&1 || { \
		echo "❌ Error: MPI compiler (mpicc) not found!"; \
		echo "   Install: sudo apt install libopenmpi-dev openmpi-bin"; \
		exit 1; \
	}
	@$(MPICC) $(MPI_FLAGS) -I. \
		-o $(BIN_DIR)/mpi_miner \
		$(MPI_DIR)/aad_sha1_mpi_miner.c
	@echo "✅ Built: $(BIN_DIR)/mpi_miner"
	@echo ""
	@echo "💡 Usage: mpirun -np N $(BIN_DIR)/mpi_miner [vault_file]"
	@echo "   N >= 2 (1 master + N-1 workers)"

# =========================================
# 🌐 WebAssembly miners
# =========================================
webAssembly:
	@echo "🌐 Building WebAssembly miner..."
	@which $(EMCC) > /dev/null 2>&1 || { \
		echo "❌ Error: Emscripten (emcc) not found!"; \
		echo "   Install: sudo apt install emscripten"; \
		echo "   Or: https://emscripten.org/docs/getting_started/downloads.html"; \
		exit 1; \
	}
	@$(EMCC) $(EMCC_FLAGS) $(EMCC_EXPORTS) \
		$(WASM_DIR)/aad_sha1_wasm_miner.c \
		-o $(WASM_DIR)/miner.js
	@echo "✅ Built: $(WASM_DIR)/miner.js, $(WASM_DIR)/miner.wasm"
	@echo ""
	@echo "💡 To run: cd $(WASM_DIR) && python3 -m http.server 8080"
	@echo "   Then open: http://localhost:8080/index.html"

webAssembly-simd:
	@echo "🌐 Building WebAssembly SIMD miner (4-way parallel)..."
	@which $(EMCC) > /dev/null 2>&1 || { \
		echo "❌ Error: Emscripten (emcc) not found!"; \
		echo "   Install: sudo apt install emscripten"; \
		exit 1; \
	}
	@$(EMCC) $(EMCC_SIMD_FLAGS) $(EMCC_EXPORTS) \
		$(WASM_SIMD_DIR)/aad_sha1_wasm_simd_miner.c \
		-o $(WASM_SIMD_DIR)/miner_simd.js
	@echo "✅ Built: $(WASM_SIMD_DIR)/miner_simd.js, $(WASM_SIMD_DIR)/miner_simd.wasm"
	@echo ""
	@echo "💡 To run: cd $(WASM_SIMD_DIR) && python3 -m http.server 8080"
	@echo "   Then open: http://localhost:8080/index.html"

# =========================================
# 🍌 BANANA MINERS - Special Form Search
# =========================================
banana-cpu:
	@echo "🍌 Building BANANA CPU miner..."
	@$(CC) $(CFLAGS_BASE) $(INCLUDES) \
		-o $(BIN_DIR)/banana_cpu_miner \
		$(BANANA_DIR)/CPU/aad_sha1_cpu_banana_miner.c
	@echo "✅ Built: $(BIN_DIR)/banana_cpu_miner"

banana-avx:
	@echo "🍌 Building BANANA AVX miner..."
	@$(CC) $(CFLAGS_BASE) -mavx $(INCLUDES) \
		-o $(BIN_DIR)/banana_avx_miner \
		$(BANANA_DIR)/AVX/aad_sha1_cpu_avx_banana_miner.c
	@echo "✅ Built: $(BIN_DIR)/banana_avx_miner"

banana-avx2:
	@echo "🍌 Building BANANA AVX2 miner..."
	@$(CC) $(CFLAGS_BASE) -mavx2 $(INCLUDES) \
		-o $(BIN_DIR)/banana_avx2_miner \
		$(BANANA_DIR)/AVX2/aad_sha1_cpu_avx2_banana_miner.c
	@echo "✅ Built: $(BIN_DIR)/banana_avx2_miner"

banana-avx512:
	@echo "🍌 Building BANANA AVX512 miner..."
	@$(CC) $(CFLAGS_BASE) -mavx512f -mavx512bw -mavx512dq -mavx512vl $(INCLUDES) \
		-o $(BIN_DIR)/banana_avx512_miner \
		$(BANANA_DIR)/AVX512/aad_sha1_cpu_avx512_banana_miner.c
	@echo "✅ Built: $(BIN_DIR)/banana_avx512_miner"

banana-cuda:
	@echo "🍌 Building BANANA CUDA miner..."
	@echo "   Compiling CUDA banana kernel..."
	@$(NVCC) $(NVCC_FLAGS) -arch=$(CUDA_ARCH) -cubin \
		-I. $(BANANA_DIR)/CUDA/mine_banana_coins_cuda_kernel.cu \
		-o $(BANANA_DIR)/CUDA/mine_banana_coins_cuda_kernel.cubin
	@echo "   Compiling host code..."
	@$(CC) $(CFLAGS_BASE) $(INCLUDES) \
		-o $(BIN_DIR)/banana_cuda_miner \
		$(BANANA_DIR)/CUDA/aad_sha1_cuda_banana_miner.c \
		$(CUDA_LDFLAGS)
	@echo "✅ Built: $(BIN_DIR)/banana_cuda_miner"

banana-opencl:
	@echo "🍌 Building BANANA OpenCL miner..."
	@echo "   Platform: $(UNAME_S)"
	@if [ ! -f "$(BANANA_DIR)/OpenCL/aad_sha1_opencl_banana_kernel.cl" ]; then \
		echo "❌ Error: Kernel file not found: $(BANANA_DIR)/OpenCL/aad_sha1_opencl_banana_kernel.cl"; \
		exit 1; \
	fi
	@echo "   Compiling OpenCL banana host code..."
	@$(CC) $(OPENCL_CFLAGS) $(INCLUDES) \
		-o $(BIN_DIR)/banana_opencl_miner \
		$(BANANA_DIR)/OpenCL/aad_sha1_banana_opencl.c \
		$(OPENCL_LDFLAGS)
	@echo "✅ Built: $(BIN_DIR)/banana_opencl_miner"

banana-cpu-openmp:
	@echo "🍌 Building BANANA CPU OpenMP miner..."
	@$(CC) $(CFLAGS_BASE) -fopenmp $(INCLUDES) \
		-o $(BIN_DIR)/banana_cpu_openmp_miner \
		$(BANANA_DIR)/SIMD_OpenMP/CPU/aad_sha1_cpu_openMP_banana_miner.c
	@echo "✅ Built: $(BIN_DIR)/banana_cpu_openmp_miner"

banana-avx-openmp:
	@echo "🍌 Building BANANA AVX OpenMP miner..."
	@$(CC) $(CFLAGS_BASE) -mavx -fopenmp $(INCLUDES) \
		-o $(BIN_DIR)/banana_avx_openmp_miner \
		$(BANANA_DIR)/SIMD_OpenMP/AVX/aad_sha1_cpu_avx_openMP_banana_miner.c
	@echo "✅ Built: $(BIN_DIR)/banana_avx_openmp_miner"

banana-avx2-openmp:
	@echo "🍌 Building BANANA AVX2 OpenMP miner..."
	@$(CC) $(CFLAGS_BASE) -mavx2 -fopenmp $(INCLUDES) \
		-o $(BIN_DIR)/banana_avx2_openmp_miner \
		$(BANANA_DIR)/SIMD_OpenMP/AVX2/aad_sha1_cpu_avx2_openMP_banana_miner.c
	@echo "✅ Built: $(BIN_DIR)/banana_avx2_openmp_miner"

banana-avx512-openmp:
	@echo "🍌 Building BANANA AVX512 OpenMP miner..."
	@$(CC) $(CFLAGS_BASE) -mavx512f -mavx512bw -mavx512dq -mavx512vl -fopenmp $(INCLUDES) \
		-o $(BIN_DIR)/banana_avx512_openmp_miner \
		$(BANANA_DIR)/SIMD_OpenMP/AVX512/aad_sha1_cpu_avx512_openMP_banana_miner.c
	@echo "✅ Built: $(BIN_DIR)/banana_avx512_openmp_miner"

# =========================================
# Batch builds
# =========================================
all-single: cpu avx avx2 avx512
	@echo ""
	@echo "✅ All single-threaded miners built!"

all-openmp: cpu-openmp avx-openmp avx2-openmp avx512-openmp
	@echo ""
	@echo "✅ All OpenMP miners built!"

all-gpu: cuda opencl
	@echo ""
	@echo "✅ All GPU miners built!"

all-webAssembly: webAssembly webAssembly-simd
	@echo ""
	@echo "✅ All WebAssembly miners built!"

all-banana: banana-cpu banana-avx banana-avx2 banana-avx512 \
            banana-cuda banana-opencl \
            banana-cpu-openmp banana-avx-openmp banana-avx2-openmp banana-avx512-openmp \
            banana-validator
	@echo ""
	@echo "🍌✅ All BANANA miners built!"

all: all-single all-openmp all-gpu mpi all-webAssembly all-banana
	@echo ""
	@echo "╔════════════════════════════════════════════════════════════╗"
	@echo "║  ✅ ALL MINERS BUILT SUCCESSFULLY!                        ║"
	@echo "╠════════════════════════════════════════════════════════════╣"
	@echo "║  Single-threaded: cpu, avx, avx2, avx512                  ║"
	@echo "║  OpenMP:          cpu-openmp, avx-openmp                  ║"
	@echo "║                   avx2-openmp, avx512-openmp              ║"
	@echo "║  GPU:             cuda, opencl                            ║"
	@echo "║  🔗 MPI:          mpi (client/server)                     ║"
	@echo "║  🌐 WebAssembly:  webAssembly, webAssembly-simd           ║"
	@echo "║  🍌 Banana SIMD:  banana-cpu, banana-avx                  ║"
	@echo "║                   banana-avx2, banana-avx512              ║"
	@echo "║  🍌 Banana MP:    banana-*-openmp (x4)                    ║"
	@echo "║  🍌 Banana GPU:   banana-cuda, banana-opencl              ║"
	@echo "╚════════════════════════════════════════════════════════════╝"

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

run-cuda: cuda
	@echo "🚀 Running CUDA miner..."
	@$(BIN_DIR)/cuda_miner

run-opencl: opencl
	@echo "🚀 Running OpenCL miner..."
	@echo ""
	@echo "Available OpenCL devices:"
	@$(BIN_DIR)/opencl_miner || true
	@echo ""
	@echo "To run: $(BIN_DIR)/opencl_miner <platform_id> <device_id>"

# MPI run target (usage: make run-mpi NP=5 TIME=60)
NP ?= 5
TIME ?= 0
run-mpi: mpi
	@echo "🔗 Running MPI Client/Server miner (1 master + $$(( $(NP) - 1 )) workers)..."
	@if [ $(NP) -lt 2 ]; then \
		echo "❌ Error: Need at least 2 processes (NP >= 2)"; \
		exit 1; \
	fi
	@mpirun --mca mpi_warn_on_fork 0 -np $(NP) $(BIN_DIR)/mpi_miner $(TIME)

# 🌐 WebAssembly run targets
run-webAssembly: webAssembly
	@echo "🌐 Starting WebAssembly miner server..."
	@echo ""
	@echo "Open http://localhost:8080/index.html in your browser"
	@echo "Press Ctrl+C to stop the server"
	@echo ""
	@cd $(WASM_DIR) && python3 -m http.server 8080

run-webAssembly-simd: webAssembly-simd
	@echo "🌐 Starting WebAssembly SIMD miner server..."
	@echo ""
	@echo "Open http://localhost:8080/index.html in your browser"
	@echo "Press Ctrl+C to stop the server"
	@echo ""
	@cd $(WASM_SIMD_DIR) && python3 -m http.server 8080

# 🍌 Banana run targets
run-banana-cpu: banana-cpu
	@echo "🍌 Running BANANA CPU miner..."
	@$(BIN_DIR)/banana_cpu_miner

run-banana-avx: banana-avx
	@echo "🍌 Running BANANA AVX miner..."
	@$(BIN_DIR)/banana_avx_miner

run-banana-avx2: banana-avx2
	@echo "🍌 Running BANANA AVX2 miner..."
	@$(BIN_DIR)/banana_avx2_miner

run-banana-avx512: banana-avx512
	@echo "🍌 Running BANANA AVX512 miner..."
	@$(BIN_DIR)/banana_avx512_miner

run-banana-cuda: banana-cuda
	@echo "🍌 Running BANANA CUDA miner..."
	@$(BIN_DIR)/banana_cuda_miner

run-banana-opencl: banana-opencl
	@echo "🍌 Running BANANA OpenCL miner..."
	@echo ""
	@echo "Available OpenCL devices:"
	@$(BIN_DIR)/banana_opencl_miner || true
	@echo ""
	@echo "To run: $(BIN_DIR)/banana_opencl_miner <platform_id> <device_id>"


run-banana-cpu-openmp: banana-cpu-openmp
	@echo "🍌 Running BANANA CPU OpenMP miner..."
	@$(BIN_DIR)/banana_cpu_openmp_miner

run-banana-avx-openmp: banana-avx-openmp
	@echo "🍌 Running BANANA AVX OpenMP miner..."
	@$(BIN_DIR)/banana_avx_openmp_miner

run-banana-avx2-openmp: banana-avx2-openmp
	@echo "🍌 Running BANANA AVX2 OpenMP miner..."
	@$(BIN_DIR)/banana_avx2_openmp_miner

run-banana-avx512-openmp: banana-avx512-openmp
	@echo "🍌 Running BANANA AVX512 OpenMP miner..."
	@$(BIN_DIR)/banana_avx512_openmp_miner

# =========================================
# Clean
# =========================================
clean:
	@echo "🧹 Cleaning build artifacts..."
	@rm -f $(BIN_DIR)/*_miner
	@rm -f $(BIN_DIR)/banana_validator
	@rm -f $(CUDA_DIR)/*.cubin
	@rm -f $(BANANA_DIR)/CUDA/*.cubin
	@rm -f $(WASM_DIR)/*.js $(WASM_DIR)/*.wasm
	@rm -f $(WASM_SIMD_DIR)/*.js $(WASM_SIMD_DIR)/*.wasm
	@rm -f *.o
	@echo "✅ Clean complete"

# =========================================
# PHONY targets
# =========================================
.PHONY: help all all-single all-openmp all-gpu all-webAssembly all-banana \
        cpu avx avx2 avx512 \
        cpu-openmp avx-openmp avx2-openmp avx512-openmp \
        cuda opencl mpi \
        webAssembly webAssembly-simd \
        banana-cpu banana-avx banana-avx2 banana-avx512 \
        banana-cuda banana-opencl \
        banana-cpu-openmp banana-avx-openmp banana-avx2-openmp banana-avx512-openmp \
        banana-validator \
        run-cpu run-avx run-avx2 run-avx512 \
        run-cpu-openmp run-avx-openmp run-avx2-openmp run-avx512-openmp \
        run-cuda run-opencl run-mpi \
        run-webAssembly run-webAssembly-simd \
        run-banana-cpu run-banana-avx run-banana-avx2 run-banana-avx512 \
        run-banana-cuda run-banana-opencl \
        run-banana-cpu-openmp run-banana-avx-openmp \
        run-banana-avx2-openmp run-banana-avx512-openmp \
        clean
