#ifndef AAD_MPI_WORKER_H
#define AAD_MPI_WORKER_H

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>
#include <immintrin.h>
#include "aad_mpi_common.h"
#include "../aad_data_types.h"
#include "../aad_sha1_cpu.h"

static volatile int worker_stop_signal = 0;

static inline void init_coin_data_avx2_mpi(v8si coin[14])
{
    coin[0] = (v8si){0x44455449u, 0x44455449u, 0x44455449u, 0x44455449u,
                     0x44455449u, 0x44455449u, 0x44455449u, 0x44455449u};
    coin[1] = (v8si){0x20636F69u, 0x20636F69u, 0x20636F69u, 0x20636F69u,
                     0x20636F69u, 0x20636F69u, 0x20636F69u, 0x20636F69u};
    coin[2] = (v8si){0x6E203220u, 0x6E203220u, 0x6E203220u, 0x6E203220u,
                     0x6E203220u, 0x6E203220u, 0x6E203220u, 0x6E203220u};
    coin[6] = (v8si){0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
    coin[7] = (v8si){0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
    coin[8] = (v8si){0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
    coin[9] = (v8si){0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
    coin[10] = (v8si){0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
    coin[11] = (v8si){0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
    coin[12] = (v8si){0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
}

static inline void update_counters_avx2_mpi(v8si coin[14], u64_t counter)
{
    u32_t base_counters[8];
    for (int i = 0; i < 8; i++)
        base_counters[i] = (u32_t)((counter + i) & 0xFFFFFFFFu);

    coin[3] = (v8si){base_counters[0], base_counters[1], base_counters[2], base_counters[3],
                     base_counters[4], base_counters[5], base_counters[6], base_counters[7]};

    u32_t counter_high = (u32_t)((counter >> 32) & 0xFFFFFFFFu);
    coin[4] = (v8si){counter_high, counter_high, counter_high, counter_high,
                     counter_high, counter_high, counter_high, counter_high};

    u32_t time_seed = (u32_t)time(NULL);
    coin[5] = (v8si){time_seed, time_seed, time_seed, time_seed,
                     time_seed, time_seed, time_seed, time_seed};

    coin[13] = (v8si){0x00000A80u, 0x00000A80u, 0x00000A80u, 0x00000A80u,
                      0x00000A80u, 0x00000A80u, 0x00000A80u, 0x00000A80u};
}

static inline int validate_coin_mpi(u32_t coin[14])
{
    u08_t *bytes = (u08_t *)coin;
    for (int i = 12; i < 54; i++)
        if (bytes[i ^ 3] == '\n')
            return 0;
    return 1;
}

static inline void send_coin_to_master(u32_t coin[14], int worker_rank)
{
    coin_message_t msg;
    memcpy(msg.coin_data, coin, COIN_DATA_SIZE * sizeof(u32_t));
    msg.worker_rank = worker_rank;
    MPI_Send(&msg, sizeof(coin_message_t), MPI_BYTE, MPI_MASTER_RANK, TAG_COIN_FOUND, MPI_COMM_WORLD);
}

static inline int check_and_send_coins_avx2_mpi(v8si coin[14], v8si hash[5], int worker_rank)
{
    __m256i target = _mm256_set1_epi32(0xAAD20250u);
    __m256i hash0_vec = (__m256i)hash[0];
    __m256i cmp = _mm256_cmpeq_epi32(hash0_vec, target);
    int mask = _mm256_movemask_epi8(cmp);
    int found = 0;

    if (__builtin_expect(mask == 0, 1))
        return 0;

    u32_t *hash_data = (u32_t *)&hash[0];
    for (int lane = 0; lane < 8; lane++) {
        if (hash_data[lane] == 0xAAD20250u) {
            u32_t coin_scalar[14] __attribute__((aligned(16)));
            for (int i = 0; i < 14; i++) {
                u32_t *coin_data = (u32_t *)&coin[i];
                coin_scalar[i] = coin_data[lane];
            }

            if (validate_coin_mpi(coin_scalar)) {
                send_coin_to_master(coin_scalar, worker_rank);
                found++;
            }
        }
    }
    return found;
}

static inline int check_stop_signal(void)
{
    int flag = 0;
    MPI_Status status;
    MPI_Iprobe(MPI_MASTER_RANK, TAG_STOP, MPI_COMM_WORLD, &flag, &status);
    if (flag) {
        int dummy;
        MPI_Recv(&dummy, 1, MPI_INT, MPI_MASTER_RANK, TAG_STOP, MPI_COMM_WORLD, &status);
        return 1;
    }
    return 0;
}

static inline void request_more_work(work_range_t *range, int worker_rank)
{
    int request = worker_rank;
    MPI_Send(&request, 1, MPI_INT, MPI_MASTER_RANK, TAG_REQUEST_WORK, MPI_COMM_WORLD);
    MPI_Recv(range, sizeof(work_range_t), MPI_BYTE, MPI_MASTER_RANK, TAG_WORK_ASSIGN, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

static inline void send_stats_to_master(u64_t hashes, int coins, int worker_rank)
{
    stats_message_t stats;
    stats.hashes_done = hashes;
    stats.coins_found = coins;
    stats.worker_rank = worker_rank;
    MPI_Send(&stats, sizeof(stats_message_t), MPI_BYTE, MPI_MASTER_RANK, TAG_STATS_UPDATE, MPI_COMM_WORLD);
}

static inline void run_worker(int worker_rank, int num_workers)
{
    (void)num_workers;
    signal(SIGINT, SIG_IGN);

    work_range_t work;
    volatile u64_t total_hashes = 0;
    volatile int total_coins = 0;
    time_t last_stats = time(NULL);
    time_t last_check = last_stats;

    MPI_Recv(&work, sizeof(work_range_t), MPI_BYTE, MPI_MASTER_RANK, TAG_WORK_ASSIGN, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();
        v8si coin[14] __attribute__((aligned(32)));
        v8si hash[5] __attribute__((aligned(32)));

        u64_t range_size = work.end_counter - work.start_counter;
        u64_t chunk_size = range_size / num_threads;
        u64_t my_start = work.start_counter + thread_id * chunk_size;
        u64_t my_end = (thread_id == num_threads - 1) ? work.end_counter : my_start + chunk_size;
        u64_t counter = my_start;
        u64_t local_hashes = 0;

        init_coin_data_avx2_mpi(coin);

        while (!worker_stop_signal) {
            if (counter >= my_end) {
                #pragma omp barrier
                #pragma omp single
                {
                    if (check_stop_signal()) {
                        worker_stop_signal = 1;
                    } else {
                        request_more_work(&work, worker_rank);
                        if (work.start_counter == 0 && work.end_counter == 0) {
                            worker_stop_signal = 1;
                        }
                    }
                }
                #pragma omp barrier

                if (worker_stop_signal) break;
                range_size = work.end_counter - work.start_counter;
                chunk_size = range_size / num_threads;
                my_start = work.start_counter + thread_id * chunk_size;
                my_end = (thread_id == num_threads - 1) ? work.end_counter : my_start + chunk_size;
                counter = my_start;
            }

            update_counters_avx2_mpi(coin, counter);
            sha1_avx2(coin, hash);

            int found = check_and_send_coins_avx2_mpi(coin, hash, worker_rank);
            if (found > 0) {
                #pragma omp atomic
                total_coins += found;
            }

            counter += 8;
            local_hashes += 8;
            if (__builtin_expect((local_hashes & 0xFFFFF) == 0, 0)) {
                #pragma omp atomic
                total_hashes += 0x100000;
                if (thread_id == 0) {
                    time_t now = time(NULL);
                    if (difftime(now, last_check) >= 1.0) {
                        if (check_stop_signal()) {
                            worker_stop_signal = 1;
                        }
                        last_check = now;
                    }
                    if (difftime(now, last_stats) >= 2.0) {
                        send_stats_to_master(total_hashes, total_coins, worker_rank);
                        last_stats = now;
                    }
                }
            }
        }
        u64_t remainder = local_hashes & 0xFFFFF;
        if (remainder > 0) {
            #pragma omp atomic
            total_hashes += remainder;
        }
    }
    send_stats_to_master(total_hashes, total_coins, worker_rank);
    int done = worker_rank;
    MPI_Send(&done, 1, MPI_INT, MPI_MASTER_RANK, TAG_WORKER_DONE, MPI_COMM_WORLD);
}

#endif
