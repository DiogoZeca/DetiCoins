#ifndef AAD_CPU_AVX_MINER_H
#define AAD_CPU_AVX_MINER_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>
#include "../aad_data_types.h"
#include "../aad_sha1_cpu.h"
#include "../aad_vault.h"

volatile int stop_signal = 0;
volatile int coins_found = 0;

#define BATCH_SIZE_AVX 128
#define SIMD_WIDTH_AVX 4

// Pre-allocated buffers
static u32_t coin_batch[BATCH_SIZE_AVX][14] __attribute__((aligned(64)));
static v4si simd_coins[BATCH_SIZE_AVX / SIMD_WIDTH_AVX][14] __attribute__((aligned(64)));
static v4si simd_hashes[BATCH_SIZE_AVX / SIMD_WIDTH_AVX][5] __attribute__((aligned(64)));

// SIMD RNG states
static __m128i rng_state_avx[SIMD_WIDTH_AVX];

static inline void init_rng_avx(void) {
    uint64_t seed = (uint64_t)time(NULL);
    for (int i = 0; i < SIMD_WIDTH_AVX; i++) {
        uint64_t s = seed ^ (0x9e3779b97f4a7c15ULL + (uint64_t)i * 0x12345678ULL);
        rng_state_avx[i] = _mm_set_epi64x((long long)s, (long long)(s ^ 0x6a09e667ULL));
    }
}

// Fast SIMD RNG (xorshift128)
static inline __m128i xorshift128_avx(__m128i state) {
    __m128i x = state;
    x = _mm_xor_si128(x, _mm_slli_epi64(x, 23));
    x = _mm_xor_si128(x, _mm_srli_epi64(x, 18));
    x = _mm_xor_si128(x, _mm_srli_epi64(state, 5));
    return _mm_add_epi64(x, state);
}

// Bulk coin generation
static inline void generate_coin_batch_avx(void) {
    const u32_t prefix[3] = { 0x49544544, 0x696F6320, 0x2032206E };
    
    for (int idx = 0; idx < BATCH_SIZE_AVX; idx++) {
        u32_t *coin = coin_batch[idx];
        u08_t *c = (u08_t*)coin;
        
        // Prefix
        coin[0] = prefix[0];
        coin[1] = prefix[1];
        coin[2] = prefix[2];
        
        // Random payload
        int lane = idx & 3;
        rng_state_avx[lane] = xorshift128_avx(rng_state_avx[lane]);
        
        uint64_t rand_vals[2];
        _mm_storeu_si128((__m128i*)rand_vals, rng_state_avx[lane]);
        
        int rand_idx = 0;
        uint64_t rand_val = rand_vals[0];
        
        for (int i = 0; i < 42; i++) {
            if (rand_idx == 0) {
                if (i < 21) {
                    rand_val = rand_vals[0];
                } else if (i == 21) {
                    rng_state_avx[lane] = xorshift128_avx(rng_state_avx[lane]);
                    _mm_storeu_si128((__m128i*)rand_vals, rng_state_avx[lane]);
                    rand_val = rand_vals[0];
                } else {
                    rand_val = rand_vals[1];
                }
                rand_idx = 8;
            }
            
            u08_t b = (u08_t)(rand_val & 0xFF);
            rand_val >>= 8;
            rand_idx--;
            
            b = 32 + (b % 95);
            c[(12 + i) ^ 3] = (b == '\n') ? 'X' : b;
        }
        
        // Padding
        u08_t *w13 = (u08_t*)&coin[13];
        w13[2] = '\n';
        w13[3] = 0x80;
        coin[10] = 0;
        coin[11] = 0;
        coin[12] = 0;
        w13[0] = 0;
        w13[1] = 0;
    }
}

// Interleave batch into SIMD format
static inline void interleave_batch_avx(void) {
    for (int group = 0; group < BATCH_SIZE_AVX / SIMD_WIDTH_AVX; group++) {
        int base = group * SIMD_WIDTH_AVX;
        for (int word = 0; word < 14; word++) {
            simd_coins[group][word] = (v4si){
                coin_batch[base + 0][word],
                coin_batch[base + 1][word],
                coin_batch[base + 2][word],
                coin_batch[base + 3][word]
            };
        }
    }
}

// Check batch results
static inline void check_batch_avx(void) {
    for (int group = 0; group < BATCH_SIZE_AVX / SIMD_WIDTH_AVX; group++) {
        for (int lane = 0; lane < SIMD_WIDTH_AVX; lane++) {
            if ((u32_t)simd_hashes[group][0][lane] == 0xAAD20250u) {
                int coin_idx = group * SIMD_WIDTH_AVX + lane;
                coins_found++;
                printf("\n💰 COIN #%d | Group:%d Lane:%d | %08X %08X %08X %08X %08X\n",
                       coins_found, group, lane,
                       simd_hashes[group][0][lane],
                       simd_hashes[group][1][lane],
                       simd_hashes[group][2][lane],
                       simd_hashes[group][3][lane],
                       simd_hashes[group][4][lane]);
                save_coin(coin_batch[coin_idx]);
            }
        }
    }
}

// Main mining loop
static void mine_cpu_avx_coins(void) {
    unsigned long long attempts = 0ULL;
    time_t start = time(NULL), last_print = start;
    
    printf("AVX miner (4-way SIMD). Ctrl+C to stop.\n\n");
    init_rng_avx();
    
    while (!stop_signal) {
        // Generate batch
        generate_coin_batch_avx();
        
        // Interleave
        interleave_batch_avx();
        
        // Hash all groups
        for (int group = 0; group < BATCH_SIZE_AVX / SIMD_WIDTH_AVX; group++) {
            sha1_avx(simd_coins[group], simd_hashes[group]);
        }
        
        // Check results
        check_batch_avx();
        
        attempts += BATCH_SIZE_AVX;
        
        // Progress
        if ((attempts & 0xFFFFFu) == 0) {
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
