#ifndef AAD_CPU_AVX512_BANANA_MINER_H
#define AAD_CPU_AVX512_BANANA_MINER_H

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
// Initialize banana coin data in AVX-512 v16si format
// Format: "DETI coin 2 TOSbananaCOIN [counter]...\n"
//
static inline void init_coin_data_avx512(v16si coin[14]) {
    // "DETI coin 2 " - all 16 lanes identical
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
    
    // "TOSbananaCOIN " - banana signature (words 3-6)
    coin[3] = (v16si){0x544F5362u, 0x544F5362u, 0x544F5362u, 0x544F5362u,
                      0x544F5362u, 0x544F5362u, 0x544F5362u, 0x544F5362u,
                      0x544F5362u, 0x544F5362u, 0x544F5362u, 0x544F5362u,
                      0x544F5362u, 0x544F5362u, 0x544F5362u, 0x544F5362u}; // "TOSb"
    
    coin[4] = (v16si){0x616E616Eu, 0x616E616Eu, 0x616E616Eu, 0x616E616Eu,
                      0x616E616Eu, 0x616E616Eu, 0x616E616Eu, 0x616E616Eu,
                      0x616E616Eu, 0x616E616Eu, 0x616E616Eu, 0x616E616Eu,
                      0x616E616Eu, 0x616E616Eu, 0x616E616Eu, 0x616E616Eu}; // "anan"
    
    coin[5] = (v16si){0x61434F49u, 0x61434F49u, 0x61434F49u, 0x61434F49u,
                      0x61434F49u, 0x61434F49u, 0x61434F49u, 0x61434F49u,
                      0x61434F49u, 0x61434F49u, 0x61434F49u, 0x61434F49u,
                      0x61434F49u, 0x61434F49u, 0x61434F49u, 0x61434F49u}; // "aCOI"
    
    coin[6] = (v16si){0x4E202020u, 0x4E202020u, 0x4E202020u, 0x4E202020u,
                      0x4E202020u, 0x4E202020u, 0x4E202020u, 0x4E202020u,
                      0x4E202020u, 0x4E202020u, 0x4E202020u, 0x4E202020u,
                      0x4E202020u, 0x4E202020u, 0x4E202020u, 0x4E202020u}; // "N   "
    
    // Zeros for remaining fixed words
    coin[10] = (v16si){0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
    coin[11] = (v16si){0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
    coin[12] = (v16si){0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
}

//
// Update counters for 16 lanes
//
static inline void update_counters_avx512(v16si coin[14], u64_t counter) {
    // Different counter for each of 16 lanes
    u32_t base_counters[16];
    for (int i = 0; i < 16; i++) {
        base_counters[i] = (u32_t)((counter + i) & 0xFFFFFFFFu);
    }
    
    coin[7] = (v16si){base_counters[0], base_counters[1], base_counters[2], base_counters[3],
                      base_counters[4], base_counters[5], base_counters[6], base_counters[7],
                      base_counters[8], base_counters[9], base_counters[10], base_counters[11],
                      base_counters[12], base_counters[13], base_counters[14], base_counters[15]};
    
    // High bits of counter (same for all lanes in same batch)
    u32_t counter_high = (u32_t)((counter >> 32) & 0xFFFFFFFFu);
    coin[8] = (v16si){counter_high, counter_high, counter_high, counter_high,
                      counter_high, counter_high, counter_high, counter_high,
                      counter_high, counter_high, counter_high, counter_high,
                      counter_high, counter_high, counter_high, counter_high};
    
    // Timestamp (same for all lanes)
    u32_t time_seed = (u32_t)time(NULL);
    coin[9] = (v16si){time_seed, time_seed, time_seed, time_seed,
                      time_seed, time_seed, time_seed, time_seed,
                      time_seed, time_seed, time_seed, time_seed,
                      time_seed, time_seed, time_seed, time_seed};
    
    // SHA-1 padding
    coin[13] = (v16si){0x00000A80u, 0x00000A80u, 0x00000A80u, 0x00000A80u,
                       0x00000A80u, 0x00000A80u, 0x00000A80u, 0x00000A80u,
                       0x00000A80u, 0x00000A80u, 0x00000A80u, 0x00000A80u,
                       0x00000A80u, 0x00000A80u, 0x00000A80u, 0x00000A80u};
}

//
// Check and save banana coins from AVX-512 lanes
//
static inline void check_and_save_coins_avx512(v16si coin[14], v16si hash[5], u64_t counter) {
    // Quick SIMD check using AVX-512 mask
    __m512i target = _mm512_set1_epi32(0xAAD20250u);
    __m512i hash0_vec = (__m512i)hash[0];
    __mmask16 cmp = _mm512_cmpeq_epi32_mask(hash0_vec, target);
    
    if (__builtin_expect(cmp == 0, 1)) {
        return;  // No coins found in any lane
    }
    
    // Extract and validate individual lanes
    u32_t *hash_data = (u32_t *)&hash[0];
    for (int lane = 0; lane < 16; lane++) {
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
                       full_hash[lane], full_hash[16+lane], full_hash[32+lane],
                       full_hash[48+lane], full_hash[64+lane]);
                printf("   Counter: %lu\n\n", counter + lane);
                
                // Save coin
                save_coin(coin_scalar);
            }
        }
    }
}

//
// Main AVX-512 mining function
//
static inline void mine_cpu_avx512_coins(void) {
    v16si coin[14] __attribute__((aligned(64)));
    v16si hash[5] __attribute__((aligned(64)));
    u64_t counter = 0;
    time_t start, last_print;
    double elapsed;
    
    init_coin_data_avx512(coin);
    start = time(NULL);
    last_print = start;
    
    printf("🍌 Starting TOS-BANANA-COIN mining (AVX-512 16-way)...\n");
    printf("   Target: DETI coin 2 TOSbananaCOIN ...\n\n");
    
    while (!stop_signal) {
        update_counters_avx512(coin, counter);
        sha1_avx512f(coin, hash);
        check_and_save_coins_avx512(coin, hash, counter);
        
        counter += 16;
        
        // Progress report every ~16M attempts
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
    printf("║   🍌 TOS-BANANA-COIN MINER - FINAL STATS (AVX-512) 🍌    ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║ Total attempts: %-37lu ║\n", counter);
    printf("║ Time: %.2f seconds%-26s║\n", elapsed, "");
    printf("║ Average rate: %.2f M/s%-30s║\n", counter/elapsed/1e6, "");
    printf("║ 🍌 TOS-Banana coins found: %-24d ║\n", coins_found);
    printf("║ AVX-512 lanes: 16%-39s║\n", "");
    printf("╚════════════════════════════════════════════════════════════╝\n");
}

#endif
