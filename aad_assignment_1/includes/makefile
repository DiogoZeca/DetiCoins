# =========================================
# Local Makefile for testing inside /includes
# =========================================

# Compiler and flags
CC      := gcc
CFLAGS  := -O2 -std=c11 -Wall -Wextra -I. -I../includes
LDFLAGS := 

# Folders
CPU_DIR := CPU
BIN_DIR := ../bin

# Ensure bin directory exists
$(shell mkdir -p $(BIN_DIR))

# Default rule: build all CPU test programs
all: cpu_miner

# Example target: CPU miner (scalar version)
cpu_miner: $(CPU_DIR)/aad_sha1_cpu_miner.c
	$(CC) $(CFLAGS) -o $(BIN_DIR)/$@ $^

# Example: if you add more .c test files, list them here
# ex: cpu_test2: $(CPU_DIR)/cpu_test2.c
#      $(CC) $(CFLAGS) -o $(BIN_DIR)/$@ $^

# Clean build artifacts
clean:
	rm -f $(BIN_DIR)/*
	rm -f *.o
	@echo "Clean done."

# Run your miner directly
run: cpu_miner
	@echo "Running CPU miner..."
	$(BIN_DIR)/cpu_miner
