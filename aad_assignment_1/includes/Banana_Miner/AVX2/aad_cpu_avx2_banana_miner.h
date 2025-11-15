#ifndef AAD_CPU_AVX2_BANANA_MINER_H
#define AAD_CPU_AVX2_BANANA_MINER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <stdint.h>
#include <immintrin.h>

#include "../aad_data_types.h"
#include "../aad_sha1_cpu.h"
#include "../aad_vault.h"
#include "../aad_utilities.h"

static volatile int stop_signal = 0;
static volatile int coins_found = 0;

//
// Initialize banana coin data in AVX2 v8si format
// Format: "DETI coin 2 TOSbananaCOIN [counter]...\n"
//
static inline void init_coin_data_avx2(v8si coin[14]) {
    // "DETI coin 2 " - same for all 8 lanes
    coin[0] = (v8si){0x44455449u, 0x44455449u, 0x44455449u, 0x44455449u,
                     0x44455449u, 0x44455449u, 0x44455449u, 0x44455449u};
    coin[1] = (v8si){0x20636F69u, 0x20636F69u, 0x20636F69u, 0x20636F69u,
                     0x20636F69u, 0x20636F69u, 0x20636F69u, 0x20636F69u};
    coin[2] = (v8si){0x6E203220u, 0x6E203220u, 0x6E203220u, 0x6E203220u,
                     0x6E203220u, 0x6E203220u, 0x6E203220u, 0x6E203220u};
    
    // "TOSbananaCOIN " - banana signature (words 3-6)
    coin[3] = (v8si){0x544F5362u, 0x544F5362u, 0x544F5362u, 0x544F5362u,
                     0x544F5362u, 0x544F5362u, 0x544F5362u, 0x544F5362u}; // "TOSb"
    coin[4] = (v8si){0x616E616Eu, 0x616E616Eu, 0x616E616Eu, 0x616E616Eu,
                     0x616E616Eu, 0x616E616Eu, 0x616E616Eu, 0x616E616Eu}; // "anan"
    coin[5] = (v8si){0x61434F49u, 0x61434F49u, 0x61434F49u, 0x61434F49u,
                     0x61434F49u, 0x61434F49u, 0x61434F49u, 0x61434F49u}; // "aCOI"
    coin[6] = (v8si){0x4E202020u, 0x4E202020u, 0x4E202020u, 0x4E202020u,
                     0x4E202020u, 0x4E202020u, 0x4E202020u, 0x4E202020u}; // "N   "
    
    // Zeros for remaining fixed words
    coin[10] = (v8si){0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
    coin[11] = (v8si){0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
    coin[12] = (v8si){0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
}

//
// Update counters for 8 lanes
//
static inline void update_counters_avx2(v8si coin[14], u64_t counter) {
    // Different counter for each of 8 lanes
    u32_t base_counters[8];
    for (int i = 0; i < 8; i++) {
        base_counters[i] = (u32_t)((counter + i) & 0xFFFFFFFFu);
    }
    
    coin[7] = (v8si){base_counters[0], base_counters[1], base_counters[2], base_counters[3],
                     base_counters[4], base_counters[5], base_counters[6], base_counters[7]};
    
    // High bits of counter (same for all lanes in same batch)
    u32_t counter_high = (u32_t)((counter >> 32) & 0xFFFFFFFFu);
    coin[8] = (v8si){counter_high, counter_high, counter_high, counter_high,
                     counter_high, counter_high, counter_high, counter_high};
    
    // Timestamp (same for all lanes)
    u32_t time_seed = (u32_t)time(NULL);
    coin[9] = (v8si){time_seed, time_seed, time_seed, time_seed,
                     time_seed, time_seed, time_seed, time_seed};
    
    // SHA-1 padding
    coin[13] = (v8si){0x00000A80u, 0x00000A80u, 0x00000A80u, 0x00000A80u,
                      0x00000A80u, 0x00000A80u, 0x00000A80u, 0x00000A80u};
}

//
// Check and save banana coins from AVX2 lanes
//
static inline void check_and_save_coins_avx2(v8si coin[14], v8si hash[5], u64_t counter) {
    // Quick SIMD check
    __m256i target = _mm256_set1_epi32(0xAAD20250u);
    __m256i hash0_vec = (__m256i)hash[0];
    __m256i cmp = _mm256_cmpeq_epi32(hash0_vec, target);
    int mask = _mm256_movemask_epi8(cmp);
    
    if (__builtin_expect(mask == 0, 1)) {
        return;  // No coins found in any lane
    }
    
    // Extract and validate individual lanes
    u32_t *hash_data = (u32_t *)&hash[0];
    for (int lane = 0; lane < 8; lane++) {
        if (hash_data[lane] == 0xAAD20250u) {
            // Extract coin from this lane
            u32_t coin_scalar[14] __attribute__((aligned(16)));
            for (int i = 0; i < 14; i++) {
                u32_t *coin_data = (u32_t *)&coin[i];
                coin_scalar[i] = coin_data[lane];
            }
            
            // Validate no '\n' in variable part
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
                printf("\n🍌💰 TOS-BANANA-COIN #%d FOUND! (Lane %d) 💰🍌\n", coins_found, lane);
                
                // Print content
                printf("   Content: ");
                for (int i = 0; i < 55; i++) {
                    char c = base_coin[i ^ 3];
                    if (c == '\n') printf("\\n");
                    else if (c >= 32 && c <= 126) printf("%c", c);
                    else printf("\\x%02x", (unsigned char)c);
                }
                printf("\n");
                
                // Print hash
                u32_t *full_hash = (u32_t *)hash;
                printf("   Hash: %08X%08X%08X%08X%08X\n",
                       full_hash[lane], full_hash[8+lane], full_hash[16+lane],
                       full_hash[24+lane], full_hash[32+lane]);
                printf("   Counter: %lu\n\n", counter + lane);
                
                // Save coin
                save_coin(coin_scalar);
            }
        }
    }
}

//
// Main AVX2 mining function
//
static inline void mine_cpu_avx2_coins(void) {
    v8si coin[14] __attribute__((aligned(32)));
    v8si hash[5] __attribute__((aligned(32)));
    u64_t counter = 0;
    time_t start, last_print;
    double elapsed;
    
    init_coin_data_avx2(coin);
    start = time(NULL);
    last_print = start;
    
    printf("🍌 Starting TOS-BANANA-COIN mining (AVX2 8-way)...\n");
    printf("   Target: DETI coin 2 TOSbananaCOIN ...\n\n");
    
    while (!stop_signal) {
        update_counters_avx2(coin, counter);
        sha1_avx2(coin, hash);
        check_and_save_coins_avx2(coin, hash, counter);
        
        counter += 8;
        
        // Progress report every ~8M attempts
        if (__builtin_expect((counter & 0xFFFFFF) == 0, 0)) {
            time_t now = time(NULL);
            if (difftime(now, last_print) >= 5.0) {
                elapsed = difftime(now, start);
                printf("[%.0fs] %luM @ %.2fM/s | 🍌 TOS-Banana Coins:%d\n",
                       elapsed, counter/1000000UL,
                       (elapsed > 0 ? counter/elapsed/1e6 : 0),
                       coins_found);
                last_print = now;
            }
        }
    }
    
    // Final statistics
    elapsed = difftime(time(NULL), start);
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║    🍌 TOS-BANANA-COIN MINER - FINAL STATS (AVX2) 🍌      ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║ Total attempts: %-37lu ║\n", counter);
    printf("║ Time: %.2f seconds%-26s║\n", elapsed, "");
    printf("║ Average rate: %.2f M/s%-30s║\n", counter/elapsed/1e6, "");
    printf("║ 🍌 TOS-Banana coins found: %-24d ║\n", coins_found);
    printf("║ AVX2 lanes: 8%-42s║\n", "");
    printf("╚════════════════════════════════════════════════════════════╝\n");
}

#endif
