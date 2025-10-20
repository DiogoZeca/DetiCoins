#ifndef AAD_CPU_MINER_H
#define AAD_CPU_MINER_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "../aad_data_types.h"
#include "../aad_sha1_cpu.h"
#include "../aad_vault.h"

volatile int stop_signal = 0;
volatile int coins_found = 0;

static u32_t rng_state = 0;

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

static inline void init_rng(void) {
    rng_state = (u32_t)time(NULL) ^ 0x9e3779b9u;
}

static inline u08_t fast_random_byte(void) {
    rng_state = 1103515245u * rng_state + 12345u;
    u08_t r = mod95_lut[(rng_state >> 16) & 0xFFu];
    return (r == '\n') ? (u08_t)'X' : r;
}

static inline void generate_coin(u32_t coin[14]) {
    // Zero initialize all 14 words (56 bytes total)
    for (int i = 0; i < 14; i++)
        coin[i] = 0;
    
    u08_t *c = (u08_t*)coin;
    
    // Prefix "DETI coin 2 " (12 bytes: 0-11) with XOR byte swapping
    c[0x03] = 'D';  c[0x02] = 'E';  c[0x01] = 'T';  c[0x00] = 'I';
    c[0x07] = ' ';  c[0x06] = 'c';  c[0x05] = 'o';  c[0x04] = 'i';
    c[0x0B] = 'n';  c[0x0A] = ' ';  c[0x09] = '2';  c[0x08] = ' ';
    
    // Random payload (42 bytes: 12-53)
    for (int i = 12; i < 54; i++)
        c[i ^ 3] = fast_random_byte();
    
    // SHA1 padding - FIXED: Use direct word access to avoid out-of-bounds
    // Bytes 54-55 are in word 13 (54 = 13*4+2, 55 = 13*4+3)
    u08_t *w13 = (u08_t*)&coin[13];
    w13[2] = '\n';   // Byte 54
    w13[3] = 0x80;   // Byte 55 (padding marker)
}

static void mine_cpu_coins(void) {
    u32_t coin[14], hash[5];
    unsigned long long attempts = 0ULL;
    time_t start = time(NULL), last_print = start;
    
    printf("CPU scalar miner. Ctrl+C to stop.\n\n");
    init_rng();
    
    while (!stop_signal) {
        generate_coin(coin);
        sha1(coin, hash);
        attempts++;
        
        if (hash[0] == 0xAAD20250u) {
            coins_found++;
            double elapsed = difftime(time(NULL), start);
            printf("\n💰 COIN #%d | %llu attempts | %.2fs | %08X %08X %08X %08X %08X\n",
                   coins_found, attempts, elapsed, 
                   hash[0], hash[1], hash[2], hash[3], hash[4]);
            save_coin(coin);
        }
        
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
