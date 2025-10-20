#ifndef AAD_CPU_AVX_MINER_H
#define AAD_CPU_AVX_MINER_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "../aad_data_types.h"
#include "../aad_sha1_cpu.h"
#include "../aad_vault.h"

volatile int stop_signal = 0;
volatile int coins_found = 0;

// Generate 4 interleaved coins for AVX (4-way SIMD)
static inline void generate_4_coins_avx(v4si coin[14]) {
    static const char prefix[12] = { 'D','E','T','I',' ','c','o','i','n',' ','2',' ' };
    
    // Generate 4 separate coins
    u32_t coin0[14], coin1[14], coin2[14], coin3[14];
    
    for (int lane = 0; lane < 4; lane++) {
        u32_t *c_words = (lane == 0) ? coin0 : (lane == 1) ? coin1 : (lane == 2) ? coin2 : coin3;
        u08_t *c = (u08_t*)c_words;
        
        // Write prefix
        for (int i = 0; i < 12; i++)
            c[i ^ 3] = (u08_t)prefix[i];
        
        // Generate random payload
        for (int i = 12; i < 54; i++) {
            int r = (rand() % 95) + 32;
            if (r == '\n') r = 'X';
            c[i ^ 3] = (u08_t)r;
        }
        
        c[54 ^ 3] = '\n';
        c[55 ^ 3] = 0x80;
    }
    
    // Interleave 4 coins into SIMD vectors
    for (int i = 0; i < 14; i++) {
        coin[i] = (v4si){ coin0[i], coin1[i], coin2[i], coin3[i] };
    }
}

// Check if any of the 4 hashes match and save valid coins
static inline void check_and_save_4_coins_avx(v4si coin[14], v4si hash[5]) {
    u32_t hash_scalar[4][5];
    
    // Extract hashes from SIMD vectors
    for (int i = 0; i < 5; i++) {
        hash_scalar[0][i] = hash[i][0];
        hash_scalar[1][i] = hash[i][1];
        hash_scalar[2][i] = hash[i][2];
        hash_scalar[3][i] = hash[i][3];
    }
    
    // Check each lane
    for (int lane = 0; lane < 4; lane++) {
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

static void mine_cpu_avx_coins(void) {
    v4si coin[14], hash[5];
    unsigned long long attempts = 0ULL;
    time_t start = time(NULL), last_print = start;
    
    printf("AVX miner (4-way SIMD). Ctrl+C to stop.\n\n");
    
    while (!stop_signal) {
        generate_4_coins_avx(coin);
        sha1_avx(coin, hash);
        attempts += 4;  // 4 hashes per iteration
        
        check_and_save_4_coins_avx(coin, hash);
        
        // Progress every 5 seconds
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

#endif
