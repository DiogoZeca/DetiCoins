//
// CUDA Kernel for Mining BANANA DETI Coins
// Format: "DETI coin 2 TOSbananaCOIN ..."
//

#include "aad_sha1.h"

typedef unsigned int u32_t;
typedef unsigned char u08_t;
typedef unsigned long long u64_t;

extern "C" __global__ __launch_bounds__(RECOMENDED_CUDA_BLOCK_SIZE, 1)
void mine_banana_coins_cuda_kernel(
    u32_t *coins_storage_area,
    u32_t base_value1,
    u32_t base_value2,
    u32_t iteration_offset
)
{
    u32_t n, idx;
    u32_t data[14];
    u32_t hash[5];
    
    // Calculate unique thread ID
    n = (u32_t)threadIdx.x + (u32_t)blockDim.x * (u32_t)blockIdx.x;
    u64_t counter = (u64_t)iteration_offset + (u64_t)n;
    
    // Initialize banana coin data
    // "DETI coin 2 " (12 bytes, words 0-2)
    data[0] = 0x44455449u;  // "DETI"
    data[1] = 0x20636F69u;  // " coi"
    data[2] = 0x6E203220u;  // "n 2 "
    
    // "TOSbananaCOIN " (bytes 12-27, words 3-6)
    data[3] = 0x544F5362u;  // "TOSb"
    data[4] = 0x616E616Eu;  // "anan"
    data[5] = 0x61434F49u;  // "aCOI"
    data[6] = 0x4E202020u;  // "N   "
    
    // Counter values (words 7-9, bytes 28-39)
    data[7] = (u32_t)(counter & 0xFFFFFFFFu);
    data[8] = (u32_t)((counter >> 32) & 0xFFFFFFFFu);
    data[9] = base_value1;
    
    // Zeros (words 10-12, bytes 40-51)
    data[10] = 0x00000000u;
    data[11] = 0x00000000u;
    data[12] = 0x00000000u;
    
    // CRITICAL: SHA-1 padding (word 13)
    data[13] = 0x00000A80u;
    
    // Compute SHA1 hash using custom SHA1 code
    {
        #define T u32_t
        #define C(c) (c)
        #define ROTATE(x,n) (((x) << (n)) | ((x) >> (32 - (n))))
        #define DATA(idx) data[idx]
        #define HASH(idx) hash[idx]
        CUSTOM_SHA1_CODE();
        #undef T
        #undef C
        #undef ROTATE
        #undef DATA
        #undef HASH
    }
    
    // Check if valid DETI coin (hash starts with 0xAAD20250)
    if(hash[0] == 0xAAD20250u)
    {
        // Atomically reserve space in output buffer
        idx = atomicAdd(coins_storage_area, 14u);
        
        // Store coin if space available
        if(idx + 14u <= 1024u)
        {
            for(u32_t i = 0; i < 14u; i++)
            {
                coins_storage_area[idx + i] = data[i];
            }
        }
    }
}
