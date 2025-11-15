#ifndef AAD_CPU_BANANA_MINER_H
#define AAD_CPU_BANANA_MINER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <stdint.h>

#include "../aad_data_types.h"
#include "../aad_sha1_cpu.h"
#include "../aad_vault.h"

static volatile int stop_signal = 0;
static volatile int coins_found = 0;

//
// Generate banana coin message
//
static inline void generate_coin_counter(u32_t coin[14], u64_t counter) {
    coin[0] = 0x44455449u;  // "DETI"
    coin[1] = 0x20636F69u;  // " coi"
    coin[2] = 0x6E203220u;  // "n 2 "
    
    // "TOSbananaCOIN"
    coin[3] = 0x544F5362u;  // "TOSb"
    coin[4] = 0x616E616Eu;  // "anan"
    coin[5] = 0x61434F49u;  // "aCOI"
    coin[6] = 0x4E202020u;  // "N   "
    
    // Counter data (words 7-9) - SAFE, no random patterns
    coin[7] = (u32_t)(counter & 0xFFFFFFFFu);
    coin[8] = (u32_t)((counter >> 32) & 0xFFFFFFFFu);
    coin[9] = (u32_t)time(NULL);
    
    // Fill remaining with zeros (SAFE - guaranteed no newlines)
    coin[10] = 0u;
    coin[11] = 0u;
    coin[12] = 0u;
    
    // Last word: newline + SHA-1 padding
    coin[13] = 0x00000A80u;
}


//
// Main mining function
//
static inline void mine_cpu_coins(void) {
    u32_t coin[14] __attribute__((aligned(16)));
    u32_t hash[5] __attribute__((aligned(16)));
    u64_t counter = 0;
    time_t start, last_print;
    double elapsed;
    
    start = time(NULL);
    last_print = start;
    
    printf("🍌 Starting TOS-BANANA-COIN mining (CPU Scalar)...\n");
    printf("   Target: DETI coin 2 TOSbananaCOIN ...\n\n");
    
    while (!stop_signal) {
        // Generate banana coin
        generate_coin_counter(coin, counter);
        
        // Compute SHA1 hash
        sha1(coin, hash);
        
        // Check if valid DETI coin (hash starts with 0xAAD20250)
        if (__builtin_expect(hash[0] == 0xAAD20250u, 0)) {
            // Additional validation: check no '\n' in variable part
            u08_t *base_coin = (u08_t *)coin;
            int valid = 1;
            
            for (int i = 12; i < 54; i++) {
                if (base_coin[i ^ 3] == '\n') {  // XOR 3 for little-endian byte access
                    valid = 0;
                    break;
                }
            }
            
            if (valid) {
                coins_found++;
                printf("\n🍌💰 TOS-BANANA-COIN #%d FOUND! 💰🍌\n", coins_found);
                
                // Print the coin content
                printf("   Content: ");
                for (int i = 0; i < 55; i++) {
                    char c = base_coin[i ^ 3];  // Adjust for endianness
                    if (c == '\n') {
                        printf("\\n");
                    } else if (c >= 32 && c <= 126) {
                        printf("%c", c);
                    } else {
                        printf("\\x%02x", (unsigned char)c);
                    }
                }
                printf("\n");
                
                // Print hash
                printf("   Hash: %08X%08X%08X%08X%08X\n", 
                       hash[0], hash[1], hash[2], hash[3], hash[4]);
                printf("   Counter: %lu\n\n", counter);
                
                // Save coin to vault
                save_coin(coin);
            }
        }
        
        counter++;
        
        // Progress report every ~16M attempts (check every 2^24)
        if (__builtin_expect((counter & 0xFFFFFF) == 0, 0)) {
            time_t now = time(NULL);
            if (difftime(now, last_print) >= 5.0) {
                elapsed = difftime(now, start);
                printf("[%.0fs] %luM @ %.2fM/s | 🍌 Banana Coins:%d\n",
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
    printf("║          🍌 BANANA COIN MINER - FINAL STATS 🍌            ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║ Total attempts: %-37lu ║\n", counter);
    printf("║ Time: %.2f seconds%-26s║\n", elapsed, "");
    printf("║ Average rate: %.2f M/s%-30s║\n", counter/elapsed/1e6, "");
    printf("║ 🍌 Banana coins found: %-28d ║\n", coins_found);
    printf("╚════════════════════════════════════════════════════════════╝\n");
}

#endif
