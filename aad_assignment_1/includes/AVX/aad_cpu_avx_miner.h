#ifndef AAD_CPU_AVX_MINER_H
#define AAD_CPU_AVX_MINER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <time.h>
#include <immintrin.h>
#include "../aad_data_types.h"
#include "../aad_sha1_cpu.h"
#include "../aad_vault.h"

volatile int stop_signal = 0;
volatile int coins_found = 0;

// Professor's LCG per lane (4 lanes for AVX)
static u32_t rng_state[4];

static inline void init_lcg_rng(void) {
    u32_t base = (u32_t)time(NULL);
    for (int i = 0; i < 4; i++) {
        rng_state[i] = base ^ (0x62815281u + i * 0x9e3779b9u);
    }
}

// Batch generate 4 bytes at once
static inline void fast_random_bytes_batch(u08_t out[4]) {
    for (int i = 0; i < 4; i++) {
        rng_state[i] = 3134521u * rng_state[i] + 1u;
        u08_t b = (u08_t)(rng_state[i] >> 23);
        b = 32 + (b % 95);
        out[i] = (b == '\n') ? 'X' : b;
    }
}

// OPTIMIZED: Generate 4 coins with batched RNG
static inline void generate_4_coins_avx_optimized(v4si coin[14]) {
    const u32_t prefix[3] = {
        0x44455449u,  // 'D','E','T','I'
        0x20636F69u,  // ' ','c','o','i'
        0x6E203220u   // 'n',' ','2',' '
    };
    
    u32_t coins[4][14];
    u08_t random_batch[4];
    
    // Set prefix for all 4 coins
    for (int lane = 0; lane < 4; lane++) {
        coins[lane][0] = prefix[0];
        coins[lane][1] = prefix[1];
        coins[lane][2] = prefix[2];
    }
    
    // Generate 42 random bytes in batches of 4
    for (int i = 0; i < 42; i++) {
        fast_random_bytes_batch(random_batch);
        for (int lane = 0; lane < 4; lane++) {
            u08_t *c = (u08_t*)coins[lane];
            c[((12 + i) ^ 3)] = random_batch[lane];
        }
    }
    
    // Set padding for all 4 coins
    for (int lane = 0; lane < 4; lane++) {
        u08_t *c = (u08_t*)coins[lane];
        c[54 ^ 3] = '\n';
        c[55 ^ 3] = 0x80;
        coins[lane][10] = 0;
        coins[lane][11] = 0;
        coins[lane][12] = 0;
        coins[lane][13] &= 0xFF00FFFFu;
    }
    
    // Interleave into SIMD vectors
    for (int i = 0; i < 14; i++) {
        coin[i] = (v4si){
            coins[0][i], coins[1][i], coins[2][i], coins[3][i]
        };
    }
}

// Fast SIMD checking
static inline void check_and_save_4_coins_avx_fast(v4si coin[14], v4si hash[5]) {
    __m128i target, hash0_vec, cmp;
    int mask;
    
    target = _mm_set1_epi32(0xAAD20250u);
    hash0_vec = (__m128i)hash[0];
    cmp = _mm_cmpeq_epi32(hash0_vec, target);
    mask = _mm_movemask_epi8(cmp);
    
    if (mask == 0) return;
    
    for (int lane = 0; lane < 4; lane++) {
        if ((u32_t)hash[0][lane] == 0xAAD20250u) {
            u32_t coin_scalar[14];
            for (int i = 0; i < 14; i++)
                coin_scalar[i] = coin[i][lane];
            
            coins_found++;
            printf("\n💰 COIN #%d (Lane %d)\n", coins_found, lane);
            save_coin(coin_scalar);
        }
    }
}

static void mine_cpu_avx_coins(void) {
    v4si coin[14], hash[5];
    unsigned long long attempts;
    time_t start, last_print;
    double elapsed;
    
    attempts = 0ULL;
    init_lcg_rng();
    start = time(NULL);
    last_print = start;
    
    while (!stop_signal) {
        generate_4_coins_avx_optimized(coin);
        sha1_avx(coin, hash);
        check_and_save_4_coins_avx_fast(coin, hash);
        
        attempts += 4;
        
        // Progress every 32M hashes
        if ((attempts & 0x1FFFFFF) == 0) {
            time_t now = time(NULL);
            if (difftime(now, last_print) >= 5.0) {
                elapsed = difftime(now, start);
                printf("[%.0fs] %lluM @ %.2fM/s | Coins:%d\n",
                       elapsed, attempts/1000000ULL,
                       (elapsed > 0 ? attempts/elapsed/1e6 : 0),
                       coins_found);
                last_print = now;
            }
        }
    }
    
    elapsed = difftime(time(NULL), start);
    printf("\n╔════════════════════════════════════════════════════╗\n");
    printf("║              FINAL STATISTICS                      ║\n");
    printf("╠════════════════════════════════════════════════════╣\n");
    printf("║  Total attempts:  %-29llu  ║\n", attempts);
    printf("║  Time:            %.2f seconds%-19s║\n", elapsed, "");
    printf("║  Average rate:    %.2f M/s%-23s║\n", attempts/elapsed/1e6, "");
    printf("║  Coins found:     %-29d  ║\n", coins_found);
    printf("╚════════════════════════════════════════════════════╝\n");
}

#endif
