#ifndef AAD_CPU_AVX2_MINER_H
#define AAD_CPU_AVX2_MINER_H

#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <immintrin.h>
#include "../aad_data_types.h"
#include "../aad_sha1_cpu.h"
#include "../aad_vault.h"
#include "../aad_utilities.h"

static volatile int stop_signal = 0;
static volatile int coins_found = 0;

static inline void init_coin_data_avx2(v8si coin[14]) {
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

static inline void update_counters_avx2(v8si coin[14], u64_t counter) {
    u32_t base_counters[8];
    for (int i = 0; i < 8; i++) {
        base_counters[i] = (u32_t)((counter + i) & 0xFFFFFFFFu);
    }

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

static inline void check_and_save_coins_avx2(v8si coin[14], v8si hash[5]) {
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
                coins_found++;
                printf("\n💰 COIN #%d (Lane %d)\n", coins_found, lane);
                save_coin(coin_scalar);
            }
        }
    }
}

static inline void mine_cpu_avx2_coins(void) {
    v8si coin[14] __attribute__((aligned(32)));
    v8si hash[5] __attribute__((aligned(32)));
    u64_t counter = 0;
    time_t start, last_print;
    double elapsed;

    init_coin_data_avx2(coin);

    start = time(NULL);
    last_print = start;

    while (!stop_signal) {
        update_counters_avx2(coin, counter);
        sha1_avx2(coin, hash);
        check_and_save_coins_avx2(coin, hash);

        counter += 8;

        if (__builtin_expect((counter & 0xFFFFFF) == 0, 0)) {
            time_t now = time(NULL);
            if (difftime(now, last_print) >= 5.0) {
                elapsed = difftime(now, start);
                printf("[%.0fs] %luM @ %.2fM/s | Coins:%d\n",
                       elapsed, counter/1000000UL,
                       (elapsed > 0 ? counter/elapsed/1e6 : 0),
                       coins_found);
                last_print = now;
            }
        }
    }

    elapsed = difftime(time(NULL), start);
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║                      FINAL STATISTICS                      ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║ Total attempts:  %-37lu ║\n", counter);
    printf("║ Time:            %.2f seconds%-26s║\n", elapsed, "");
    printf("║ Average rate:    %.2f M/s%-30s║\n", counter/elapsed/1e6, "");
    printf("║ Coins found:     %-37d ║\n", coins_found);
    printf("╚════════════════════════════════════════════════════════════╝\n");
}

#endif