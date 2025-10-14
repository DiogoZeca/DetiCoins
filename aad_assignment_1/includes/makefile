# =========================================
# Local Makefile for testing inside /includes
# =========================================

# Compiler and flags
CC      := gcc
CFLAGS  := -O2 -std=c11 -Wall -Wextra -I. -I../includes
LDFLAGS := 

# Folders
CPU_DIR := CPU
AVX_DIR := AVX
BIN_DIR := ../bin

# Ensure bin directory exists
$(shell mkdir -p $(BIN_DIR))

# Default rule: show available targets
all:
	@echo "Available targets:"
	@echo "  make cpu       - Build CPU scalar miner"
	@echo "  make avx       - Build CPU AVX miner (4 parallel hashes)"
	@echo "  make run-cpu   - Build and run CPU scalar miner"
	@echo "  make run-avx   - Build and run CPU AVX miner"
	@echo "  make clean     - Clean build artifacts"

# CPU scalar miner
cpu: $(CPU_DIR)/aad_sha1_cpu_miner.c
	$(CC) $(CFLAGS) -o $(BIN_DIR)/cpu_miner $^
	@echo "CPU scalar miner built successfully: $(BIN_DIR)/cpu_miner"

# AVX miner (requires AVX support)
avx: $(AVX_DIR)/aad_sha1_cpu_avx_miner.c
	$(CC) $(CFLAGS) -mavx -o $(BIN_DIR)/avx_miner $^
	@echo "AVX miner built successfully: $(BIN_DIR)/avx_miner"

# Run CPU scalar miner
run-cpu: cpu
	@echo "Running CPU scalar miner..."
	$(BIN_DIR)/cpu_miner

# Run AVX miner
run-avx: avx
	@echo "Running AVX miner..."
	$(BIN_DIR)/avx_miner

# Backward compatibility aliases
cpu_miner: cpu
run: run-cpu

# Clean build artifacts
clean:
	rm -f $(BIN_DIR)/*
	rm -f *.o
	@echo "Clean done."

.PHONY: all cpu avx run-cpu run-avx cpu_miner run clean
