#ifndef AAD_CPU_AVX2_OPENMP_BANANA_MINER_H
#define AAD_CPU_AVX2_OPENMP_BANANA_MINER_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <omp.h>
#include <string.h>
#include <immintrin.h>
#include <unistd.h>
#include "../aad_data_types.h"
#include "../aad_sha1_cpu.h"
#include "../aad_vault.h"
#include "../aad_utilities.h"

static volatile int stop_signal = 0;
static volatile int coins_found = 0;

// BANANA VERSION: Initialize banana coin data (8-wide AVX2)
static inline void init_banana_coin_data_avx2(v8si coin[14]) {
  // "DETI coin 2 " (bytes 0-11, words 0-2)
  coin[0] = (v8si){0x44455449u, 0x44455449u, 0x44455449u, 0x44455449u,
                   0x44455449u, 0x44455449u, 0x44455449u, 0x44455449u};  // "DETI"
  coin[1] = (v8si){0x20636F69u, 0x20636F69u, 0x20636F69u, 0x20636F69u,
                   0x20636F69u, 0x20636F69u, 0x20636F69u, 0x20636F69u};  // " coi"
  coin[2] = (v8si){0x6E203220u, 0x6E203220u, 0x6E203220u, 0x6E203220u,
                   0x6E203220u, 0x6E203220u, 0x6E203220u, 0x6E203220u};  // "n 2 "
  
  // "TOSbananaCOIN " (bytes 12-27, words 3-6)
  coin[3] = (v8si){0x544F5362u, 0x544F5362u, 0x544F5362u, 0x544F5362u,
                   0x544F5362u, 0x544F5362u, 0x544F5362u, 0x544F5362u};  // "TOSb"
  coin[4] = (v8si){0x616E616Eu, 0x616E616Eu, 0x616E616Eu, 0x616E616Eu,
                   0x616E616Eu, 0x616E616Eu, 0x616E616Eu, 0x616E616Eu};  // "anan"
  coin[5] = (v8si){0x61434F49u, 0x61434F49u, 0x61434F49u, 0x61434F49u,
                   0x61434F49u, 0x61434F49u, 0x61434F49u, 0x61434F49u};  // "aCOI"
  coin[6] = (v8si){0x4E202020u, 0x4E202020u, 0x4E202020u, 0x4E202020u,
                   0x4E202020u, 0x4E202020u, 0x4E202020u, 0x4E202020u};  // "N   "
  
  // Zeros (words 10-12)
  coin[10] = (v8si){0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
  coin[11] = (v8si){0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
  coin[12] = (v8si){0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
}

// BANANA VERSION: Update counters (now at words 7-9)
static inline void update_banana_counters_avx2(v8si coin[14], u64_t counter) {
  u32_t base_counters[8];
  for (int i = 0; i < 8; i++) {
    base_counters[i] = (u32_t)((counter + i) & 0xFFFFFFFFu);
  }
  
  // Counter low (word 7)
  coin[7] = (v8si){base_counters[0], base_counters[1], base_counters[2], base_counters[3],
                   base_counters[4], base_counters[5], base_counters[6], base_counters[7]};
  
  // Counter high (word 8)
  u32_t counter_high = (u32_t)((counter >> 32) & 0xFFFFFFFFu);
  coin[8] = (v8si){counter_high, counter_high, counter_high, counter_high,
                   counter_high, counter_high, counter_high, counter_high};
  
  // Time seed (word 9)
  u32_t time_seed = (u32_t)time(NULL);
  coin[9] = (v8si){time_seed, time_seed, time_seed, time_seed,
                   time_seed, time_seed, time_seed, time_seed};
  
  // Padding (word 13)
  coin[13] = (v8si){0x00000A80u, 0x00000A80u, 0x00000A80u, 0x00000A80u,
                    0x00000A80u, 0x00000A80u, 0x00000A80u, 0x00000A80u};
}

static inline void check_and_save_banana_coins_avx2(v8si coin[14], v8si hash[5]) {
  __m256i target = _mm256_set1_epi32(0xAAD20250u);
  __m256i hash0_vec = (__m256i)hash[0];
  __m256i cmp = _mm256_cmpeq_epi32(hash0_vec, target);
  int mask = _mm256_movemask_epi8(cmp);
  
  if (__builtin_expect(mask == 0, 1)) {
    return;
  }
  
  u32_t *hash_data = (u32_t *)&hash[0];
  for (int lane = 0; lane < 8; lane++) {
    if (hash_data[lane] == 0xAAD20250u) {
      u32_t coin_scalar[14] __attribute__((aligned(16)));
      
      for (int i = 0; i < 14; i++) {
        u32_t *coin_data = (u32_t *)&coin[i];
        coin_scalar[i] = coin_data[lane];
      }
      
      u08_t *base_coin = (u08_t *)coin_scalar;
      int valid = 1;
      
      for (int i = 12; i < 54; i++) {
        if (base_coin[i ^ 3] == '\n') {
          valid = 0;
          break;
        }
      }
      
      if (valid) {
        #pragma omp atomic
        coins_found++;
        
        #pragma omp critical
        {
          printf("\n🍌💰 BANANA-COIN #%d (Thread %d, Lane %d)\n",
                 coins_found, omp_get_thread_num(), lane);
          save_coin(coin_scalar);
        }
      }
    }
  }
}

static inline void mine_cpu_avx2_banana_coins_openmp(void) {
  int num_threads = omp_get_max_threads();
  printf("🍌 Starting AVX2 OpenMP BANANA mining with %d threads\n", num_threads);
  printf("============================================================\n");
  
  time_t start = time(NULL);
  u64_t total_attempts = 0;
  
  #pragma omp parallel
  {
    int thread_id = omp_get_thread_num();
    v8si coin[14] __attribute__((aligned(32)));
    v8si hash[5] __attribute__((aligned(32)));
    u64_t local_counter = 0;
    u64_t thread_offset = (u64_t)thread_id * 1000000000ULL;
    time_t last_print = start;
    
    // BANANA: Initialize with banana format
    init_banana_coin_data_avx2(coin);
    
    while (!stop_signal) {
      u64_t counter = thread_offset + local_counter;
      
      // BANANA: Update banana counters
      update_banana_counters_avx2(coin, counter);
      
      sha1_avx2(coin, hash);
      
      // BANANA: Check and save banana coins
      check_and_save_banana_coins_avx2(coin, hash);
      
      local_counter += 8;
      
      // Update shared counter periodically (every 1M hashes)
      if (__builtin_expect((local_counter & 0xFFFFF) == 0, 0)) {
        #pragma omp atomic
        total_attempts += 0x100000;
      }
      
      // Progress reporting (master thread only every 5 seconds)
      if (thread_id == 0 && __builtin_expect((local_counter & 0xFFFFF) == 0, 0)) {
        time_t now = time(NULL);
        double elapsed = difftime(now, start);
        
        if (elapsed >= 5.0 && difftime(now, last_print) >= 5.0) {
          u64_t snapshot;
          #pragma omp atomic read
          snapshot = total_attempts;
          
          double hash_rate = snapshot / elapsed / 1e6;
          printf("[%ds] %lu M @ %.2f M/s | 🍌 Banana Coins: %d | Threads: %d\n",
                 (int)elapsed,
                 snapshot / 1000000UL,
                 hash_rate,
                 coins_found,
                 num_threads);
          last_print = now;
        }
      }
    }
    
    // Final accumulation of remaining hashes
    u64_t remainder = local_counter & 0xFFFFF;
    if (remainder > 0) {
      #pragma omp atomic
      total_attempts += remainder;
    }
  }
  
  time_t end = time(NULL);
  double elapsed = difftime(end, start);
  double final_rate = total_attempts / elapsed / 1e6;
  
  printf("\n╔════════════════════════════════════════════════════════════╗\n");
  printf("║ 🍌 AVX2 OPENMP BANANA FINAL STATISTICS 🍌                 ║\n");
  printf("╠════════════════════════════════════════════════════════════╣\n");
  printf("║ Threads: %-37d ║\n", num_threads);
  printf("║ Total attempts: %-37lu ║\n", total_attempts);
  printf("║ Time: %.2f seconds%-26s║\n", elapsed, "");
  printf("║ Average rate: %.2f M/s%-30s║\n", final_rate, "");
  printf("║ 🍌 Banana coins found: %-30d ║\n", coins_found);
  printf("╚════════════════════════════════════════════════════════════╝\n");
}

#endif
