#ifndef AAD_CPU_MINER_H
#define AAD_CPU_MINER_H

#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../aad_data_types.h"
#include "../aad_sha1_cpu.h"
#include "../aad_vault.h"
#include "../aad_coin_types.h"

static volatile int stop_signal = 0;
static volatile int coins_found = 0;

// optional custom text
static inline void generate_coin_counter(u32_t coin[14], u64_t counter, const coin_config_t *config) {
    coin[0] = 0x44455449u;  // "DETI"
    coin[1] = 0x20636F69u;  // " coi"
    coin[2] = 0x6E203220u;  // "n 2 "

    coin[3] = (u32_t)(counter & 0xFFFFFFFFu);
    coin[4] = (u32_t)((counter >> 32) & 0xFFFFFFFFu);

    int next_word = 5;  

    // if CUSTOM
    if (config->type == COIN_TYPE_CUSTOM && config->custom_text != NULL) {
        next_word = encode_custom_text(coin, config->custom_text, 5);
    }

    coin[next_word] = (u32_t)time(NULL);
    next_word++;

    for (int i = next_word; i < 13; i++) {
        coin[i] = 0u;
    }

    coin[13] = 0x00000A80u;
}

static inline void mine_cpu_coins(const coin_config_t *config) {
    u32_t coin[14] __attribute__((aligned(16)));
    u32_t hash[5] __attribute__((aligned(16)));
    u64_t counter = 0;
    time_t start, last_print;
    double elapsed;

    start = time(NULL);
    last_print = start;

    // startup message
    if (config->type == COIN_TYPE_CUSTOM) {
        printf("[+] Starting CUSTOM coin mining (CPU Scalar)...\n");
        printf("   Custom text: \"%s\"\n\n", config->custom_text);
    } else {
        printf("[*] Starting DETI coin mining (CPU Scalar)...\n\n");
    }

    while (!stop_signal) {
        generate_coin_counter(coin, counter, config);
        sha1(coin, hash);
        if (__builtin_expect(hash[0] == 0xAAD20250u, 0)) {
            u08_t *base_coin = (u08_t *)coin;
            int valid = 1;
            for (int i = 12; i < 54; i++) {
                if (base_coin[i ^ 3] == '\n') {
                    valid = 0;
                    break;
                }
            }
            if (valid) {
                coins_found++;
                printf("\n%s COIN #%d\n", (config->type == COIN_TYPE_CUSTOM ? "[+]" : "[*]"), coins_found);
                save_coin(coin);
            }
        }
        counter++;

        if (__builtin_expect((counter & 0xFFFFFF) == 0, 0)) {
            time_t now = time(NULL);
            if (difftime(now, last_print) >= 5.0) {
                elapsed = difftime(now, start);
                printf("[%.0fs] %luM @ %.2fM/s | Coins:%d\n",
                       elapsed, counter/1000000UL,
                       (elapsed > 0 ? counter/elapsed/1e6 : 0),
                       coins_found);
                last_print = now;
            }
        }
    }

    elapsed = difftime(time(NULL), start);
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║                      FINAL STATISTICS                      ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║ Total attempts:  %-37lu ║\n", counter);
    printf("║ Time:            %.2f seconds%-26s║\n", elapsed, "");
    printf("║ Average rate:    %.2f M/s%-30s║\n", counter/elapsed/1e6, "");
    printf("║ Coins found:     %-37d ║\n", coins_found);
    printf("╚════════════════════════════════════════════════════════════╝\n");
}

#endif