//
// CUDA Kernel for Mining DETI Coins
//

#include "../aad_sha1.h"

typedef unsigned int u32_t;
typedef unsigned char u08_t;
typedef unsigned long long u64_t;

extern "C" __global__ __launch_bounds__(RECOMENDED_CUDA_BLOCK_SIZE, 1)
void mine_deti_coins_cuda_kernel(
    u32_t *coins_storage_area,
    u32_t base_value1,
    u32_t base_value2,
    u32_t iteration_offset
)
{
    u32_t n, idx;
    u32_t data[14];
    u32_t hash[5];
    
    n = (u32_t)threadIdx.x + (u32_t)blockDim.x * (u32_t)blockIdx.x;
    u64_t counter = (u64_t)iteration_offset + (u64_t)n;
    
    // Initialize coin data EXACTLY like CPU miners
    data[0] = 0x44455449u;  // "DETI"
    data[1] = 0x20636F69u;  // " coi"
    data[2] = 0x6E203220u;  // "n 2 "
    
    // Words 3-5: Counter values (VARIABLE)
    data[3] = (u32_t)(counter & 0xFFFFFFFFu);
    data[4] = (u32_t)((counter >> 32) & 0xFFFFFFFFu);
    data[5] = base_value1;
    
    // Words 6-12: ZEROS
    data[6] = 0x00000000u;
    data[7] = 0x00000000u;
    data[8] = 0x00000000u;
    data[9] = 0x00000000u;
    data[10] = 0x00000000u;
    data[11] = 0x00000000u;
    data[12] = 0x00000000u;
    
    // Word 13: CRITICAL - Must be 0x00000A80u (newline + padding)
    data[13] = 0x00000A80u;
    
    // Compute SHA1 hash
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
    
    if(hash[0] == 0xAAD20250u)
    {
        idx = atomicAdd(coins_storage_area, 14u);
        
        if(idx + 14u <= 1024u)
        {
            for(u32_t i = 0; i < 14u; i++)
            {
                coins_storage_area[idx + i] = data[i];
            }
        }
    }
}
