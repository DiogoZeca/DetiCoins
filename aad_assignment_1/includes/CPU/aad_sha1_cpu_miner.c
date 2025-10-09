//
// aad_sha1_cpu_miner.c
// Simple entry point that calls the miner in aad_cpu_miner.h
//
// Place this file in: includes/CPU/aad_sha1_cpu_miner.c
//

#include "aad_cpu_miner.h"

int main(void)
{
    /* run the CPU miner (exits when a coin is found) */
    mine_cpu_coins();
    return 0;
}
