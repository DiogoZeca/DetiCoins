#ifndef AAD_CUDA_MINER_H
#define AAD_CUDA_MINER_H

#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include "../aad_data_types.h"
#include "../aad_utilities.h"
#include "../aad_sha1_cpu.h"
#include "../aad_cuda_utilities.h"
#include "../aad_vault.h"
#include "../aad_coin_types.h"

static volatile int stop_signal = 0;
static volatile int coins_found = 0;

#define COINS_BUFFER_SIZE 1024u
#define THREADS_PER_KERNEL_LAUNCH (128u * 65536u)

static inline void mine_cuda_coins(const coin_config_t *config) {
    cuda_data_t cd;
    u32_t *host_coins_buffer;
    u32_t base_value1, base_value2, iteration_counter;
    u64_t total_attempts = 0;

    // Prepare custom text words for kernel
    u32_t custom_words[8] = {0};  // Initialize to zeros (words 5-12)
    u32_t timestamp_word_idx = 5;

    if (config->type == COIN_TYPE_CUSTOM && config->custom_text != NULL) {
        u32_t temp_coin[14] = {0};
        encode_custom_text(temp_coin, config->custom_text, 5);

        // Extract custom text words (5-12)
        for (int i = 0; i < 8; i++) {
            custom_words[i] = temp_coin[5 + i];
        }

        // Calculate timestamp position
        size_t text_len = strlen(config->custom_text);
        timestamp_word_idx = 5 + ((text_len + 3) / 4);
    }

    cd.device_number = 0;
    cd.cubin_file_name = "./CUDA/mine_deti_coins_cuda_kernel.cubin";
    cd.kernel_name = "mine_deti_coins_cuda_kernel";
    cd.data_size[0] = COINS_BUFFER_SIZE * sizeof(u32_t);
    cd.data_size[1] = 0u;

    // Print startup message
    if (config->type == COIN_TYPE_CUSTOM) {
        printf("[+] Starting CUSTOM coin mining (CUDA)...\n");
        printf("   Custom text: \"%s\"\n", config->custom_text);
    } else {
        printf("[*] Starting DETI coin mining (CUDA)...\n");
    }
    printf("============================================================\n");

    initialize_cuda(&cd);
    printf("GPU: %s\n", cd.device_name);

    host_coins_buffer = (u32_t *)cd.host_data[0];
    base_value1 = (u32_t)time(NULL);
    base_value2 = (u32_t)getpid();
    iteration_counter = 0u;

    time_t start = time(NULL);
    time_t last_print = start;

    while (!stop_signal) {
        host_coins_buffer[0] = 1u;
        host_to_device_copy(&cd, 0);

        cd.grid_dim_x = THREADS_PER_KERNEL_LAUNCH / RECOMENDED_CUDA_BLOCK_SIZE;
        cd.block_dim_x = RECOMENDED_CUDA_BLOCK_SIZE;
        cd.n_kernel_arguments = 13;
        cd.arg[0] = &cd.device_data[0];
        cd.arg[1] = &base_value1;
        cd.arg[2] = &base_value2;
        cd.arg[3] = &iteration_counter;
        cd.arg[4] = &custom_words[0];  // word 5
        cd.arg[5] = &custom_words[1];  // word 6
        cd.arg[6] = &custom_words[2];  // word 7
        cd.arg[7] = &custom_words[3];  // word 8
        cd.arg[8] = &custom_words[4];  // word 9
        cd.arg[9] = &custom_words[5];  // word 10
        cd.arg[10] = &custom_words[6]; // word 11
        cd.arg[11] = &custom_words[7]; // word 12
        cd.arg[12] = &timestamp_word_idx;

        lauch_kernel(&cd);
        device_to_host_copy(&cd, 0);

        u32_t next_free_idx = host_coins_buffer[0];
        if (next_free_idx > 1u && next_free_idx <= COINS_BUFFER_SIZE) {
            u32_t coins = (next_free_idx - 1u) / 14u;
            for (u32_t i = 0; i < coins; i++) {
                u32_t coin_offset = 1u + i * 14u;
                if (coin_offset + 14u <= next_free_idx) {
                    u32_t *coin_data = &host_coins_buffer[coin_offset];
                    u08_t *base_coin = (u08_t *)coin_data;
                    int valid = 1;

                    for (int j = 12; j < 54; j++) {
                        if (base_coin[j ^ 3] == '\n') {
                            valid = 0;
                            break;
                        }
                    }

                    if (valid) {
                        u32_t hash[5];
                        sha1(coin_data, hash);

                        if (hash[0] == 0xAAD20250u) {
                            coins_found++;
                            printf("\n%s COIN #%d (GPU)\n", (config->type == COIN_TYPE_CUSTOM ? "[+]" : "[*]"), coins_found);
                            save_coin(coin_data);
                        }
                    }
                }
            }
        }

        total_attempts += THREADS_PER_KERNEL_LAUNCH;
        iteration_counter += THREADS_PER_KERNEL_LAUNCH;
        base_value1 = base_value1 * 1103515245u + 12345u;
        base_value2 ^= iteration_counter;

        time_t now = time(NULL);
        double elapsed = difftime(now, start);
        if (elapsed >= 5.0 && difftime(now, last_print) >= 5.0) {
            double hash_rate = total_attempts / elapsed / 1e6;
            printf("[%ds] %lu M @ %.2f M/s | Coins: %d | GPU: 1\n",
                   (int)elapsed,
                   total_attempts / 1000000UL,
                   hash_rate,
                   coins_found);
            last_print = now;
        }
    }

    time_t end = time(NULL);
    double elapsed = difftime(end, start);
    double final_rate = total_attempts / elapsed / 1e6;

    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║ CUDA GPU FINAL STATISTICS                                  ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║ GPU: %-51s║\n", cd.device_name);
    printf("║ Total attempts: %-37lu ║\n", total_attempts);
    printf("║ Time: %.2f seconds%-26s║\n", elapsed, "");
    printf("║ Average rate: %.2f M/s%-30s║\n", final_rate, "");
    printf("║ Coins found: %-37d ║\n", coins_found);
    printf("╚════════════════════════════════════════════════════════════╝\n");

    terminate_cuda(&cd);
}

#endif
