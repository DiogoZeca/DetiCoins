//
// aad_sha1_cpu_avx_miner.c
// Entry point for AVX CPU DETI coin miner
//
//

#include "aad_cpu_avx_miner.h"

int main(void)
{
    /* Run the AVX CPU miner (exits when a coin is found) */
    mine_cpu_coins_avx();
    return 0;
}