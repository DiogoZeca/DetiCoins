#define _POSIX_C_SOURCE 199309L
#include <time.h>
#include "aad_data_types.h"
#include "aad_sha1_cpu.h"
#include "aad_sha1_banana_opencl.h"
#include "aad_vault.h"
#include "aad_utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>


static volatile int stop_signal = 0;
static volatile int coins_found = 0;


void handle_sigint(int sig)
{
  (void)sig;
  printf("\n[Stopping OpenCL banana mining...]\n");
  stop_signal = 1;
}


static char *load_kernel_source(const char *filename)
{
  FILE *fp = fopen(filename, "r");
  if(fp == NULL)
  {
    fprintf(stderr, "Cannot open %s\n", filename);
    return NULL;
  }
  
  fseek(fp, 0, SEEK_END);
  size_t size = (size_t)ftell(fp);
  fseek(fp, 0, SEEK_SET);
  
  char *source = (char *)malloc(size + 1);
  if(source == NULL)
  {
    fclose(fp);
    return NULL;
  }
  
  size_t read = fread(source, 1, size, fp);
  source[read] = '\0';
  fclose(fp);
  
  return source;
}


int main(int argc, char **argv)
{
  opencl_context_t ctx;
  cl_int err;
  
  signal(SIGINT, handle_sigint);
  
  if(argc < 3)
  {
    printf("Usage: %s <platform_id> <device_id>\n\n", argv[0]);
    list_opencl_devices();
    return 0;
  }
  
  int platform_id = atoi(argv[1]);
  int device_id = atoi(argv[2]);
  
  if(initialize_opencl(&ctx, platform_id, device_id) != 0)
  {
    return 1;
  }
  
  char *kernel_source = load_kernel_source("Banana_Miner/OpenCL/aad_sha1_opencl_banana_kernel.cl");
  if(kernel_source == NULL)
  {
    cleanup_opencl(&ctx);
    return 1;
  }
  
  if(load_opencl_kernel(&ctx, kernel_source, "mine_banana_coins", "-cl-fast-relaxed-math -cl-mad-enable") != 0)
  {
    free(kernel_source);
    cleanup_opencl(&ctx);
    return 1;
  }
  free(kernel_source);
  
  // Mining configuration (match CUDA structure)
  const u32_t COINS_BUFFER_SIZE = 1024u;
  const u32_t THREADS_PER_LAUNCH = 128u * 65536u;  // Same as CUDA
  const size_t GLOBAL_WORK_SIZE = THREADS_PER_LAUNCH;
  const size_t LOCAL_WORK_SIZE = 256u;
  
  cl_mem found_coins_buffer = clCreateBuffer(ctx.context, CL_MEM_READ_WRITE, 
                                             COINS_BUFFER_SIZE * sizeof(u32_t), NULL, &err);
  if(err != CL_SUCCESS)
  {
    fprintf(stderr, "Error creating buffer\n");
    cleanup_opencl(&ctx);
    return 1;
  }
  
  u32_t *host_coins_buffer = (u32_t *)malloc(COINS_BUFFER_SIZE * sizeof(u32_t));
  
  printf("🍌 Starting OpenCL BANANA mining\n");
  printf("============================================================\n");
  printf("GPU: %s\n", ctx.device_name);
  
  // Initialize base values like CUDA
  u32_t base_value1 = (u32_t)time(NULL);
  u32_t base_value2 = (u32_t)getpid();
  u32_t iteration_counter = 0u;
  u64_t total_attempts = 0ull;
  
  time_t start = time(NULL);
  time_t last_print = start;
  
  while(!stop_signal)
  {
    // Initialize buffer
    host_coins_buffer[0] = 1u;
    clEnqueueWriteBuffer(ctx.queue, found_coins_buffer, CL_TRUE, 0, 
                        sizeof(u32_t), &host_coins_buffer[0], 0, NULL, NULL);
    
    // Set kernel arguments (match CUDA: buffer, base_value1, base_value2, iteration_counter)
    clSetKernelArg(ctx.kernel, 0, sizeof(cl_mem), &found_coins_buffer);
    clSetKernelArg(ctx.kernel, 1, sizeof(u32_t), &base_value1);
    clSetKernelArg(ctx.kernel, 2, sizeof(u32_t), &base_value2);
    clSetKernelArg(ctx.kernel, 3, sizeof(u32_t), &iteration_counter);
    
    // Launch kernel
    err = clEnqueueNDRangeKernel(ctx.queue, ctx.kernel, 1, NULL, 
                                &GLOBAL_WORK_SIZE, &LOCAL_WORK_SIZE, 0, NULL, NULL);
    if(err != CL_SUCCESS)
    {
      fprintf(stderr, "Kernel launch failed: %d\n", err);
      break;
    }
    
    // Read results
    clEnqueueReadBuffer(ctx.queue, found_coins_buffer, CL_TRUE, 0, 
                       COINS_BUFFER_SIZE * sizeof(u32_t), host_coins_buffer, 0, NULL, NULL);
    
    // Check for found coins
    u32_t next_free_idx = host_coins_buffer[0];
    if(next_free_idx > 1u && next_free_idx <= COINS_BUFFER_SIZE)
    {
      u32_t coins = (next_free_idx - 1u) / 14u;
      
      for(u32_t i = 0u; i < coins; i++)
      {
        u32_t coin_offset = 1u + i * 14u;
        if(coin_offset + 14u <= next_free_idx)
        {
          u32_t *coin_data = &host_coins_buffer[coin_offset];
          
          // VALIDATE: Check for newlines in bytes 12-53
          u08_t *base_coin = (u08_t *)coin_data;
          int valid = 1;
          
          for(int j = 12; j < 54; j++)
          {
            if(base_coin[j ^ 3] == '\n')
            {
              valid = 0;
              break;
            }
          }
          
          if(valid)
          {
            // Verify hash on host
            u32_t hash[5];
            sha1(coin_data, hash);
            
            if(hash[0] == 0xAAD20250u)
            {
              coins_found++;
              printf("\n🍌💰 BANANA-COIN #%d (OpenCL)\n", coins_found);
              save_coin(coin_data);
            }
          }
        }
      }
    }
    
    // Update counters
    total_attempts += THREADS_PER_LAUNCH;
    iteration_counter += THREADS_PER_LAUNCH;
    
    // Vary base values like CUDA
    base_value1 = base_value1 * 1103515245u + 12345u;
    base_value2 ^= iteration_counter;
    
    // Progress reporting (every 5 seconds)
    time_t now = time(NULL);
    double elapsed = difftime(now, start);
    
    if(elapsed >= 5.0 && difftime(now, last_print) >= 5.0)
    {
      double hash_rate = total_attempts / elapsed / 1e6;
      printf("[%ds] %lu M @ %.2f M/s | 🍌 Banana Coins: %d | GPU: 1\n",
             (int)elapsed,
             total_attempts / 1000000UL,
             hash_rate,
             coins_found);
      last_print = now;
    }
  }
  
  save_coin(NULL);
  
  // Final statistics
  time_t end = time(NULL);
  double elapsed = difftime(end, start);
  double final_rate = (elapsed > 0) ? (total_attempts / elapsed / 1e6) : 0.0;
  
  printf("\n╔════════════════════════════════════════════════════════════╗\n");
  printf("║ 🍌 OpenCL BANANA GPU FINAL STATISTICS 🍌                  ║\n");
  printf("╠════════════════════════════════════════════════════════════╣\n");
  printf("║ GPU: %-51s║\n", ctx.device_name);
  printf("║ Total attempts: %-37lu ║\n", total_attempts);
  printf("║ Time: %.2f seconds%-36s║\n", elapsed, "");
  printf("║ Average rate: %.2f M/s%-34s║\n", final_rate, "");
  printf("║ 🍌 Banana coins found: %-30d ║\n", coins_found);
  printf("╚════════════════════════════════════════════════════════════╝\n");
  
  free(host_coins_buffer);
  clReleaseMemObject(found_coins_buffer);
  cleanup_opencl(&ctx);
  
  return 0;
}
