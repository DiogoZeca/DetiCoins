#ifndef AAD_CPU_AVX_MINER_H
#define AAD_CPU_AVX_MINER_H

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
#include "../aad_coin_types.h"

static volatile int stop_signal = 0;
static volatile int coins_found = 0;

// custom text
static inline void init_coin_data_avx(v4si coin[14], const coin_config_t *config) {
    coin[0] = (v4si){0x44455449u, 0x44455449u, 0x44455449u, 0x44455449u};
    coin[1] = (v4si){0x20636F69u, 0x20636F69u, 0x20636F69u, 0x20636F69u};
    coin[2] = (v4si){0x6E203220u, 0x6E203220u, 0x6E203220u, 0x6E203220u};
    for (int i = 3; i < 13; i++) {
        coin[i] = (v4si){0u, 0u, 0u, 0u};
    }
    // If CUSTOM 
    if (config->type == COIN_TYPE_CUSTOM && config->custom_text != NULL) {
        u32_t temp_coin[14] = {0};  // Initialize to zeros
        encode_custom_text(temp_coin, config->custom_text, 5);

        int word_idx = 5;
        while (word_idx < 13 && temp_coin[word_idx] != 0) {
            coin[word_idx] = (v4si){temp_coin[word_idx], temp_coin[word_idx],
                                     temp_coin[word_idx], temp_coin[word_idx]};
            word_idx++;
        }
    }
}

static inline void update_counters_avx(v4si coin[14], u64_t counter, const coin_config_t *config) {
    // Counter low 
    u32_t base_counters[4];
    for (int i = 0; i < 4; i++) {
        base_counters[i] = (u32_t)((counter + i) & 0xFFFFFFFFu);
    }
    coin[3] = (v4si){base_counters[0], base_counters[1], base_counters[2], base_counters[3]};

    // Counter high 
    u32_t counter_high = (u32_t)((counter >> 32) & 0xFFFFFFFFu);
    coin[4] = (v4si){counter_high, counter_high, counter_high, counter_high};

    int timestamp_word = 5;
    if (config->type == COIN_TYPE_CUSTOM && config->custom_text != NULL) {
        size_t text_len = strlen(config->custom_text);
        timestamp_word = 5 + ((text_len + 3) / 4);  
    }
    u32_t time_seed = (u32_t)time(NULL);
    coin[timestamp_word] = (v4si){time_seed, time_seed, time_seed, time_seed};
    for (int i = timestamp_word + 1; i < 13; i++) {
        coin[i] = (v4si){0u, 0u, 0u, 0u};
    }
    coin[13] = (v4si){0x00000A80u, 0x00000A80u, 0x00000A80u, 0x00000A80u};
}

static inline void check_and_save_coins_avx(v4si coin[14], v4si hash[5], const coin_config_t *config) {
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
                coins_found++;
                printf("\n%s COIN #%d (Lane %d)\n", (config->type == COIN_TYPE_CUSTOM ? "[+]" : "[*]"), coins_found, lane);
                save_coin(coin_scalar);
            }
        }
    }
}

static inline void mine_cpu_avx_coins(const coin_config_t *config) {
    v4si coin[14] __attribute__((aligned(32)));
    v4si hash[5] __attribute__((aligned(32)));
    u64_t counter = 0;
    time_t start, last_print;
    double elapsed;

    init_coin_data_avx(coin, config);

    start = time(NULL);
    last_print = start;

    // startup message
    if (config->type == COIN_TYPE_CUSTOM) {
        printf("[+] Starting CUSTOM coin mining (AVX)...\n");
        printf("   Custom text: \"%s\"\n\n", config->custom_text);
    } else {
        printf("[*] Starting DETI coin mining (AVX)...\n\n");
    }

    while (!stop_signal) {
        update_counters_avx(coin, counter, config);
        sha1_avx(coin, hash);
        check_and_save_coins_avx(coin, hash, config);

        counter += 4;

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
