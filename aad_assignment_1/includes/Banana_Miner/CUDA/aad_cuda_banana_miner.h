#ifndef AAD_CUDA_BANANA_MINER_H
#define AAD_CUDA_BANANA_MINER_H

#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <unistd.h> 

#include "../aad_data_types.h"
#include "../aad_utilities.h"
#include "../aad_sha1_cpu.h"
#include "../aad_cuda_utilities.h"
#include "../aad_vault.h"

static volatile int stop_signal = 0;
static volatile int coins_found = 0;

#define COINS_BUFFER_SIZE 1024u
#define THREADS_PER_KERNEL_LAUNCH (128u * 65536u)

static inline void mine_cuda_banana_coins(void) {
    cuda_data_t cd;
    u32_t *host_coins_buffer;
    u32_t base_value1, base_value2, iteration_counter;
    u64_t total_attempts = 0;
    
    // Initialize CUDA
    cd.device_number = 0;
    cd.cubin_file_name = "./Banana_Miner/CUDA/mine_banana_coins_cuda_kernel.cubin";
    cd.kernel_name = "mine_banana_coins_cuda_kernel";
    cd.data_size[0] = COINS_BUFFER_SIZE * sizeof(u32_t);
    cd.data_size[1] = 0u;
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║     🍌 CUDA TOS-BANANA-COIN GPU MINER v1.0 🍌            ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║ Initializing CUDA device...                               ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    initialize_cuda(&cd);
    
    printf("\n🚀 GPU: %s\n", cd.device_name);
    printf("🍌 Target: DETI coin 2 TOSbananaCOIN ...\n");
    printf("⚡ Threads per launch: %u\n\n", THREADS_PER_KERNEL_LAUNCH);
    
    host_coins_buffer = (u32_t *)cd.host_data[0];
    base_value1 = (u32_t)time(NULL);
    base_value2 = (u32_t)getpid();
    iteration_counter = 0u;
    
    time_t start = time(NULL);
    time_t last_print = start;
    
    // Mining loop
    while (!stop_signal) {
        // Initialize buffer (first word is counter)
        host_coins_buffer[0] = 1u;
        
        // Copy to device
        host_to_device_copy(&cd, 0);
        
        // Configure and launch kernel
        cd.grid_dim_x = THREADS_PER_KERNEL_LAUNCH / RECOMENDED_CUDA_BLOCK_SIZE;
        cd.block_dim_x = RECOMENDED_CUDA_BLOCK_SIZE;
        cd.n_kernel_arguments = 4;
        cd.arg[0] = &cd.device_data[0];
        cd.arg[1] = &base_value1;
        cd.arg[2] = &base_value2;
        cd.arg[3] = &iteration_counter;
        
        lauch_kernel(&cd);
        
        // Copy results back
        device_to_host_copy(&cd, 0);
        
        // Check for found coins
        u32_t next_free_idx = host_coins_buffer[0];
        if (next_free_idx > 1u && next_free_idx <= COINS_BUFFER_SIZE) {
            u32_t num_coins = (next_free_idx - 1u) / 14u;
            
            for (u32_t i = 0; i < num_coins; i++) {
                u32_t coin_offset = 1u + i * 14u;
                
                if (coin_offset + 14u <= next_free_idx) {
                    u32_t *coin_data = &host_coins_buffer[coin_offset];
                    
                    // Validate: No '\n' in bytes 12-53
                    u08_t *base_coin = (u08_t *)coin_data;
                    int valid = 1;
                    
                    for (int j = 12; j < 54; j++) {
                        if (base_coin[j ^ 3] == '\n') {
                            valid = 0;
                            break;
                        }
                    }
                    
                    if (valid) {
                        // Verify hash on host
                        u32_t hash[5];
                        sha1(coin_data, hash);
                        
                        if (hash[0] == 0xAAD20250u) {
                            coins_found++;
                            
                            printf("\n🍌💰 TOS-BANANA-COIN #%d FOUND! (GPU) 💰🍌\n", coins_found);
                            
                            // Print content
                            printf("   Content: ");
                            for (int k = 0; k < 55; k++) {
                                char c = base_coin[k ^ 3];
                                if (c == '\n') printf("\\n");
                                else if (c >= 32 && c <= 126) printf("%c", c);
                                else printf("\\x%02x", (unsigned char)c);
                            }
                            printf("\n");
                            
                            // Print hash
                            printf("   Hash: %08X%08X%08X%08X%08X\n",
                                   hash[0], hash[1], hash[2], hash[3], hash[4]);
                            printf("\n");
                            
                            // Save coin
                            save_coin(coin_data);
                        }
                    }
                }
            }
        }
        
        // Update counters
        total_attempts += THREADS_PER_KERNEL_LAUNCH;
        iteration_counter += THREADS_PER_KERNEL_LAUNCH;
        
        // Vary base values
        base_value1 = base_value1 * 1103515245u + 12345u;
        base_value2 ^= iteration_counter;
        
        // Progress reporting (every 5 seconds)
        time_t now = time(NULL);
        double elapsed = difftime(now, start);
        
        if (elapsed >= 5.0 && difftime(now, last_print) >= 5.0) {
            double hash_rate = total_attempts / elapsed / 1e6;
            printf("[%.0fs] %lu M @ %.2f M/s | 🍌 Banana Coins: %d\n",
                   elapsed,
                   total_attempts / 1000000UL,
                   hash_rate,
                   coins_found);
            last_print = now;
        }
    }
    
    // Final statistics
    time_t end = time(NULL);
    double elapsed = difftime(end, start);
    double final_rate = total_attempts / elapsed / 1e6;
    
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║   🍌 CUDA TOS-BANANA-COIN MINER - FINAL STATS 🍌         ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║ GPU: %-51s║\n", cd.device_name);
    printf("║ Total attempts: %-37lu ║\n", total_attempts);
    printf("║ Time: %.2f seconds%-26s║\n", elapsed, "");
    printf("║ Average rate: %.2f M/s%-30s║\n", final_rate, "");
    printf("║ 🍌 TOS-Banana coins found: %-24d ║\n", coins_found);
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    // Cleanup
    terminate_cuda(&cd);
}

#endif
