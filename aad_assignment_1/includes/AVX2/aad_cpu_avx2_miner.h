#ifndef AAD_CPU_AVX2_MINER_H
#define AAD_CPU_AVX2_MINER_H

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

// Professor's ultra-fast LCG RNG
static u32_t rng_state[8];

static inline void init_lcg_rng(void) {
    u32_t base = (u32_t)time(NULL);
    for (int i = 0; i < 8; i++) {
        rng_state[i] = base ^ (0x62815281u + i * 0x9e3779b9u);
    }
}

// Professor's LCG: x = 3134521*x + 1
static inline u08_t fast_random_byte(int lane) {
    rng_state[lane] = 3134521u * rng_state[lane] + 1u;
    return (u08_t)(rng_state[lane] >> 23);
}

// Map to printable ASCII [32-126], avoiding '\n'
static inline u08_t random_printable_ascii(int lane) {
    u08_t b = fast_random_byte(lane);
    b = 32 + (b % 95);
    return (b == '\n') ? 'X' : b;
}

// Generate 8 coins in parallel
static inline void generate_8_coins_avx2_ultimate(v8si coin[14]) {
    const u32_t prefix[3] = {
        0x44455449u,  // 'D','E','T','I'
        0x20636F69u,  // ' ','c','o','i'
        0x6E203220u   // 'n',' ','2',' '
    };
    
    u32_t coins[8][14];
    
    for (int lane = 0; lane < 8; lane++) {
        u08_t *c = (u08_t*)coins[lane];
        
        coins[lane][0] = prefix[0];
        coins[lane][1] = prefix[1];
        coins[lane][2] = prefix[2];
        
        for (int i = 0; i < 42; i++) {
            c[((12 + i) ^ 3)] = random_printable_ascii(lane);
        }
        
        c[54 ^ 3] = '\n';
        c[55 ^ 3] = 0x80;
        
        coins[lane][10] = 0;
        coins[lane][11] = 0;
        coins[lane][12] = 0;
        coins[lane][13] &= 0xFF00FFFFu;
    }
    
    for (int i = 0; i < 14; i++) {
        coin[i] = (v8si){
            coins[0][i], coins[1][i], coins[2][i], coins[3][i],
            coins[4][i], coins[5][i], coins[6][i], coins[7][i]
        };
    }
}

// Fast SIMD checking
static inline void check_and_save_8_coins_avx2_fast(v8si coin[14], v8si hash[5]) {
    __m256i target, hash0_vec, cmp;
    int mask;
    
    // SIMD comparison
    target = _mm256_set1_epi32(0xAAD20250u);
    hash0_vec = (__m256i)hash[0];
    cmp = _mm256_cmpeq_epi32(hash0_vec, target);
    mask = _mm256_movemask_epi8(cmp);
    
    if (mask == 0) return;
    
    // Extract matching coins
    for (int lane = 0; lane < 8; lane++) {
        if ((u32_t)hash[0][lane] == 0xAAD20250u) {
            u32_t coin_scalar[14];
            for (int i = 0; i < 14; i++)
                coin_scalar[i] = coin[i][lane];
            
            coins_found++;
            printf("\n💰 COIN #%d (Lane %d) | %08X %08X %08X %08X %08X\n",
                   coins_found, lane,
                   (u32_t)hash[0][lane], (u32_t)hash[1][lane],
                   (u32_t)hash[2][lane], (u32_t)hash[3][lane],
                   (u32_t)hash[4][lane]);
            save_coin(coin_scalar);
        }
    }
}

static void mine_cpu_avx2_coins(void) {
    v8si coin[14], hash[5];
    unsigned long long attempts;
    time_t start, last_print;
    
    attempts = 0ULL;
    init_lcg_rng();
    start = time(NULL);
    last_print = start;
    
    printf("AVX2 miner (8-way SIMD). Ctrl+C to stop.\n");
    
    while (!stop_signal) {
        generate_8_coins_avx2_ultimate(coin);
        sha1_avx2(coin, hash);
        check_and_save_8_coins_avx2_fast(coin, hash);
        
        attempts += 8;
        
        if ((attempts & 0x1FFFFFF) == 0) {
            time_t now = time(NULL);
            if (difftime(now, last_print) >= 3.0) {
                double elapsed = difftime(now, start);
                printf("[%.1fs] %lluM @ %.2fM/s | Coins:%d\n",
                       elapsed, attempts/1000000ULL,
                       (elapsed > 0 ? attempts/elapsed/1e6 : 0),
                       coins_found);
                last_print = now;
            }
        }
    }
    
    double total = difftime(time(NULL), start);
    printf("\n========== FINAL STATISTICS ==========\n");
    printf("Total attempts:  %llu\n", attempts);
    printf("Time:            %.2f seconds\n", total);
    printf("Average rate:    %.2f M hashes/sec\n", attempts/total/1e6);
    printf("Coins found:     %d\n", coins_found);
    printf("======================================\n");
}

#endif
