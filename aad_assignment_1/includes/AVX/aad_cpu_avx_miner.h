//
// aad_cpu_avx_miner.h
// CPU AVX DETI coin miner (4 parallel hashes using sha1_avx)
//

#ifndef AAD_CPU_AVX_MINER_H
#define AAD_CPU_AVX_MINER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include "../aad_data_types.h" // u08_t, u32_t, v4si
#include "../aad_sha1_cpu.h"   // sha1_avx()
#include "../aad_vault.h"      // save_coin()

#if defined(__AVX__)

/*
 * generate_coin_avx()
 */
static void generate_coin_avx(u32_t coin[14])
{
    /* template prefix (12 bytes) */
    const char prefix[12] = { 'D','E','T','I',' ','c','o','i','n',' ','2',' ' };
    
    /* Fill a small temporary 55-byte buffer (0..54) safely */
    u08_t buf[55];
    
    /* Set prefix 0..11 */
    for (int i = 0; i < 12; ++i) buf[i] = (u08_t)prefix[i];
    
    /* Fill bytes 12..53 with printable ASCII characters (32..126), avoiding '\n' (0x0A) */
    for (int i = 12; i <= 53; ++i)
    {
        /* pick a printable char: space..tilde (32..126) */
        int r = (rand() % 95) + 32; /* 95 chars from 32..126 inclusive */
        /* If random happens to be newline (0x0A) — avoid it (but it won't from 32..126) */
        if (r == 0x0A) r = 'X';
        buf[i] = (u08_t)r;
    }
    
    /* Ensure last message byte (index 54) is newline '\n' */
    buf[54] = (u08_t)'\n';
    
    /* Now store the 55 message bytes into the coin buffer using the ^3 byte mapping */
    for (int i = 0; i < 55; ++i)
        ((u08_t *)coin)[i ^ 3] = buf[i];
    
    /* Set the SHA1 padding byte at index 55 as required by the project's SHA1 macro */
    ((u08_t *)coin)[55 ^ 3] = 0x80;
}

/*
 * interleave4_data()
 * 
 * Takes 4 coin arrays (each 14 u32_t words) and interleaves them into AVX format.
 * For word index w (0..13):
 *   out[w] = { coin0[w], coin1[w], coin2[w], coin3[w] }
 */
static void interleave4_data(const u32_t coin0[14], const u32_t coin1[14],
                            const u32_t coin2[14], const u32_t coin3[14],
                            v4si out[14])
{
    for (int w = 0; w < 14; ++w) {
        out[w] = (v4si){ (int)coin0[w], (int)coin1[w], (int)coin2[w], (int)coin3[w] };
    }
}

/*
 * deinterleave4_hash()
 * 
 * Takes 4 interleaved hash results (5 v4si words) and extracts them into
 * 4 separate hash arrays (each 5 u32_t words).
 */
static void deinterleave4_hash(const v4si in[5], u32_t h0[5], u32_t h1[5],
                              u32_t h2[5], u32_t h3[5])
{
    for (int k = 0; k < 5; ++k) {
        v4si v = in[k];
        int* p = (int*)&v;
        h0[k] = (u32_t)p[0];
        h1[k] = (u32_t)p[1];
        h2[k] = (u32_t)p[2];
        h3[k] = (u32_t)p[3];
    }
}

/*
 * mine_cpu_coins_avx()
 * 
 * Loop generating 4 coins at a time, computing 4 parallel SHA1 hashes using AVX,
 * testing signatures, and exiting after the first valid DETI coin is found.
 * Should provide ~4x speedup over scalar version.
 */
static void mine_cpu_coins_avx(void)
{
    const unsigned long long REPORT_INTERVAL_ATTEMPTS = 1000000ULL;
    
    /* Initialize PRNG with time-based seed */
    srand((unsigned)time(NULL) ^ (unsigned)(uintptr_t)&REPORT_INTERVAL_ATTEMPTS);
    
    /* Storage for 4 coins and their hashes */
    u32_t coin0[14], coin1[14], coin2[14], coin3[14];
    u32_t hash0[5], hash1[5], hash2[5], hash3[5];
    
    /* AVX interleaved data structures */
    v4si interleaved_data[14];
    v4si interleaved_hash[5];
    
    unsigned long long attempts = 0ULL;
    time_t start_time = time(NULL);
    
    printf("Starting CPU AVX mining loop...\n");
    
    while (1)
    {
        /* Generate 4 coins */
        generate_coin_avx(coin0);
        generate_coin_avx(coin1);
        generate_coin_avx(coin2);
        generate_coin_avx(coin3);
        
        /* Interleave data for AVX processing */
        interleave4_data(coin0, coin1, coin2, coin3, interleaved_data);
        
        /* Compute 4 SHA1 hashes in parallel using AVX */
        sha1_avx(interleaved_data, interleaved_hash);
        
        /* De-interleave results */
        deinterleave4_hash(interleaved_hash, hash0, hash1, hash2, hash3);
        
        attempts += 4;
        
        /* Check each of the 4 hashes for valid DETI coin signature */
        if (hash0[0] == 0xAAD20250u) {
            double elapsed = difftime(time(NULL), start_time);
            printf("\n====================================\n");
            printf("💰 FOUND DETI COIN! 💰\n");
            printf("Attempts: %llu\n", attempts);
            printf("Time elapsed: %.2f seconds\n", elapsed);
            printf("SHA1: %08X %08X %08X %08X %08X\n",
                   hash0[0], hash0[1], hash0[2], hash0[3], hash0[4]);
            printf("====================================\n\n");
            
            save_coin(coin0);
            save_coin(NULL);
            exit(0);
        }
        
        if (hash1[0] == 0xAAD20250u) {
            double elapsed = difftime(time(NULL), start_time);
            printf("\n====================================\n");
            printf("💰 FOUND DETI COIN! 💰\n");
            printf("Attempts: %llu\n", attempts);
            printf("Time elapsed: %.2f seconds\n", elapsed);
            printf("SHA1: %08X %08X %08X %08X %08X\n",
                   hash1[0], hash1[1], hash1[2], hash1[3], hash1[4]);
            printf("====================================\n\n");
            
            save_coin(coin1);
            save_coin(NULL);
            exit(0);
        }
        
        if (hash2[0] == 0xAAD20250u) {
            double elapsed = difftime(time(NULL), start_time);
            printf("\n====================================\n");
            printf("💰 FOUND DETI COIN! 💰\n");
            printf("Attempts: %llu\n", attempts);
            printf("Time elapsed: %.2f seconds\n", elapsed);
            printf("SHA1: %08X %08X %08X %08X %08X\n",
                   hash2[0], hash2[1], hash2[2], hash2[3], hash2[4]);
            printf("====================================\n\n");
            
            save_coin(coin2);
            save_coin(NULL);
            exit(0);
        }
        
        if (hash3[0] == 0xAAD20250u) {
            double elapsed = difftime(time(NULL), start_time);
            printf("\n====================================\n");
            printf("💰 FOUND DETI COIN! 💰\n");
            printf("Attempts: %llu\n", attempts);
            printf("Time elapsed: %.2f seconds\n", elapsed);
            printf("SHA1: %08X %08X %08X %08X %08X\n",
                   hash3[0], hash3[1], hash3[2], hash3[3], hash3[4]);
            printf("====================================\n\n");
            
            save_coin(coin3);
            save_coin(NULL);
            exit(0);
        }
        
        /* Progress reporting */
        if ((attempts % REPORT_INTERVAL_ATTEMPTS) == 0ULL)
        {
            double elapsed = difftime(time(NULL), start_time);
            double rate = elapsed > 0.0 ? (double)attempts / elapsed : 0.0;
            printf("[%.1fs] Attempts: %llu (%.2f attempts/s)\n",
                   elapsed, attempts, rate);
            fflush(stdout);
        }
    }
    
    /* Never reached, but keep for completeness */
    save_coin(NULL);
}

#else
/* AVX not available - provide a fallback or error message */
static void mine_cpu_coins_avx(void)
{
    printf("ERROR: AVX support not available. Please compile with -mavx or use scalar miner.\n");
    exit(1);
}
#endif /* __AVX__ */

#endif /* AAD_CPU_AVX_MINER_H */