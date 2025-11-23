//
// Arquiteturas de Alto Desempenho 2025/2026
//
// MPI Client/Server DETI Coin Miner - Master (Server) Logic
//

#ifndef AAD_MPI_MASTER_H
#define AAD_MPI_MASTER_H

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "aad_mpi_common.h"
#include "../aad_data_types.h"
#include "../aad_sha1_cpu.h"
#include "../aad_vault.h"

#define MAX_WORKERS 256

static volatile int master_stop_signal = 0;
static u64_t worker_hashes[MAX_WORKERS] = {0};  // Per-worker cumulative hash counts

static void master_signal_handler(int sig)
{
    (void)sig;
    master_stop_signal = 1;
}

static inline void setup_master_signal_handler(void)
{
    struct sigaction sa;
    sa.sa_handler = master_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

static inline void distribute_initial_work(int num_workers, u64_t *next_counter)
{
    for (int w = 1; w <= num_workers; w++) {
        work_range_t work;
        work.start_counter = *next_counter;
        work.end_counter = *next_counter + WORK_CHUNK_SIZE;
        *next_counter += WORK_CHUNK_SIZE;
        MPI_Send(&work, sizeof(work_range_t), MPI_BYTE, w, TAG_WORK_ASSIGN, MPI_COMM_WORLD);
    }
}

static inline void broadcast_stop_signal(int num_workers)
{
    int stop = 1;
    for (int w = 1; w <= num_workers; w++) {
        MPI_Send(&stop, 1, MPI_INT, w, TAG_STOP, MPI_COMM_WORLD);
    }
}

static inline int handle_coin_found(int *total_coins)
{
    MPI_Status status;
    int flag = 0;

    MPI_Iprobe(MPI_ANY_SOURCE, TAG_COIN_FOUND, MPI_COMM_WORLD, &flag, &status);
    if (flag) {
        coin_message_t msg;
        MPI_Recv(&msg, sizeof(coin_message_t), MPI_BYTE, status.MPI_SOURCE, TAG_COIN_FOUND, MPI_COMM_WORLD, &status);

        (*total_coins)++;
        printf("\n[*] COIN #%d (Worker %d)\n", *total_coins, msg.worker_rank);

        save_coin(msg.coin_data);
        save_coin(NULL);  // Flush immediately so coins aren't lost on SIGINT
        return 1;
    }
    return 0;
}

static inline int handle_work_request(u64_t *next_counter, int *active_workers)
{
    MPI_Status status;
    int flag = 0;

    MPI_Iprobe(MPI_ANY_SOURCE, TAG_REQUEST_WORK, MPI_COMM_WORLD, &flag, &status);
    if (flag) {
        int worker_rank;
        MPI_Recv(&worker_rank, 1, MPI_INT, status.MPI_SOURCE, TAG_REQUEST_WORK, MPI_COMM_WORLD, &status);

        work_range_t work;
        if (master_stop_signal) {
            work.start_counter = 0;
            work.end_counter = 0;
            (*active_workers)--;
        } else {
            work.start_counter = *next_counter;
            work.end_counter = *next_counter + WORK_CHUNK_SIZE;
            *next_counter += WORK_CHUNK_SIZE;
        }

        MPI_Send(&work, sizeof(work_range_t), MPI_BYTE, worker_rank, TAG_WORK_ASSIGN, MPI_COMM_WORLD);
        return 1;
    }
    return 0;
}

static inline int handle_stats_update(int num_workers, u64_t *total_hashes)
{
    MPI_Status status;
    int flag = 0;

    MPI_Iprobe(MPI_ANY_SOURCE, TAG_STATS_UPDATE, MPI_COMM_WORLD, &flag, &status);
    if (flag) {
        stats_message_t stats;
        MPI_Recv(&stats, sizeof(stats_message_t), MPI_BYTE, status.MPI_SOURCE, TAG_STATS_UPDATE, MPI_COMM_WORLD, &status);

        // Store per-worker cumulative count (REPLACE, not add!)
        worker_hashes[stats.worker_rank] = stats.hashes_done;

        // Sum all worker hashes for total
        *total_hashes = 0;
        for (int w = 1; w <= num_workers; w++) {
            *total_hashes += worker_hashes[w];
        }
        return 1;
    }
    return 0;
}

static inline int handle_worker_done(int *active_workers)
{
    MPI_Status status;
    int flag = 0;

    MPI_Iprobe(MPI_ANY_SOURCE, TAG_WORKER_DONE, MPI_COMM_WORLD, &flag, &status);
    if (flag) {
        int worker_rank;
        MPI_Recv(&worker_rank, 1, MPI_INT, status.MPI_SOURCE, TAG_WORKER_DONE, MPI_COMM_WORLD, &status);
        (*active_workers)--;
        return 1;
    }
    return 0;
}

static inline void run_master(int num_workers, int time_limit)
{
    setup_master_signal_handler();

    u64_t next_counter = 0;
    u64_t total_hashes = 0;
    int total_coins = 0;
    int active_workers = num_workers;
    int shutdown_sent = 0;
    time_t start_time = time(NULL);
    time_t last_print = start_time;

    // Initialize worker hash counts
    for (int w = 0; w < MAX_WORKERS; w++) {
        worker_hashes[w] = 0;
    }

    printf(">>> Starting MPI mining with %d workers\n", num_workers);
    printf("============================================================\n");

    distribute_initial_work(num_workers, &next_counter);

    while (active_workers > 0) {
        // Handle all pending messages
        while (handle_coin_found(&total_coins));
        while (handle_work_request(&next_counter, &active_workers));
        while (handle_stats_update(num_workers, &total_hashes));
        while (handle_worker_done(&active_workers));

        // Check elapsed time
        time_t now = time(NULL);
        double elapsed = difftime(now, start_time);

        // Check if time limit reached
        if (time_limit > 0 && elapsed >= (double)time_limit && !master_stop_signal) {
            printf("\n[Time limit reached (%d seconds)]\n", time_limit);
            master_stop_signal = 1;
        }

        // Periodic status print (every 5 seconds, like AVX2_OpenMP)
        if (difftime(now, last_print) >= 5.0) {
            double hash_rate = (elapsed > 0) ? (total_hashes / elapsed / 1e6) : 0;
            printf("[%.0fs] %lu M @ %.2f MH/s | Coins: %d | Workers: %d\n",
                   elapsed, total_hashes / 1000000UL, hash_rate, total_coins, active_workers);
            last_print = now;
        }

        // Handle stop signal (from time limit or Ctrl+C)
        if (master_stop_signal && !shutdown_sent) {
            printf("[Stopping all workers...]\n");
            broadcast_stop_signal(num_workers);
            shutdown_sent = 1;
        }

        // Small sleep to avoid busy-waiting
        struct timespec ts = {0, 100000};  // 0.1ms
        nanosleep(&ts, NULL);
    }

    // Flush all coins to vault file
    save_coin(NULL);

    double elapsed = difftime(time(NULL), start_time);
    double final_rate = (elapsed > 0) ? (total_hashes / elapsed / 1e6) : 0;

    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║              MPI CLIENT/SERVER FINAL STATISTICS            ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║ Workers:         %-37d ║\n", num_workers);
    printf("║ Total hashes:    %-37lu ║\n", total_hashes);
    printf("║ Time:            %.2f seconds%-26s║\n", elapsed, "");
    printf("║ Average rate:    %.2f MH/s%-29s║\n", final_rate, "");
    printf("║ Coins found:     %-37d ║\n", total_coins);
    printf("╚════════════════════════════════════════════════════════════╝\n");
}

#endif
