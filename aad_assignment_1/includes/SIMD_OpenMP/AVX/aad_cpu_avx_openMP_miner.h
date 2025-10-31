#ifndef AAD_CPU_AVX_OPENMP_MINER_H
#define AAD_CPU_AVX_OPENMP_MINER_H

#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>
#include <immintrin.h>
#include "../aad_data_types.h"
#include "../aad_sha1_cpu.h"
#include "../aad_vault.h"
#include "../aad_utilities.h"

static volatile int stop_signal = 0;
static volatile int coins_found = 0;

static inline void init_coin_data_avx(v4si coin[14]) {
    coin[0] = (v4si){0x44455449u, 0x44455449u, 0x44455449u, 0x44455449u};
    coin[1] = (v4si){0x20636F69u, 0x20636F69u, 0x20636F69u, 0x20636F69u};
    coin[2] = (v4si){0x6E203220u, 0x6E203220u, 0x6E203220u, 0x6E203220u};
    coin[6] = (v4si){0u, 0u, 0u, 0u};
    coin[7] = (v4si){0u, 0u, 0u, 0u};
    coin[8] = (v4si){0u, 0u, 0u, 0u};
    coin[9] = (v4si){0u, 0u, 0u, 0u};
    coin[10] = (v4si){0u, 0u, 0u, 0u};
    coin[11] = (v4si){0u, 0u, 0u, 0u};
    coin[12] = (v4si){0u, 0u, 0u, 0u};
}

static inline void update_counters_avx(v4si coin[14], u64_t counter) {
    u32_t base_counters[4];
    for (int i = 0; i < 4; i++) {
        base_counters[i] = (u32_t)((counter + i) & 0xFFFFFFFFu);
    }

    coin[3] = (v4si){base_counters[0], base_counters[1], base_counters[2], base_counters[3]};

    u32_t counter_high = (u32_t)((counter >> 32) & 0xFFFFFFFFu);
    coin[4] = (v4si){counter_high, counter_high, counter_high, counter_high};

    u32_t time_seed = (u32_t)time(NULL);
    coin[5] = (v4si){time_seed, time_seed, time_seed, time_seed};

    coin[13] = (v4si){0x00000A80u, 0x00000A80u, 0x00000A80u, 0x00000A80u};
}

static inline void check_and_save_coins_avx(v4si coin[14], v4si hash[5]) {
    __m128i target = _mm_set1_epi32(0xAAD20250u);
    __m128i hash0_vec = (__m128i)hash[0];
    __m128i cmp = _mm_cmpeq_epi32(hash0_vec, target);
    int mask = _mm_movemask_epi8(cmp);

    if (__builtin_expect(mask == 0, 1)) {
        return;
    }

    u32_t *hash_data = (u32_t *)&hash[0];
    for (int lane = 0; lane < 4; lane++) {
        if (hash_data[lane] == 0xAAD20250u) {
            u32_t coin_scalar[14] __attribute__((aligned(16)));
            for (int i = 0; i < 14; i++) {
                u32_t *coin_data = (u32_t *)&coin[i];
                coin_scalar[i] = coin_data[lane];
            }

            u08_t *base_coin = (u08_t *)coin_scalar;
            int valid = 1;

            for (int i = 12; i < 54; i++) {
                if (base_coin[i ^ 3] == '\n') {
                    valid = 0;
                    break;
                }
            }

            if (valid) {
                #pragma omp atomic
                coins_found++;

                #pragma omp critical
                {
                    printf("\n💰 COIN #%d (Thread %d, Lane %d)\n", 
                           coins_found, omp_get_thread_num(), lane);
                    save_coin(coin_scalar);
                }
            }
        }
    }
}

static inline void mine_cpu_avx_coins_openmp(void) {
    int num_threads = omp_get_max_threads();
    printf("🚀 Starting OpenMP mining with %d threads\n", num_threads);
    printf("============================================================\n");

    time_t start = time(NULL);
    u64_t total_attempts = 0;

    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        v4si coin[14] __attribute__((aligned(32)));
        v4si hash[5] __attribute__((aligned(32)));
        u64_t local_counter = 0;
        u64_t thread_offset = (u64_t)thread_id * 1000000000ULL;
        time_t last_print = start;

        init_coin_data_avx(coin);

        while (!stop_signal) {
            u64_t counter = thread_offset + local_counter;

            update_counters_avx(coin, counter);
            sha1_avx(coin, hash);
            check_and_save_coins_avx(coin, hash);

            local_counter += 4;

            // Update shared counter periodically
            if (__builtin_expect((local_counter & 0xFFFFF) == 0, 0)) {
                #pragma omp atomic
                total_attempts += 0x100000;
            }

            // Progress reporting (master thread only)
            if (thread_id == 0 && __builtin_expect((local_counter & 0xFFFFF) == 0, 0)) {
                time_t now = time(NULL);
                double elapsed = difftime(now, start);

                if (elapsed >= 5.0 && difftime(now, last_print) >= 5.0) {
                    u64_t snapshot;
                    #pragma omp atomic read
                    snapshot = total_attempts;

                    double hash_rate = snapshot / elapsed / 1e6;

                    printf("[%ds] %lu M @ %.2f M/s | Coins: %d | Threads: %d\n",
                           (int)elapsed,
                           snapshot / 1000000UL,
                           hash_rate,
                           coins_found,
                           num_threads);
                    last_print = now;
                }
            }
        }

        // Final accumulation
        u64_t remainder = local_counter & 0xFFFFF;
        if (remainder > 0) {
            #pragma omp atomic
            total_attempts += remainder;
        }
    }

    time_t end = time(NULL);
    double elapsed = difftime(end, start);
    double final_rate = total_attempts / elapsed / 1e6;

    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║              OPENMP AVX FINAL STATISTICS                   ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║ Threads:         %-37d ║\n", num_threads);
    printf("║ Total attempts:  %-37lu ║\n", total_attempts);
    printf("║ Time:            %.2f seconds%-26s║\n", elapsed, "");
    printf("║ Average rate:    %.2f M/s%-30s║\n", final_rate, "");
    printf("║ Coins found:     %-37d ║\n", coins_found);
    printf("╚════════════════════════════════════════════════════════════╝\n");
}

#endif