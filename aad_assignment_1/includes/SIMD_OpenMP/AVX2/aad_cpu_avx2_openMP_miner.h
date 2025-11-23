#ifndef AAD_CPU_AVX2_OPENMP_MINER_H
#define AAD_CPU_AVX2_OPENMP_MINER_H

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
#include "../aad_coin_types.h"

static volatile int stop_signal = 0;
static volatile int coins_found = 0;

//
// Initialize coin data with optional custom text (broadcast to all 8 lanes)
//
static inline void init_coin_data_avx2(v8si coin[14], const coin_config_t *config) {
    // Common prefix (words 0-2): broadcast to all lanes
    coin[0] = (v8si){0x44455449u, 0x44455449u, 0x44455449u, 0x44455449u,
                     0x44455449u, 0x44455449u, 0x44455449u, 0x44455449u};
    coin[1] = (v8si){0x20636F69u, 0x20636F69u, 0x20636F69u, 0x20636F69u,
                     0x20636F69u, 0x20636F69u, 0x20636F69u, 0x20636F69u};
    coin[2] = (v8si){0x6E203220u, 0x6E203220u, 0x6E203220u, 0x6E203220u,
                     0x6E203220u, 0x6E203220u, 0x6E203220u, 0x6E203220u};

    // Zero all remaining words
    for (int i = 3; i < 13; i++) {
        coin[i] = (v8si){0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
    }

    // If CUSTOM type, encode custom text into scalar coin then broadcast
    if (config->type == COIN_TYPE_CUSTOM && config->custom_text != NULL) {
        u32_t temp_coin[14] = {0};  // Initialize to zeros
        encode_custom_text(temp_coin, config->custom_text, 5);

        // Broadcast custom text words to all lanes (starting at word 5)
        int word_idx = 5;
        while (word_idx < 13 && temp_coin[word_idx] != 0) {
            coin[word_idx] = (v8si){temp_coin[word_idx], temp_coin[word_idx],
                                     temp_coin[word_idx], temp_coin[word_idx],
                                     temp_coin[word_idx], temp_coin[word_idx],
                                     temp_coin[word_idx], temp_coin[word_idx]};
            word_idx++;
        }
    }
}

//
// Update counters for all 8 lanes
//
static inline void update_counters_avx2(v8si coin[14], u64_t counter, const coin_config_t *config) {
    // Counter low (word 3): different for each lane
    u32_t base_counters[8];
    for (int i = 0; i < 8; i++) {
        base_counters[i] = (u32_t)((counter + i) & 0xFFFFFFFFu);
    }
    coin[3] = (v8si){base_counters[0], base_counters[1], base_counters[2], base_counters[3],
                     base_counters[4], base_counters[5], base_counters[6], base_counters[7]};

    // Counter high (word 4): same for all lanes
    u32_t counter_high = (u32_t)((counter >> 32) & 0xFFFFFFFFu);
    coin[4] = (v8si){counter_high, counter_high, counter_high, counter_high,
                     counter_high, counter_high, counter_high, counter_high};

    // Calculate timestamp position based on custom text
    int timestamp_word = 5;
    if (config->type == COIN_TYPE_CUSTOM && config->custom_text != NULL) {
        size_t text_len = strlen(config->custom_text);
        timestamp_word = 5 + ((text_len + 3) / 4);
    }

    // Timestamp: broadcast to all lanes
    u32_t time_seed = (u32_t)time(NULL);
    coin[timestamp_word] = (v8si){time_seed, time_seed, time_seed, time_seed,
                                   time_seed, time_seed, time_seed, time_seed};

    // Zero remaining words after timestamp
    for (int i = timestamp_word + 1; i < 13; i++) {
        coin[i] = (v8si){0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
    }

    // SHA-1 padding (word 13): always last
    coin[13] = (v8si){0x00000A80u, 0x00000A80u, 0x00000A80u, 0x00000A80u,
                      0x00000A80u, 0x00000A80u, 0x00000A80u, 0x00000A80u};
}

static inline void check_and_save_coins_avx2(v8si coin[14], v8si hash[5], const coin_config_t *config) {
    __m256i target = _mm256_set1_epi32(0xAAD20250u);
    __m256i hash0_vec = (__m256i)hash[0];
    __m256i cmp = _mm256_cmpeq_epi32(hash0_vec, target);
    int mask = _mm256_movemask_epi8(cmp);

    if (__builtin_expect(mask == 0, 1)) {
        return;
    }

    u32_t *hash_data = (u32_t *)&hash[0];
    for (int lane = 0; lane < 8; lane++) {
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
                    printf("\n%s COIN #%d (Thread %d, Lane %d)\n",
                           (config->type == COIN_TYPE_CUSTOM ? "[+]" : "[*]"), coins_found, omp_get_thread_num(), lane);
                    save_coin(coin_scalar);
                }
            }
        }
    }
}

static inline void mine_cpu_avx2_coins_openmp(const coin_config_t *config) {
    int num_threads = omp_get_max_threads();

    // Print startup message
    if (config->type == COIN_TYPE_CUSTOM) {
        printf("[+] Starting CUSTOM coin mining (AVX2 OpenMP, %d threads)...\n", num_threads);
        printf("   Custom text: \"%s\"\n", config->custom_text);
    } else {
        printf("[*] Starting DETI coin mining (AVX2 OpenMP, %d threads)...\n", num_threads);
    }
    printf("============================================================\n");

    time_t start = time(NULL);
    u64_t total_attempts = 0;

    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        v8si coin[14] __attribute__((aligned(32)));
        v8si hash[5] __attribute__((aligned(32)));
        u64_t local_counter = 0;
        u64_t thread_offset = (u64_t)thread_id * 1000000000ULL;
        time_t last_print = start;

        init_coin_data_avx2(coin, config);

        while (!stop_signal) {
            u64_t counter = thread_offset + local_counter;

            update_counters_avx2(coin, counter, config);
            sha1_avx2(coin, hash);
            check_and_save_coins_avx2(coin, hash, config);

            local_counter += 8;

            // Update shared counter periodically (every 1M hashes)
            if (__builtin_expect((local_counter & 0xFFFFF) == 0, 0)) {
                #pragma omp atomic
                total_attempts += 0x100000;
            }

            // Progress reporting (master thread only every 5 seconds)
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

        // Final accumulation of remaining hashes
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
    printf("║              OPENMP AVX2 FINAL STATISTICS                  ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║ Threads:         %-37d ║\n", num_threads);
    printf("║ Total attempts:  %-37lu ║\n", total_attempts);
    printf("║ Time:            %.2f seconds%-26s║\n", elapsed, "");
    printf("║ Average rate:    %.2f M/s%-30s║\n", final_rate, "");
    printf("║ Coins found:     %-37d ║\n", coins_found);
    printf("╚════════════════════════════════════════════════════════════╝\n");
}

#endif