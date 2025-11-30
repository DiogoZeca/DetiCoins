#ifndef AAD_CPU_AVX512_OPENMP_MINER_H
#define AAD_CPU_AVX512_OPENMP_MINER_H

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

// optional custom text 
static inline void init_coin_data_avx512(v16si coin[14], const coin_config_t *config) {
    coin[0] = (v16si){0x44455449u, 0x44455449u, 0x44455449u, 0x44455449u,
                      0x44455449u, 0x44455449u, 0x44455449u, 0x44455449u,
                      0x44455449u, 0x44455449u, 0x44455449u, 0x44455449u,
                      0x44455449u, 0x44455449u, 0x44455449u, 0x44455449u};
    coin[1] = (v16si){0x20636F69u, 0x20636F69u, 0x20636F69u, 0x20636F69u,
                      0x20636F69u, 0x20636F69u, 0x20636F69u, 0x20636F69u,
                      0x20636F69u, 0x20636F69u, 0x20636F69u, 0x20636F69u,
                      0x20636F69u, 0x20636F69u, 0x20636F69u, 0x20636F69u};
    coin[2] = (v16si){0x6E203220u, 0x6E203220u, 0x6E203220u, 0x6E203220u,
                      0x6E203220u, 0x6E203220u, 0x6E203220u, 0x6E203220u,
                      0x6E203220u, 0x6E203220u, 0x6E203220u, 0x6E203220u,
                      0x6E203220u, 0x6E203220u, 0x6E203220u, 0x6E203220u};

    for (int i = 3; i < 13; i++) {
        coin[i] = (v16si){0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
    }

    // If CUSTOM 
    if (config->type == COIN_TYPE_CUSTOM && config->custom_text != NULL) {
        u32_t temp_coin[14] = {0};  // Initialize to zeros
        encode_custom_text(temp_coin, config->custom_text, 5);

        int word_idx = 5;
        while (word_idx < 13 && temp_coin[word_idx] != 0) {
            coin[word_idx] = (v16si){temp_coin[word_idx], temp_coin[word_idx],
                                     temp_coin[word_idx], temp_coin[word_idx],
                                     temp_coin[word_idx], temp_coin[word_idx],
                                     temp_coin[word_idx], temp_coin[word_idx],
                                     temp_coin[word_idx], temp_coin[word_idx],
                                     temp_coin[word_idx], temp_coin[word_idx],
                                     temp_coin[word_idx], temp_coin[word_idx],
                                     temp_coin[word_idx], temp_coin[word_idx]};
            word_idx++;
        }
    }
}

static inline void update_counters_avx512(v16si coin[14], u64_t counter, const coin_config_t *config) {
    // Counter low 
    u32_t base_counters[16];
    for (int i = 0; i < 16; i++) {
        base_counters[i] = (u32_t)((counter + i) & 0xFFFFFFFFu);
    }
    coin[3] = (v16si){base_counters[0], base_counters[1], base_counters[2], base_counters[3],
                      base_counters[4], base_counters[5], base_counters[6], base_counters[7],
                      base_counters[8], base_counters[9], base_counters[10], base_counters[11],
                      base_counters[12], base_counters[13], base_counters[14], base_counters[15]};

    // Counter high 
    u32_t counter_high = (u32_t)((counter >> 32) & 0xFFFFFFFFu);
    coin[4] = (v16si){counter_high, counter_high, counter_high, counter_high,
                      counter_high, counter_high, counter_high, counter_high,
                      counter_high, counter_high, counter_high, counter_high,
                      counter_high, counter_high, counter_high, counter_high};

    int timestamp_word = 5;
    if (config->type == COIN_TYPE_CUSTOM && config->custom_text != NULL) {
        size_t text_len = strlen(config->custom_text);
        timestamp_word = 5 + ((text_len + 3) / 4);
    }
    u32_t time_seed = (u32_t)time(NULL);
    coin[timestamp_word] = (v16si){time_seed, time_seed, time_seed, time_seed,
                                     time_seed, time_seed, time_seed, time_seed,
                                     time_seed, time_seed, time_seed, time_seed,
                                     time_seed, time_seed, time_seed, time_seed};

    for (int i = timestamp_word + 1; i < 13; i++) {
        coin[i] = (v16si){0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
    }

    coin[13] = (v16si){0x00000A80u, 0x00000A80u, 0x00000A80u, 0x00000A80u,
                       0x00000A80u, 0x00000A80u, 0x00000A80u, 0x00000A80u,
                       0x00000A80u, 0x00000A80u, 0x00000A80u, 0x00000A80u,
                       0x00000A80u, 0x00000A80u, 0x00000A80u, 0x00000A80u};
}

static inline void check_and_save_coins_avx512(v16si coin[14], v16si hash[5], const coin_config_t *config) {
    __m512i target = _mm512_set1_epi32(0xAAD20250u);
    __m512i hash0_vec = (__m512i)hash[0];
    __mmask16 cmp = _mm512_cmpeq_epi32_mask(hash0_vec, target);

    if (__builtin_expect(cmp == 0, 1)) {
        return;
    }

    u32_t *hash_data = (u32_t *)&hash[0];
    for (int lane = 0; lane < 16; lane++) {
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

static inline void mine_cpu_avx512_coins_openmp(const coin_config_t *config) {
    int num_threads = omp_get_max_threads();
    // startup message
    if (config->type == COIN_TYPE_CUSTOM) {
        printf("[+] Starting CUSTOM coin mining (AVX512 OpenMP, %d threads)...\n", num_threads);
        printf("   Custom text: \"%s\"\n", config->custom_text);
    } else {
        printf("[*] Starting DETI coin mining (AVX512 OpenMP, %d threads)...\n", num_threads);
    }
    printf("============================================================\n");

    time_t start = time(NULL);
    u64_t total_attempts = 0;

    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        v16si coin[14] __attribute__((aligned(64)));
        v16si hash[5] __attribute__((aligned(64)));
        u64_t local_counter = 0;
        u64_t thread_offset = (u64_t)thread_id * 1000000000ULL;
        time_t last_print = start;

        init_coin_data_avx512(coin, config);

        while (!stop_signal) {
            u64_t counter = thread_offset + local_counter;

            update_counters_avx512(coin, counter, config);
            sha1_avx512f(coin, hash);
            check_and_save_coins_avx512(coin, hash, config);

            local_counter += 16;
            if (__builtin_expect((local_counter & 0xFFFFF) == 0, 0)) {
                #pragma omp atomic
                total_attempts += 0x100000;
            }

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
    printf("║             OPENMP AVX512 FINAL STATISTICS                 ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║ Threads:         %-37d ║\n", num_threads);
    printf("║ Total attempts:  %-37lu ║\n", total_attempts);
    printf("║ Time:            %.2f seconds%-26s║\n", elapsed, "");
    printf("║ Average rate:    %.2f M/s%-30s║\n", final_rate, "");
    printf("║ Coins found:     %-37d ║\n", coins_found);
    printf("╚════════════════════════════════════════════════════════════╝\n");
}

#endif
