#ifndef AAD_CPU_AVX2_MINER_H
#define AAD_CPU_AVX2_MINER_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "../aad_data_types.h"
#include "../aad_sha1_cpu.h"
#include "../aad_vault.h"

volatile int stop_signal = 0;
volatile int coins_found = 0;

// Fast inline LCG random number generator
static u32_t rng_state = 0;

static inline void init_rng(void) {
    rng_state = (u32_t)time(NULL) ^ 0x9e3779b9;
}

static inline u08_t fast_random_byte(void) {
    rng_state = 3134521u * rng_state + 1u;
    return (u08_t)((rng_state >> 23) % 95 + 32);
}

// Generate 8 interleaved coins for AVX2 (8-way SIMD)
static inline void generate_8_coins_avx2(v8si coin[14]) {
    static const char prefix[12] = { 'D','E','T','I',' ','c','o','i','n',' ','2',' ' };
    
    // Generate 8 separate coins
    u32_t coins[8][14];
    
    for (int lane = 0; lane < 8; lane++) {
        u08_t *c = (u08_t*)coins[lane];
        
        // Write prefix with XOR mapping
        for (int i = 0; i < 12; i++)
            c[i ^ 3] = (u08_t)prefix[i];
        
        // Generate random payload using fast RNG
        for (int i = 12; i < 54; i++) {
            u08_t r = fast_random_byte();
            if (r == '\n') r = 'X';
            c[i ^ 3] = r;
        }
        
        c[54 ^ 3] = '\n';
        c[55 ^ 3] = 0x80;
    }
    
    // Interleave 8 coins into SIMD vectors
    for (int i = 0; i < 14; i++) {
        coin[i] = (v8si){
            coins[0][i], coins[1][i], coins[2][i], coins[3][i],
            coins[4][i], coins[5][i], coins[6][i], coins[7][i]
        };
    }
}

// Check if any of the 8 hashes match and save valid coins
static inline void check_and_save_8_coins_avx2(v8si coin[14], v8si hash[5]) {
    u32_t hash_scalar[8][5];
    
    // Extract hashes from SIMD vectors
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
    
    // Check each lane for valid coin
    for (int lane = 0; lane < 8; lane++) {
        if (hash_scalar[lane][0] == 0xAAD20250) {
            // Extract coin from interleaved data
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
    init_rng();  // Initialize fast RNG
    
    while (!stop_signal) {
        generate_8_coins_avx2(coin);
        sha1_avx2(coin, hash);
        attempts += 8;  // 8 hashes per iteration
        
        check_and_save_8_coins_avx2(coin, hash);
        
        // Progress report every 5 seconds
        if ((attempts & 0xFFFFF) == 0) {
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

#endif  // AAD_CPU_AVX2_MINER_H
