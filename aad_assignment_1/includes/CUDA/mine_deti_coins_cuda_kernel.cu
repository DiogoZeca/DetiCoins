//
// CUDA Kernel for Mining DETI Coins (with Custom Text Support)
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
    u32_t iteration_offset,
    u32_t custom_word5,
    u32_t custom_word6,
    u32_t custom_word7,
    u32_t custom_word8,
    u32_t custom_word9,
    u32_t custom_word10,
    u32_t custom_word11,
    u32_t custom_word12,
    u32_t timestamp_word_idx
)
{
    u32_t n, idx;
    u32_t data[14];
    u32_t hash[5];

    n = (u32_t)threadIdx.x + (u32_t)blockDim.x * (u32_t)blockIdx.x;
    u64_t counter = (u64_t)iteration_offset + (u64_t)n;

    // Common prefix: "DETI coin 2 "
    data[0] = 0x44455449u;
    data[1] = 0x20636F69u;
    data[2] = 0x6E203220u;

    // Counter (64-bit)
    data[3] = (u32_t)(counter & 0xFFFFFFFFu);
    data[4] = (u32_t)((counter >> 32) & 0xFFFFFFFFu);

    // Custom text words (broadcast from host)
    data[5] = custom_word5;
    data[6] = custom_word6;
    data[7] = custom_word7;
    data[8] = custom_word8;
    data[9] = custom_word9;
    data[10] = custom_word10;
    data[11] = custom_word11;
    data[12] = custom_word12;

    // Timestamp at designated position
    data[timestamp_word_idx] = base_value1;

    // SHA-1 padding
    data[13] = 0x00000A80u;

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
