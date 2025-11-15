#include <stdio.h>
#include <signal.h>

#include "aad_cpu_avx512_banana_miner.h"

void handle_sigint(int sig) {
    (void)sig;
    printf("\n[Stopping AVX-512 banana miner...]\n");
    stop_signal = 1;
}

int main(void) {
    
    signal(SIGINT, handle_sigint);
    
    mine_cpu_avx512_coins();
    
    // Save final vault state
    save_coin(NULL);
    
    printf("\n TOS-Banana coins saved successfully!\n\n");
    
    return 0;
}
