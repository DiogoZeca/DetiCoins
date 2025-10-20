#ifndef AAD_CPU_AVX2_MINER_H
#define AAD_CPU_AVX2_MINER_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include "../aad_data_types.h"
#include "../aad_sha1_cpu.h"
#include "../aad_vault.h"

volatile int stop_signal = 0;
volatile int coins_found = 0;

static u32_t rng_state[8];

static const u08_t mod95_lut[256] = {
    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
    64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
    80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
    96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
    112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 32,
    33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64,
    65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80,
    81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96,
    97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112,
    113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 32, 33,
    34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
    50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65,
    66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81,
    82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97
};

static inline void init_rng_avx2(void) {
    u32_t seed = (u32_t)time(NULL);
    for (int i = 0; i < 8; i++)
        rng_state[i] = seed ^ (0x9e3779b9u + (u32_t)i * 0x12345678u);
}

static inline u08_t fast_random_byte_avx2(int lane) {
    rng_state[lane] = 1103515245u * rng_state[lane] + 12345u;
    u08_t r = mod95_lut[(rng_state[lane] >> 16) & 0xFFu];
    return (r == '\n') ? (u08_t)'X' : r;
}

static inline void generate_8_coins_avx2(v8si coin[14]) {
    u32_t coins[8][14];
    
    for (int lane = 0; lane < 8; lane++) {
        // Zero initialize
        for (int i = 0; i < 14; i++)
            coins[lane][i] = 0;
        
        u08_t *c = (u08_t*)coins[lane];
        
        // Prefix
        c[0x03] = 'D';  c[0x02] = 'E';  c[0x01] = 'T';  c[0x00] = 'I';
        c[0x07] = ' ';  c[0x06] = 'c';  c[0x05] = 'o';  c[0x04] = 'i';
        c[0x0B] = 'n';  c[0x0A] = ' ';  c[0x09] = '2';  c[0x08] = ' ';
        
        // Random payload
        for (int i = 12; i < 54; i++)
            c[i ^ 3] = fast_random_byte_avx2(lane);
        
        // Padding - FIXED: direct word access
        u08_t *w13 = (u08_t*)&coins[lane][13];
        w13[2] = '\n';
        w13[3] = 0x80;
    }
    
    // Interleave
    for (int i = 0; i < 14; i++) {
        coin[i] = (v8si){
            coins[0][i], coins[1][i], coins[2][i], coins[3][i],
            coins[4][i], coins[5][i], coins[6][i], coins[7][i]
        };
    }
}

static inline void check_and_save_8_coins_avx2(v8si coin[14], v8si hash[5]) {
    u32_t hash_scalar[8][5];
    
    for (int i = 0; i < 5; i++) {
        hash_scalar[0][i] = hash[i][0];
        hash_scalar[1][i] = hash[i][1];
        hash_scalar[2][i] = hash[i][2];
        hash_scalar[3][i] = hash[i][3];
        hash_scalar[4][i] = hash[i][4];
        hash_scalar[5][i] = hash[i][5];
        hash_scalar[6][i] = hash[i][6];
        hash_scalar[7][i] = hash[i][7];
    }
    
    for (int lane = 0; lane < 8; lane++) {
        if (hash_scalar[lane][0] == 0xAAD20250u) {
            u32_t coin_scalar[14];
            for (int i = 0; i < 14; i++)
                coin_scalar[i] = coin[i][lane];
            
            coins_found++;
            printf("\n💰 COIN #%d (Lane %d) | %08X %08X %08X %08X %08X\n",
                   coins_found, lane,
                   hash_scalar[lane][0], hash_scalar[lane][1],
                   hash_scalar[lane][2], hash_scalar[lane][3],
                   hash_scalar[lane][4]);
            save_coin(coin_scalar);
        }
    }
}

static void mine_cpu_avx2_coins(void) {
    v8si coin[14], hash[5];
    unsigned long long attempts = 0ULL;
    time_t start = time(NULL), last_print = start;
    
    printf("AVX2 miner (8-way SIMD). Ctrl+C to stop.\n\n");
    init_rng_avx2();
    
    while (!stop_signal) {
        generate_8_coins_avx2(coin);
        sha1_avx2(coin, hash);
        attempts += 8;
        
        check_and_save_8_coins_avx2(coin, hash);
        
        if ((attempts & 0x3FFFFFu) == 0) {
            time_t now = time(NULL);
            if (difftime(now, last_print) >= 5.0) {
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
