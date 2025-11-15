#ifndef AAD_CPU_OPENMP_BANANA_MINER_H
#define AAD_CPU_OPENMP_BANANA_MINER_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <omp.h>
#include <string.h>
#include <unistd.h>
#include "../aad_data_types.h"
#include "../aad_sha1_cpu.h"
#include "../aad_vault.h"

static volatile int stop_signal = 0;
static volatile int coins_found = 0;

// BANANA VERSION: Change coin generation
static inline void generate_banana_coin_counter(u32_t coin[14], u64_t counter) {
  // "DETI coin 2 " (bytes 0-11, words 0-2)
  coin[0] = 0x44455449u;  // "DETI"
  coin[1] = 0x20636F69u;  // " coi"
  coin[2] = 0x6E203220u;  // "n 2 "
  
  // "TOSbananaCOIN " (bytes 12-27, words 3-6)
  coin[3] = 0x544F5362u;  // "TOSb"
  coin[4] = 0x616E616Eu;  // "anan"
  coin[5] = 0x61434F49u;  // "aCOI"
  coin[6] = 0x4E202020u;  // "N   "
  
  // Counter values (words 7-9, bytes 28-39)
  coin[7] = (u32_t)(counter & 0xFFFFFFFFu);
  coin[8] = (u32_t)((counter >> 32) & 0xFFFFFFFFu);
  coin[9] = (u32_t)time(NULL);
  
  // Zeros (words 10-12, bytes 40-51)
  coin[10] = 0u;
  coin[11] = 0u;
  coin[12] = 0u;
  
  // SHA-1 padding (word 13)
  coin[13] = 0x00000A80u;
}

static inline void mine_cpu_banana_coins_openmp(void) {
  int num_threads = omp_get_max_threads();
  printf("🍌 Starting OpenMP BANANA mining with %d threads\n", num_threads);
  printf("============================================================\n");
  
  time_t start = time(NULL);
  u64_t total_attempts = 0;
  
  #pragma omp parallel
  {
    int thread_id = omp_get_thread_num();
    u32_t coin[14] __attribute__((aligned(16)));
    u32_t hash[5] __attribute__((aligned(16)));
    u64_t local_counter = 0;
    u64_t thread_offset = (u64_t)thread_id * 1000000000ULL;
    time_t last_print = start;
    
    while (!stop_signal) {
      u64_t counter = thread_offset + local_counter;
      
      // BANANA: Use banana coin generator
      generate_banana_coin_counter(coin, counter);
      
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
          #pragma omp atomic
          coins_found++;
          
          #pragma omp critical
          {
            printf("\n🍌💰 BANANA-COIN #%d (Thread %d)\n", coins_found, thread_id);
            save_coin(coin);
          }
        }
      }
      
      local_counter++;
      
      // Update shared counter periodically
      if (__builtin_expect((local_counter & 0xFFFFF) == 0, 0)) {
        #pragma omp atomic
        total_attempts += 0x100000;
      }
      
      // Progress reporting (master thread only)
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
    
    // Final accumulation
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
  printf("║ 🍌 OPENMP BANANA CPU FINAL STATISTICS 🍌                  ║\n");
  printf("╠════════════════════════════════════════════════════════════╣\n");
  printf("║ Threads: %-37d ║\n", num_threads);
  printf("║ Total attempts: %-37lu ║\n", total_attempts);
  printf("║ Time: %.2f seconds%-36s║\n", elapsed, "");
  printf("║ Average rate: %.2f M/s%-34s║\n", final_rate, "");
  printf("║ 🍌 Banana coins found: %-30d ║\n", coins_found);
  printf("╚════════════════════════════════════════════════════════════╝\n");
}

#endif
