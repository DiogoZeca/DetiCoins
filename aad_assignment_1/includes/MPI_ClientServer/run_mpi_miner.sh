#!/bin/bash
#
# MPI Miner wrapper script - handles graceful shutdown
#

NP=${1:-5}
BIN_DIR="../../bin"

if [ $NP -lt 2 ]; then
    echo "Error: Need at least 2 processes (NP >= 2)"
    exit 1
fi

# Run mpirun and wait for it
mpirun --mca mpi_warn_on_fork 0 -np $NP $BIN_DIR/mpi_miner
exit 0
