//
// Arquiteturas de Alto Desempenho 2025/2026
//
// WebAssembly DETI Coin Miner
//

#ifndef AAD_WASM_MINER_H
#define AAD_WASM_MINER_H

#include <stdint.h>
#include <string.h>

typedef uint8_t  u08_t;
typedef uint32_t u32_t;
typedef uint64_t u64_t;

#define T           u32_t
#define C(c)        (c)
#define ROTATE(x,n) (((x) << (n)) | ((x) >> (32 - (n))))
#define DATA(idx)   coin[idx]
#define HASH(idx)   hash[idx]

#include "../aad_sha1.h"

static u64_t g_counter = 0;
static u64_t g_total_hashes = 0;
static int g_coins_found = 0;
static int g_running = 0;

#define MAX_COINS 64
static u32_t g_coin_buffer[MAX_COINS * 14];
static int g_coin_buffer_count = 0;

static inline void generate_coin(u32_t coin[14], u64_t counter, u32_t seed)
{
    coin[0] = 0x44455449u;
    coin[1] = 0x20636F69u;
    coin[2] = 0x6E203220u;
    coin[3] = (u32_t)(counter & 0xFFFFFFFFu);
    coin[4] = (u32_t)((counter >> 32) & 0xFFFFFFFFu);
    coin[5] = seed;
    coin[6] = 0u;
    coin[7] = 0u;
    coin[8] = 0u;
    coin[9] = 0u;
    coin[10] = 0u;
    coin[11] = 0u;
    coin[12] = 0u;
    coin[13] = 0x00000A80u;
}

static inline void sha1_wasm(u32_t coin[14], u32_t hash[5])
{
    CUSTOM_SHA1_CODE();
}

static inline int validate_coin(u32_t coin[14])
{
    u08_t *bytes = (u08_t *)coin;
    for (int i = 12; i < 54; i++)
        if (bytes[i ^ 3] == '\n')
            return 0;
    return 1;
}

static inline void store_coin(u32_t coin[14])
{
    if (g_coin_buffer_count < MAX_COINS) {
        memcpy(&g_coin_buffer[g_coin_buffer_count * 14], coin, 14 * sizeof(u32_t));
        g_coin_buffer_count++;
        g_coins_found++;
    }
}

static inline int mine_batch(u32_t batch_size, u32_t seed)
{
    u32_t coin[14], hash[5];
    int found = 0;

    for (u32_t i = 0; i < batch_size && g_running; i++) {
        generate_coin(coin, g_counter, seed);
        sha1_wasm(coin, hash);
        g_total_hashes++;

        if (hash[0] == 0xAAD20250u && validate_coin(coin)) {
            store_coin(coin);
            found++;
        }
        g_counter++;
    }
    return found;
}

#undef T
#undef C
#undef ROTATE
#undef DATA
#undef HASH

#endif
