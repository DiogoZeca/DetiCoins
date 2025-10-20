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

static inline void generate_coin(u32_t coin[14]) {
    static const char prefix[12] = { 'D','E','T','I',' ','c','o','i','n',' ','2',' ' };
    u08_t *c = (u08_t*)coin;
    
    for (int i = 0; i < 12; i++)
        c[i ^ 3] = (u08_t)prefix[i];
    
    for (int i = 12; i < 54; i++) {
        int r = (rand() % 95) + 32;
        if (r == '\n') r = 'X';
        c[i ^ 3] = (u08_t)r;
    }
    
    c[54 ^ 3] = '\n';
    c[55 ^ 3] = 0x80;
}

static void mine_cpu_coins(void) {
    u32_t coin[14], hash[5];
    unsigned long long attempts = 0ULL;
    time_t start = time(NULL), last_print = start;
    
    while (!stop_signal) {
        generate_coin(coin);
        sha1(coin, hash);
        attempts++;
        
        if (hash[0] == 0xAAD20250) {
            coins_found++;
            double elapsed = difftime(time(NULL), start);
            printf("\n💰 COIN #%d | %llu attempts | %.2fs | %08X %08X %08X %08X %08X\n",
                   coins_found, attempts, elapsed, 
                   hash[0], hash[1], hash[2], hash[3], hash[4]);
            save_coin(coin);
        }
        
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
