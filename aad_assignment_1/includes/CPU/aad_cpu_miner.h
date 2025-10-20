#ifndef AAD_CPU_MINER_H
#define AAD_CPU_MINER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <time.h>
#include "../aad_data_types.h"
#include "../aad_sha1_cpu.h"
#include "../aad_vault.h"

volatile int stop_signal = 0;
volatile int coins_found = 0;

// Professor's ultra-fast LCG RNG
static u32_t rng_state = 0;

static inline void init_lcg_rng(void) {
    rng_state = (u32_t)time(NULL) ^ 0x62815281u;
}

static inline u08_t fast_random_byte(void) {
    rng_state = 3134521u * rng_state + 1u;
    return (u08_t)(rng_state >> 23);
}

static inline u08_t random_printable_ascii(void) {
    u08_t b = fast_random_byte();
    b = 32 + (b % 95);
    return (b == '\n') ? 'X' : b;
}

// OPTIMIZED: Generate single coin with professor's LCG
static inline void generate_coin_optimized(u32_t coin[14]) {
    const u32_t prefix[3] = {
        0x44455449u,  // 'D','E','T','I'
        0x20636F69u,  // ' ','c','o','i'
        0x6E203220u   // 'n',' ','2',' '
    };
    u08_t *c = (u08_t*)coin;
    
    // Set prefix
    coin[0] = prefix[0];
    coin[1] = prefix[1];
    coin[2] = prefix[2];
    
    // Generate 42 random bytes
    for (int i = 0; i < 42; i++) {
        c[((12 + i) ^ 3)] = random_printable_ascii();
    }
    
    // Padding
    c[54 ^ 3] = '\n';
    c[55 ^ 3] = 0x80;
    
    // Zero tail
    coin[10] = 0;
    coin[11] = 0;
    coin[12] = 0;
    coin[13] &= 0xFF00FFFFu;
}

static void mine_cpu_coins(void) {
    u32_t coin[14], hash[5];
    unsigned long long attempts;
    time_t start, last_print;
    double elapsed;
    
    attempts = 0ULL;
    init_lcg_rng();
    start = time(NULL);
    last_print = start;
    
    while (!stop_signal) {
        generate_coin_optimized(coin);
        sha1(coin, hash);
        attempts++;
        
        if (hash[0] == 0xAAD20250u) {
            coins_found++;
            printf("\n💰 COIN #%d\n", coins_found);
            save_coin(coin);
        }
        
        // Progress every 16M hashes
        if ((attempts & 0xFFFFFF) == 0) {
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
