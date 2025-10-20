#ifndef AAD_CPU_MINER_H
#define AAD_CPU_MINER_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <stdint.h>
#include "../aad_data_types.h"
#include "../aad_sha1_cpu.h"
#include "../aad_vault.h"

volatile int stop_signal = 0;
volatile int coins_found = 0;

// Batch processing - generate many coins at once
#define BATCH_SIZE_CPU 128

// Pre-allocated buffers (no malloc overhead!)
static u32_t coin_batch[BATCH_SIZE_CPU][14] __attribute__((aligned(64)));
static u32_t hash_batch[BATCH_SIZE_CPU][5] __attribute__((aligned(64)));

// Fast xorshift64* RNG
static uint64_t rng_state = 0;

static inline void init_rng_cpu(void) {
    rng_state = (uint64_t)time(NULL) ^ 0x9e3779b97f4a7c15ULL;
    // Warm up
    for (int i = 0; i < 10; i++) {
        rng_state ^= rng_state >> 12;
        rng_state ^= rng_state << 25;
        rng_state ^= rng_state >> 27;
    }
}

// Ultra-fast random byte generation
static inline uint64_t xorshift64star(void) {
    rng_state ^= rng_state >> 12;
    rng_state ^= rng_state << 25;
    rng_state ^= rng_state >> 27;
    return rng_state * 0x2545F4914F6CDD1DULL;
}

// Bulk coin generation - entire batch at once!
static inline void generate_coin_batch_cpu(void) {
    // Fixed prefix (12 bytes: "DETI coin 2 ")
    const u32_t prefix[3] = {
        0x49544544,  // "DETI"
        0x696F6320,  // " coi"
        0x2032206E   // "n 2 "
    };
    
    for (int idx = 0; idx < BATCH_SIZE_CPU; idx++) {
        u32_t *coin = coin_batch[idx];
        u08_t *c = (u08_t*)coin;
        
        // Copy prefix
        coin[0] = prefix[0];
        coin[1] = prefix[1];
        coin[2] = prefix[2];
        
        // Generate 42 random bytes (use 64-bit chunks for speed!)
        uint64_t rand_val = xorshift64star();
        int rand_idx = 0;
        
        for (int i = 0; i < 42; i++) {
            if (rand_idx == 0) {
                rand_val = xorshift64star();
                rand_idx = 8;
            }
            
            u08_t b = (u08_t)(rand_val & 0xFF);
            rand_val >>= 8;
            rand_idx--;
            
            b = 32 + (b % 95);
            c[(12 + i) ^ 3] = (b == '\n') ? 'X' : b;
        }
        
        // SHA1 padding
        u08_t *w13 = (u08_t*)&coin[13];
        w13[2] = '\n';
        w13[3] = 0x80;
        
        // Zero padding
        coin[10] = 0;
        coin[11] = 0;
        coin[12] = 0;
        w13[0] = 0;
        w13[1] = 0;
    }
}

// Check all hashes in batch
static inline void check_batch_cpu(void) {
    for (int idx = 0; idx < BATCH_SIZE_CPU; idx++) {
        if (hash_batch[idx][0] == 0xAAD20250u) {
            coins_found++;
            printf("\n💰 COIN #%d | Batch:%d | %08X %08X %08X %08X %08X\n",
                   coins_found, idx,
                   hash_batch[idx][0], hash_batch[idx][1],
                   hash_batch[idx][2], hash_batch[idx][3],
                   hash_batch[idx][4]);
            save_coin(coin_batch[idx]);
        }
    }
}

// Main mining loop
static void mine_cpu_coins(void) {
    unsigned long long attempts = 0ULL;
    time_t start = time(NULL), last_print = start;
    
    printf("CPU scalar miner. Ctrl+C to stop.\n\n");
    init_rng_cpu();
    
    while (!stop_signal) {
        // Generate entire batch
        generate_coin_batch_cpu();
        
        // Hash all coins in batch
        for (int idx = 0; idx < BATCH_SIZE_CPU; idx++) {
            sha1(coin_batch[idx], hash_batch[idx]);
        }
        
        // Check results
        check_batch_cpu();
        
        attempts += BATCH_SIZE_CPU;
        
        // Progress update
        if ((attempts & 0x7FFFFu) == 0) {
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
