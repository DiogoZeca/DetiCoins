#ifndef AAD_CPU_AVX_BANANA_MINER_H
#define AAD_CPU_AVX_BANANA_MINER_H

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

static volatile int stop_signal = 0;
static volatile int coins_found = 0;

//
// Initialize fixed banana coin data in AVX v4si format
//
static inline void init_coin_data_avx(v4si coin[14]) {
    // "DETI coin 2 " - same for all 4 lanes
    coin[0] = (v4si){0x44455449u, 0x44455449u, 0x44455449u, 0x44455449u};
    coin[1] = (v4si){0x20636F69u, 0x20636F69u, 0x20636F69u, 0x20636F69u};
    coin[2] = (v4si){0x6E203220u, 0x6E203220u, 0x6E203220u, 0x6E203220u};
    
    // "TOSbananaCOIN " - banana signature
    coin[3] = (v4si){0x544F5362u, 0x544F5362u, 0x544F5362u, 0x544F5362u}; // "TOSb"
    coin[4] = (v4si){0x616E616Eu, 0x616E616Eu, 0x616E616Eu, 0x616E616Eu}; // "anan"
    coin[5] = (v4si){0x61434F49u, 0x61434F49u, 0x61434F49u, 0x61434F49u}; // "aCOI"
    coin[6] = (v4si){0x4E202020u, 0x4E202020u, 0x4E202020u, 0x4E202020u}; // "N   "
    
    // Zeros for remaining words
    coin[10] = (v4si){0u, 0u, 0u, 0u};
    coin[11] = (v4si){0u, 0u, 0u, 0u};
    coin[12] = (v4si){0u, 0u, 0u, 0u};
}

//
// Update counters for 4 lanes
//
static inline void update_counters_avx(v4si coin[14], u64_t counter) {
    // Different counter for each lane
    u32_t base_counters[4];
    for (int i = 0; i < 4; i++) {
        base_counters[i] = (u32_t)((counter + i) & 0xFFFFFFFFu);
    }
    coin[7] = (v4si){base_counters[0], base_counters[1], base_counters[2], base_counters[3]};
    
    // High bits of counter (same for all lanes in same batch)
    u32_t counter_high = (u32_t)((counter >> 32) & 0xFFFFFFFFu);
    coin[8] = (v4si){counter_high, counter_high, counter_high, counter_high};
    
    // Timestamp (same for all lanes)
    u32_t time_seed = (u32_t)time(NULL);
    coin[9] = (v4si){time_seed, time_seed, time_seed, time_seed};
    
    // SHA-1 padding
    coin[13] = (v4si){0x00000A80u, 0x00000A80u, 0x00000A80u, 0x00000A80u};
}

//
// Check and save coins from AVX lanes
//
static inline void check_and_save_coins_avx(v4si coin[14], v4si hash[5], u64_t counter) {
    // Quick check using SIMD
    __m128i target = _mm_set1_epi32(0xAAD20250u);
    __m128i hash0_vec = (__m128i)hash[0];
    __m128i cmp = _mm_cmpeq_epi32(hash0_vec, target);
    int mask = _mm_movemask_epi8(cmp);
    
    if (__builtin_expect(mask == 0, 1)) {
        return;  // No coins found in any lane
    }
    
    // Extract and check individual lanes
    u32_t *hash_data = (u32_t *)&hash[0];
    for (int lane = 0; lane < 4; lane++) {
        if (hash_data[lane] == 0xAAD20250u) {
            // Extract coin from this lane to scalar format
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
                       full_hash[lane], full_hash[4+lane], full_hash[8+lane],
                       full_hash[12+lane], full_hash[16+lane]);
                printf("   Counter: %lu\n\n", counter + lane);
                
                // Save coin
                save_coin(coin_scalar);
            }
        }
    }
}

//
// Main AVX mining function
//
static inline void mine_cpu_coins(void) {
    v4si coin[14] __attribute__((aligned(32)));
    v4si hash[5] __attribute__((aligned(32)));
    u64_t counter = 0;
    time_t start, last_print;
    double elapsed;
    
    init_coin_data_avx(coin);
    start = time(NULL);
    last_print = start;
    
    printf("🍌 Starting TOS-BANANA-COIN mining (AVX 4-way)...\n");
    printf("   Target: DETI coin 2 TOSbananaCOIN ...\n\n");
    
    while (!stop_signal) {
        update_counters_avx(coin, counter);
        sha1_avx(coin, hash);
        check_and_save_coins_avx(coin, hash, counter);
        
        counter += 4;
        
        // Progress report
        if (__builtin_expect((counter & 0xFFFFF) == 0, 0)) {
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
    printf("║     🍌 TOS-BANANA-COIN MINER - FINAL STATS (AVX) 🍌      ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║ Total attempts: %-37lu ║\n", counter);
    printf("║ Time: %.2f seconds%-26s║\n", elapsed, "");
    printf("║ Average rate: %.2f M/s%-30s║\n", counter/elapsed/1e6, "");
    printf("║ 🍌 TOS-Banana coins found: %-24d ║\n", coins_found);
    printf("║ AVX lanes: 4%-43s║\n", "");
    printf("╚════════════════════════════════════════════════════════════╝\n");
}

#endif
