//
// Arquiteturas de Alto Desempenho 2025/2026
//
// MPI Client/Server DETI Coin Miner - Main Entry Point
//
// Build: mpicc -O3 -march=native -fopenmp aad_sha1_mpi_miner.c -o mpi_miner
// Run:   mpirun -np 5 ./mpi_miner [time_seconds]
//        (1 master + 4 workers)
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#include "aad_mpi_common.h"
#include "aad_mpi_master.h"
#include "aad_mpi_worker.h"

#define DEFAULT_TIME_LIMIT 0  // 0 = unlimited (Ctrl+C to stop)

int main(int argc, char *argv[])
{
    int rank, size;
    int provided;

    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);
    if (provided < MPI_THREAD_FUNNELED) {
        fprintf(stderr, "Warning: MPI thread support level lower than requested\n");
    }

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        if (rank == 0) {
            fprintf(stderr, "Error: Need at least 2 processes (1 master + 1 worker)\n");
            fprintf(stderr, "Usage: mpirun -np N %s [time_seconds]\n", argv[0]);
            fprintf(stderr, "       N >= 2 (1 master + (N-1) workers)\n");
            fprintf(stderr, "       time_seconds: 0 = unlimited (default), >0 = run for N seconds\n");
        }
        MPI_Finalize();
        return 1;
    }

    // Parse time limit argument
    int time_limit = DEFAULT_TIME_LIMIT;
    if (argc > 1) {
        time_limit = atoi(argv[1]);
        if (time_limit < 0) time_limit = 0;
    }

    int num_workers = size - 1;

    if (rank == MPI_MASTER_RANK) {
        printf("===========================================\n");
        printf("  DETI Coin MPI Miner - AAD 2025/2026\n");
        printf("===========================================\n");
        printf("Processes: %d (1 master + %d workers)\n", size, num_workers);
        printf("Mining for SHA1 signature: 0xAAD20250\n");
        printf("Using: AVX2 SIMD + OpenMP parallelization\n");
        if (time_limit > 0) {
            printf("Time limit: %d seconds\n", time_limit);
        } else {
            printf("Time limit: unlimited (Ctrl+C to stop)\n");
        }
        printf("===========================================\n\n");

        run_master(num_workers, time_limit);
    } else {
        run_worker(rank, num_workers);
    }

    MPI_Finalize();
    return 0;
}
