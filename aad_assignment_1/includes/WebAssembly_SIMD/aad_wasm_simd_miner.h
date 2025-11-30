#ifndef AAD_WASM_SIMD_MINER_H
#define AAD_WASM_SIMD_MINER_H

#include <stdint.h>
#include <string.h>
#include <wasm_simd128.h>

typedef uint8_t  u08_t;
typedef uint32_t u32_t;
typedef uint64_t u64_t;
typedef v128_t   v4u32;

static u64_t g_counter = 0;
static u64_t g_total_hashes = 0;
static int g_coins_found = 0;
static int g_running = 0;

#define MAX_COINS 64
static u32_t g_coin_buffer[MAX_COINS * 14];
static int g_coin_buffer_count = 0;

static inline v4u32 v4u32_set1(u32_t x)
{
    return wasm_i32x4_splat(x);
}

static inline v4u32 v4u32_set4(u32_t a, u32_t b, u32_t c, u32_t d)
{
    return wasm_i32x4_make(a, b, c, d);
}

static inline v4u32 v4u32_rotl(v4u32 x, int n)
{
    return wasm_v128_or(wasm_i32x4_shl(x, n), wasm_u32x4_shr(x, 32 - n));
}

static inline v4u32 v4u32_and(v4u32 a, v4u32 b) { return wasm_v128_and(a, b); }
static inline v4u32 v4u32_or(v4u32 a, v4u32 b) { return wasm_v128_or(a, b); }
static inline v4u32 v4u32_xor(v4u32 a, v4u32 b) { return wasm_v128_xor(a, b); }
static inline v4u32 v4u32_not(v4u32 a) { return wasm_v128_not(a); }
static inline v4u32 v4u32_add(v4u32 a, v4u32 b) { return wasm_i32x4_add(a, b); }

static inline u32_t v4u32_extract(v4u32 v, int lane)
{
    switch (lane) {
        case 0: return wasm_i32x4_extract_lane(v, 0);
        case 1: return wasm_i32x4_extract_lane(v, 1);
        case 2: return wasm_i32x4_extract_lane(v, 2);
        case 3: return wasm_i32x4_extract_lane(v, 3);
        default: return 0;
    }
}

static inline void sha1_simd(v4u32 data[14], v4u32 hash[5])
{
    v4u32 a, b, c, d, e, f, k, temp;
    v4u32 w[80];

    for (int i = 0; i < 14; i++)
        w[i] = data[i];

    w[14] = v4u32_set1(0x00000000u);
    w[15] = v4u32_set1(0x000001B8u);

    for (int i = 16; i < 80; i++)
        w[i] = v4u32_rotl(v4u32_xor(v4u32_xor(v4u32_xor(w[i-3], w[i-8]), w[i-14]), w[i-16]), 1);

    a = v4u32_set1(0x67452301u);
    b = v4u32_set1(0xEFCDAB89u);
    c = v4u32_set1(0x98BADCFEu);
    d = v4u32_set1(0x10325476u);
    e = v4u32_set1(0xC3D2E1F0u);

    for (int i = 0; i < 20; i++) {
        f = v4u32_or(v4u32_and(b, c), v4u32_and(v4u32_not(b), d));
        k = v4u32_set1(0x5A827999u);
        temp = v4u32_add(v4u32_add(v4u32_add(v4u32_add(v4u32_rotl(a, 5), f), e), k), w[i]);
        e = d; d = c; c = v4u32_rotl(b, 30); b = a; a = temp;
    }
    for (int i = 20; i < 40; i++) {
        f = v4u32_xor(v4u32_xor(b, c), d);
        k = v4u32_set1(0x6ED9EBA1u);
        temp = v4u32_add(v4u32_add(v4u32_add(v4u32_add(v4u32_rotl(a, 5), f), e), k), w[i]);
        e = d; d = c; c = v4u32_rotl(b, 30); b = a; a = temp;
    }
    for (int i = 40; i < 60; i++) {
        f = v4u32_or(v4u32_or(v4u32_and(b, c), v4u32_and(b, d)), v4u32_and(c, d));
        k = v4u32_set1(0x8F1BBCDCu);
        temp = v4u32_add(v4u32_add(v4u32_add(v4u32_add(v4u32_rotl(a, 5), f), e), k), w[i]);
        e = d; d = c; c = v4u32_rotl(b, 30); b = a; a = temp;
    }
    for (int i = 60; i < 80; i++) {
        f = v4u32_xor(v4u32_xor(b, c), d);
        k = v4u32_set1(0xCA62C1D6u);
        temp = v4u32_add(v4u32_add(v4u32_add(v4u32_add(v4u32_rotl(a, 5), f), e), k), w[i]);
        e = d; d = c; c = v4u32_rotl(b, 30); b = a; a = temp;
    }

    hash[0] = v4u32_add(v4u32_set1(0x67452301u), a);
    hash[1] = v4u32_add(v4u32_set1(0xEFCDAB89u), b);
    hash[2] = v4u32_add(v4u32_set1(0x98BADCFEu), c);
    hash[3] = v4u32_add(v4u32_set1(0x10325476u), d);
    hash[4] = v4u32_add(v4u32_set1(0xC3D2E1F0u), e);
}

static inline void init_coin_data_simd(v4u32 coin[14])
{
    coin[0] = v4u32_set1(0x44455449u);
    coin[1] = v4u32_set1(0x20636F69u);
    coin[2] = v4u32_set1(0x6E203220u);
    coin[6] = v4u32_set1(0u);
    coin[7] = v4u32_set1(0u);
    coin[8] = v4u32_set1(0u);
    coin[9] = v4u32_set1(0u);
    coin[10] = v4u32_set1(0u);
    coin[11] = v4u32_set1(0u);
    coin[12] = v4u32_set1(0u);
    coin[13] = v4u32_set1(0x00000A80u);
}

static inline void update_counters_simd(v4u32 coin[14], u64_t counter, u32_t seed)
{
    u32_t c0 = (u32_t)((counter + 0) & 0xFFFFFFFFu);
    u32_t c1 = (u32_t)((counter + 1) & 0xFFFFFFFFu);
    u32_t c2 = (u32_t)((counter + 2) & 0xFFFFFFFFu);
    u32_t c3 = (u32_t)((counter + 3) & 0xFFFFFFFFu);

    coin[3] = v4u32_set4(c0, c1, c2, c3);

    u32_t counter_high = (u32_t)((counter >> 32) & 0xFFFFFFFFu);
    coin[4] = v4u32_set1(counter_high);
    coin[5] = v4u32_set1(seed);
}

static inline int validate_scalar_coin(u32_t coin[14])
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

static inline void check_and_save_coins_simd(v4u32 coin[14], v4u32 hash[5])
{
    v4u32 target = v4u32_set1(0xAAD20250u);
    v4u32 cmp = wasm_i32x4_eq(hash[0], target);

    if (!wasm_v128_any_true(cmp))
        return;

    for (int lane = 0; lane < 4; lane++) {
        if (v4u32_extract(hash[0], lane) == 0xAAD20250u) {
            u32_t coin_scalar[14];
            for (int i = 0; i < 14; i++)
                coin_scalar[i] = v4u32_extract(coin[i], lane);

            if (validate_scalar_coin(coin_scalar))
                store_coin(coin_scalar);
        }
    }
}

static inline int mine_batch_simd(u32_t batch_size, u32_t seed)
{
    v4u32 coin[14], hash[5];
    int found_before = g_coins_found;

    init_coin_data_simd(coin);

    u32_t iterations = batch_size / 4;
    for (u32_t i = 0; i < iterations && g_running; i++) {
        update_counters_simd(coin, g_counter, seed);
        sha1_simd(coin, hash);
        check_and_save_coins_simd(coin, hash);

        g_total_hashes += 4;
        g_counter += 4;
    }

    return g_coins_found - found_before;
}

#endif
